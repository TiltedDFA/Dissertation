//
// Created by malikt on 11/30/24.
//
#include "FitnessFunction.hpp"
#include "Genetics.hpp"
#include <iostream>

int main()
{
    using GenAlg = GeneticAlgorithm<Constants::Genetic::POPULATION_SIZE, Constants::General::FILE_DATA_TYPE>;

    //
    FileData<Constants::General::FILE_DATA_TYPE> files("../Data/MITBIH");
    files.ReadCSVFiles(1);
    //
    // // FileData<FileDataType::Bipolar> files("../Data/BONN/Healthy");
    // // files.ReadCSVFiles(0);
    //
    // BandConfig random_config({4,4,4}, HeaderType::Uniform, config);
    // std::cout << std::format("Calculated average compression ratio: {:4.5}", FindCompressionRatio<FileDataType::Unipolar>(files, random_config, config)) << std::endl;
    // std::cout << std::format("Calculated average compression ratio: {:4.5}", FindCompressionRatio1<FileDataType::Unipolar>(files, random_config, config)) << std::endl;
    // random_config.Print();

    GenAlg ga(files);
    ga.Run();
}
