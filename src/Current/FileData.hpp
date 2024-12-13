//
// Created by malikt on 12/6/24.
//

#ifndef FILEDATA_HPP
#define FILEDATA_HPP

#include <fstream>
#include <ranges>
#include <filesystem>
#include <algorithm>

#include "BandConfig.hpp"


template<FileDataType type>
class FileData
{
public:
    explicit FileData(std::string_view const path, std::reference_wrapper<GenPar const>&& gp):
        path_to_files_(path),
        file_data_(),
        gen_par_(gp)
        {}
    [[nodiscard]]
    constexpr RawDataType QuantiseData(double data) const
    {
        auto const data_range = static_cast<double>(gen_par_.get().Get(GenPar::Params::DataRange));
        auto const quantisation = static_cast<double>(gen_par_.get().Get(GenPar::Params::Quantisation));
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
        else{static_assert(false, "");}
        //get rid of warnings
        return std::numeric_limits<RawDataType>::max();
    }

    ///
    /// @param term the term within a line of the csv file that we are interested in (the data)
    void ReadCSVFiles(uint32_t const term)
    {
        auto const files = FindCSVFiles();
        std::size_t const data_limit = gen_par_.get().Get(GenPar::Params::FileDataReadLimit);
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
        std::cout << std::format("Read: {:5.2f} MB of data across {} data points", static_cast<double>(sizeof(RawDataType) * file_data_.size()) / ByteToMB, file_data_.size()) << std::endl;
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
                files.emplace_back(std::move(entry.path().string()));
            }
        }
        if (files.empty())
        {
            std::cerr << "No files found" << std::endl;
            std::exit(EXIT_FAILURE);
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
    std::reference_wrapper<GenPar const> gen_par_;
};



#endif //FILEDATA_HPP
