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
#include <cmath>

constexpr double ByteToMB = 1024.0 * 1024.0;
enum class FileDataType
{
    Unipolar = 0,
    Bipolar
};

//ditching this design pattern since it would be hard to tell if calling a getter before using a setter,
//also wanting to avoid a messy ctor with a lotta params.
class L2SBConfig
{
public:
    L2SBConfig()=default;

    L2SBConfig& SetBitWidth(uint32_t const bit_width){ bit_width_ = bit_width; return *this; }
    L2SBConfig& SetDataRange(double const dr) { data_range_ = dr; return *this; }
    L2SBConfig& SetQuantisation(double const q) { quantisation_ = q; return *this; }

    [[nodiscard]]
    uint32_t bit_width() const { return bit_width_; }
    [[nodiscard]]
    double data_range() const { return data_range_; }
    [[nodiscard]]
    double quantisation() const { return quantisation_; }
private:
    uint32_t bit_width_;
    double data_range_;
    double quantisation_;
};



template<FileDataType type>
class FileData
{
public:
    explicit FileData(const std::string_view path, L2SBConfig const& config, const int64_t data_limit = std::numeric_limits<int64_t>::max()):
        path_to_files_(path),
        config_(config),
        data_limit_(data_limit),
        file_data_()
        {}
    [[nodiscard]]
    constexpr uint32_t QuantiseData(double data) const
    {
        double constexpr data_range = 2048.0;
        auto const quantisation = static_cast<double>(config_.bit_width());
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
    void ReadCSVFiles(uint32_t const term)
    {
        auto const files = FindCSVFiles();
        for (auto const& f : files)
        {
            std::ifstream file(f);
            std::string line;
            uint64_t data_count{};
            uint32_t data_max{};
            while (std::getline(file, line) && data_count < data_limit_)
            {
                const double data_term = std::stod(SplitCSVLine(line)AT(term));
                // PRINTNL(std::format("data before quant: {}, {}", data_term, std::stod(data_term)));
                data_max = std::max(data_max, static_cast<uint32_t>(data_term));
                const uint32_t data = QuantiseData(data_term);
                file_data_.push_back(data);
                ++data_count;
            }
            PRINTNL(std::format("File Imported{}, {} data points quantised, data range <0-{}>", f, data_count, data_max));
        }
        PRINTNL(std::format("Read: {:5.2f} MB of data", static_cast<double>(sizeof(uint32_t) * file_data_.size()) / ByteToMB));
    }
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
    L2SBConfig const& config_;
    int64_t const data_limit_;
    std::vector<uint32_t> file_data_;
};
#endif //L2SB_REAL_HPP
