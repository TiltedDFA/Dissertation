//
// Created by malikt on 11/30/24.
//
#include "ML2SB.hpp"

int main()
{
    GenPar config{};
    GenPar::Set(GenPar::Params::BitWidth, 12);
    GenPar::Set(GenPar::Params::Quantisation, 12);
    GenPar::Set(GenPar::Params::DataRange, 2048);
    GenPar::Set(GenPar::Params::FileDataReadLimit, std::numeric_limits<int64_t>::max());
    GenPar::Set(GenPar::Params::MaxBands, std::numeric_limits<int64_t>::max());

    FileData<FileDataType::Unipolar> files("../Data/MITBIH");
    files.ReadCSVFiles(1);

    // FileData<FileDataType::Bipolar> files("../Data/BONN/Healthy");
    // files.ReadCSVFiles(0);

    // BandConfig random_config(5, {4,4,2,1,1}, {3,3,3,3,3});
    BandConfig random_config(7, {2,2,2,2,2,1,1}, {3,3,3,3,3,3,3});


    std::cout << std::format("Calculated average compression ratio: {:4.5}", FindCompressionRatio<FileDataType::Unipolar>(files, random_config)) << std::endl;
    std::cin.get();
}