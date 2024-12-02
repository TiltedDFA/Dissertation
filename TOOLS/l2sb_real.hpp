//
// Created by malikt on 11/26/24.
//

#ifndef L2SB_REAL_HPP
#define L2SB_REAL_HPP

#define LTWOSB_DEBUG 1

#if LTWOSB_DEBUG == 1
#define PRINTNL(x) std::cout << (x) << std::endl
#define ERRNL(x) std::cerr << (x) << std::endl
#define AT(x) .at((x))
#else
#define NDEBUG
#define PRINTNL(x)
#define ERRNL(x)
#define AT(x) [(x)]
#endif

#include <filesystem>
#include <string_view>
#include <vector>
#include <string>
#include <cinttypes>
#include <fstream>
#include <format>
#include <iostream>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <complex>
#include <numeric>

constexpr double ByteToMB = 1024.0 * 1024.0;
enum class FileDataType
{
    Unipolar = 0,
    Bipolar
};
//Short for GeneralParameters, shorted as it was getting rather verbose
class GenPar
{
public:
    enum class Params
    {
        BitWidth,
        DataRange,
        Quantisation,
        FileDataReadLimit, // previously known as data_limit
        MaxBands,
    };
public:
    GenPar()
    {
        assert(instance_ == nullptr);
        instance_ = this;
    };

    static void Set(Params const param, int64_t const value)
    {
        assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
        instance_->config_[param]  = value;
    }
    static int64_t Get(Params const param)
    {
        assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
        //void casting to bypass unused error as is intentional
        assert(((void)"ERROR, uninitialized parameter attempted to be accessed", instance_->config_.contains(param)));
        return instance_->config_.at(param);
    }
private:
    static inline GenPar* instance_ = nullptr;
    std::unordered_map<Params, int64_t> config_;
};
class BandConfig
{
public:
    BandConfig()=delete;
    BandConfig(uint32_t const num_bands,std::vector<uint32_t>&& band_config, std::vector<uint32_t>&& header_config):
        bands_(num_bands),
        band_config_(std::move(band_config)),
        header_config_(std::move(header_config))
    {
        //verify that we have a "legal" configuration
        assert(((void)"Mismatched header - band config sizes", header_config_.size() == band_config_.size()));
        assert(((void)"Configuration band number exceeds the maximum", band_config_.size() <= GenPar::Get(GenPar::Params::MaxBands)));
        assert(
            ((void)"Mismatched band number-bit size count",
            std::accumulate(band_config_.cbegin(), band_config_.cend(), 0U) ==
            GenPar::Get(GenPar::Params::BitWidth))
            );
    }
    [[nodiscard]]
    std::vector<uint32_t> const& GetBandConfig() const{return band_config_;}
    [[nodiscard]]
    std::vector<uint32_t> const& GetHeaderConfig() const{return header_config_;}
private:
    uint32_t const bands_;
    std::vector<uint32_t> const band_config_;
    std::vector<uint32_t> const header_config_;
};
template<FileDataType type>
class FileData
{
public:
    explicit FileData(const std::string_view path):
        path_to_files_(path),
        file_data_()
        {}
    [[nodiscard]]
    static constexpr uint32_t QuantiseData(double data)
    {
        auto const data_range = static_cast<double>(GenPar::Get(GenPar::Params::DataRange));
        auto const quantisation = static_cast<double>(GenPar::Get(GenPar::Params::Quantisation));
        if constexpr (type == FileDataType::Unipolar)
        {
            data *= std::pow(2.0, quantisation) / data_range;
            return static_cast<uint32_t>(data);
        }
        else if constexpr (type == FileDataType::Bipolar)
        {
            data = data + data_range;
            data *= std::pow(2.0, quantisation) / data_range / 2.0;;
            return static_cast<uint32_t>(data);
        }
        else{static_assert(false);}
        //get rid of warnings
        return std::numeric_limits<uint32_t>::max();
    }

    /// 
    /// @param term the term within a line of the csv file that we are interested in (the data)
    void ReadCSVFiles(uint32_t const term)
    {
        auto const files = FindCSVFiles();
        std::size_t const data_limit = GenPar::Get(GenPar::Params::FileDataReadLimit);
        for (auto const& f : files)
        {
            std::ifstream file(f);
            std::string line;
            uint64_t data_count{};
            uint32_t data_max{};
            // auto const it = file_data_.begin() + file_data_.size();
            while (std::getline(file, line) && data_count < data_limit)
            {
                double const data_term = std::stod(SplitCSVLine(line)AT(term));
                data_max = std::max(data_max, static_cast<uint32_t>(data_term));
                uint32_t const data = QuantiseData(data_term);
                file_data_.push_back(data);
                ++data_count;
            }
            PRINTNL(std::format("File Imported \"{}\", {} data points quantised, data range <0-{}>", f, data_count, data_max));
        }
        PRINTNL(std::format("Read: {:5.2f} MB of data", static_cast<double>(sizeof(uint32_t) * file_data_.size()) / ByteToMB));
    }
    [[nodiscard]]
    std::vector<uint32_t> const& GetFileData() const{return file_data_;}
private:
    [[nodiscard]]
    std::vector<std::string> FindCSVFiles()const
    {
        std::vector<std::string> files;
        for (auto const& entry : std::filesystem::directory_iterator(path_to_files_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".csv")
            {
                //std::move?
                files.push_back(entry.path().string());
            }
        }
        return files;
    }

    static std::vector<std::string> SplitCSVLine(std::string const& line, char const delimiter = ',')
    {
        std::vector<std::string> terms;
        std::stringstream ss(line);
        std::string term;

        while (std::getline(ss, term, delimiter))
            terms.push_back(term);

        return terms;
    }
private:
    std::string const path_to_files_;
    std::vector<uint32_t> file_data_;
};
template <FileDataType type>
static double FindCompressionRatio(FileData<type> const& file_data, BandConfig const& band_config)
{
    double compression_ratio{};
    std::vector<uint32_t> const& bands   = band_config.GetBandConfig();
    std::vector<uint32_t> const& headers = band_config.GetHeaderConfig();
    std::vector<uint32_t> const& data = file_data.GetFileData();
    uint32_t const quantisation = GenPar::Get(GenPar::Params::Quantisation);
    uint64_t bit_count = headers.back() + GenPar::Get(GenPar::Params::Quantisation);
    PRINTNL(std::format("headers.back: {}", headers.back()));
    for (auto it = data.cbegin(); it != (data.cend() - 1); ++it)
    {
        uint32_t difference = *it ^ *(it + 1);
        for (int i = 0; i <= quantisation; ++i)
        {
            difference |= difference >> 1;
        }
        uint32_t q_point = quantisation;
        uint32_t header = headers.back();
        std::size_t i{};
        for (; i < bands.size(); ++i)
        {
            //if 0
            if (!(difference & 1U << (q_point - 1)))  break;
            q_point -= bands[i];
            header = headers[i];
        }
        bit_count += header + std::accumulate(bands.cbegin(), bands.cbegin() + i, 0U);
        // PRINTNL(std::format("bit_count: {}", bit_count));
    }
    PRINTNL(quantisation);
    PRINTNL(data.size());
    PRINTNL(quantisation * data.size());
    PRINTNL(bit_count);
    compression_ratio = static_cast<double>(quantisation * data.size()) / static_cast<double>(bit_count);
    return compression_ratio;
}
#endif //L2SB_REAL_HPP
