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



enum class FileDataType
{
    Unipolar = 0,
    Bipolar
};
class L2SBConfig
{
public:

private:

};

template<FileDataType type>
constexpr uint32_t QuantiseData(double data)
{

}

template<FileDataType type>
class FileData
{
public:
    explicit FileData(const std::string_view path, const uint64_t data_limit = std::numeric_limits<uint64_t>::max()):
        path_to_files_(path),
        data_limit_(data_limit),
        file_data_()
        {}

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

    static std::vector<std::string> SplitCSVLine(const std::string& line, const char delimiter = ',')
    {
        std::vector<std::string> terms;
        std::stringstream ss(line);
        std::string term;

        while (std::getline(ss, term, delimiter))
            terms.push_back(term);

        return terms;
    }
    void ReadCSVFiles(const uint32_t term = 0)
    {
        auto const files = FindCSVFiles();
        for (auto const& f : files)
        {
            std::ifstream file(f);
            std::string line;
            uint64_t data_count{};

            while (std::getline(file, line) && data_count < data_limit_)
            {
                const std::string&& data_term = SplitCSVLine(line)AT(term);
                file_data_.push_back(QuantiseData<type>(std::stod(data_term)));
                ++data_count;
            }
            PRINTNL(std::format("File Imported{}, {} data points quantised, data range <0-{}>"), f, data_count, );
        }
    }
private:
    const std::string path_to_files_;
    const uint64_t data_limit_;
    std::vector<uint32_t> file_data_;
};
#endif //L2SB_REAL_HPP
