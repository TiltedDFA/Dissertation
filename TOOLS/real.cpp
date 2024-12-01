//
// Created by malikt on 11/30/24.
//
#include "l2sb_real.hpp"

int main()
{
    GeneralParameters config{};
    GeneralParameters::Set(GeneralParameters::Params::BitWidth, 12);
    GeneralParameters::Set(GeneralParameters::Params::Quantisation, 11);
    GeneralParameters::Set(GeneralParameters::Params::DataRange, 2048);
    GeneralParameters::Set(GeneralParameters::Params::FileDataReadLimit, std::numeric_limits<int64_t>::max());
    GeneralParameters::Set(GeneralParameters::Params::MaxBands, std::numeric_limits<int64_t>::max());

    FileData<FileDataType::Unipolar> files("../Data/MITBIH");
    files.ReadCSVFiles(1);

    BandConfig random_config(5, {4,4,2,1,1}, {3,3,3,3,3});

    std::cin.get();
}