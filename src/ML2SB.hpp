//
// Created by malikt on 11/26/24.
//

#ifndef L2SB_REAL_HPP
#define L2SB_REAL_HPP

#define LTWOSB_DEBUG 1

#if LTWOSB_DEBUG == 1
#define PRINTNL(x) std::cout << (x) << std::endl
#define PRINTNLF(x, y) std::cout << std::format((x),(y)) << std::endl
#define ERRNL(x) std::cerr << (x) << std::endl
#define AT(x) .at((x))
#else
#define NDEBUG
#define PRINTNL(x)
#define PRINTNLF(x, y)
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
using RawDataType = uint32_t;
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
        //void casting to bypass unused error as it is intentional
        assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
        instance_->config_[param]  = value;
    }
    static int64_t Get(Params const param)
    {
        assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
        assert(((void)"ERROR, uninitialized parameter attempted to be accessed", instance_->config_.contains(param)));
        return instance_->config_.at(param);
    }
private:
    static inline GenPar* instance_ = nullptr;
    std::unordered_map<Params, int64_t> config_;
};
enum class HeaderType
{
    Uniform,
    Truncated
};
class BandConfig
{
public:
    BandConfig()=delete;
    BandConfig(std::vector<uint32_t>&& band_config, HeaderType const header_type):
        band_config_(band_config.cbegin(), band_config.cend()),
        header_config_(band_config.size()),
        header_type_(header_type)
    {
        uint32_t const header_bit_width = static_cast<uint32_t>(std::ceil(std::log2(band_config.size() + 1)));
        if (header_type == HeaderType::Uniform)
        {
            std::ranges::fill(header_config_, header_bit_width);
        }
        else
        {
            //doesn't seem to quite work yet
            uint32_t const unused = static_cast<uint32_t>(std::pow(2, header_bit_width)) - band_config_.size();
            for (uint32_t i = 0; i < band_config_.size(); i++)
            {
                if (i < unused){ header_config_[i] = header_bit_width; }
                else {header_config_[i] = header_bit_width + 1; }
            }
        }
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
    void Print() const
    {
        std::string final{"\n\nBAND AND HEADER CONFIGURATION:\nUsing "};
        final += (header_type_ == HeaderType::Uniform ? "uniform" : "truncated");
        final += " headers\n";
        final += "Bands: \n";
        for (auto i = band_config_.rbegin(); i != std::prev(band_config_.rend()); ++i)
        {
            final += std::to_string(*i) + " | ";
        }
        final += std::to_string(*std::prev(band_config_.rend())) + '\n';
        final += "Headers: \n";
        for (auto i = header_config_.rbegin(); i != std::prev(header_config_.rend()); ++i)
        {
            final += std::to_string(*i) + " | ";
        }
        final += std::to_string(*std::prev(header_config_.rend())) + '\n';
        std::cout << final << std::endl;
    }
private:
    std::vector<uint32_t> const band_config_;
    std::vector<uint32_t> header_config_;
    HeaderType const header_type_;
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
    static constexpr RawDataType QuantiseData(double data)
    {
        auto const data_range = static_cast<double>(GenPar::Get(GenPar::Params::DataRange));
        auto const quantisation = static_cast<double>(GenPar::Get(GenPar::Params::Quantisation));
        if constexpr (type == FileDataType::Unipolar)
        {
            data *= std::pow(2.0, quantisation) / data_range;
            return static_cast<RawDataType>(data);
        }
        else if constexpr (type == FileDataType::Bipolar)
        {
            data = data + data_range;
            data *= std::pow(2.0, quantisation) / data_range / 2.0;;
            return static_cast<RawDataType>(data);
        }
        else{static_assert(false);}
        //get rid of warnings
        return std::numeric_limits<RawDataType>::max();
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
            RawDataType data_max{};
            // auto const it = file_data_.begin() + file_data_.size();
            while (std::getline(file, line) && data_count < data_limit)
            {
                double const data_term = std::stod(SplitCSVLine(line)AT(term));
                data_max = std::max(data_max, static_cast<RawDataType>(data_term));
                RawDataType const data = QuantiseData(data_term);
                file_data_.push_back(data);
                ++data_count;
            }
            PRINTNL(std::format("File Imported \"{}\", {} data points quantised, data range <0-{}>", f, data_count, data_max));
        }
        PRINTNL(std::format("Read: {:5.2f} MB of data", static_cast<double>(sizeof(RawDataType) * file_data_.size()) / ByteToMB));
    }
    [[nodiscard]]
    std::vector<RawDataType> const& GetFileData() const{return file_data_;}
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
        std::ranges::sort(files);
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
    std::vector<RawDataType> file_data_;
};
template<typename T>
requires std::is_integral_v<T>
constexpr uint8_t FindMS1B(T inp)
{
    uint8_t count{};
    while (inp >>= 1)
        ++count;
    // return count + bool(count);
    return count;
}
template <FileDataType type>
static double FindCompressionRatio(FileData<type> const& file_data, BandConfig const& band_config)
{
    double compression_ratio{};
    std::vector<uint32_t> const& bands   = band_config.GetBandConfig();
    std::vector<uint32_t> const& headers = band_config.GetHeaderConfig();
    std::vector<RawDataType> const& data = file_data.GetFileData();
    uint64_t const data_bit_width = GenPar::Get(GenPar::Params::BitWidth);
    // uint64_t bit_count = headers.back() + quantisation;
    uint64_t bit_count{};

    for (auto it = data.cbegin(); it != std::prev(data.cend()); ++it)
    {
        uint32_t const difference = *it ^ *(it + 1);
        uint32_t const msb_loc = FindMS1B(difference);
        uint32_t band_total{};
        size_t idx{};

        while (msb_loc > band_total)
            band_total += bands[idx++];

        bit_count += band_total + headers[idx - bool(msb_loc)];
    }
    compression_ratio = static_cast<double>(data_bit_width * data.size()) / static_cast<double>(bit_count);
    PRINTNLF("Raw:\t\t\t\t{}", data_bit_width * data.size());
    PRINTNLF("Compressed:\t\t\t{}", bit_count);
    return compression_ratio;
}
#endif //L2SB_REAL_HPP
