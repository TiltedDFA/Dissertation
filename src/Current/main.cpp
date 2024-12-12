//
// Created by malikt on 11/30/24.
//
#include "FitnessFunction.hpp"
#include "Genetics.hpp"

int main()
{
    GenPar config{};
    config.Set(GenPar::Params::BitWidth, 12);
    config.Set(GenPar::Params::Quantisation, 12);
    config.Set(GenPar::Params::DataRange, 2048);
    config.Set(GenPar::Params::FileDataReadLimit, std::numeric_limits<int64_t>::max());
    // config.Set(GenPar::Params::FileDataReadLimit, 1000);
    config.Set(GenPar::Params::MaxBands, std::numeric_limits<int64_t>::max());
    config.Set(GenPar::Params::HeaderType, static_cast<int64_t>(HeaderType::Truncated));
    //
    FileData<FileDataType::Unipolar> files("../Data/MITBIH", std::cref(config));
    files.ReadCSVFiles(1);
    //
    // // FileData<FileDataType::Bipolar> files("../Data/BONN/Healthy");
    // // files.ReadCSVFiles(0);
    //
    // BandConfig random_config({4,4,4}, HeaderType::Uniform, config);
    // std::cout << std::format("Calculated average compression ratio: {:4.5}", FindCompressionRatio<FileDataType::Unipolar>(files, random_config, config)) << std::endl;
    // std::cout << std::format("Calculated average compression ratio: {:4.5}", FindCompressionRatio1<FileDataType::Unipolar>(files, random_config, config)) << std::endl;
    // random_config.Print();

    GeneticAlgorithm<20, FileDataType::Unipolar> ga(std::cref(config), files);
    ga.Run();
    // ga.PrintPopulation();
    // ga.EvaluatePopulation();
    // ga.PrintPopulation();
    // std::cin.get();
}
