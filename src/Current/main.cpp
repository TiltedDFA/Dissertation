//
// Created by malikt on 11/30/24.
//
#include "FitnessFunction.hpp"
#include "Genetics.hpp"
#include <iostream>

int main()
{

    // // MITBIH: 11 bit data-width, BIPOLAR data, recommended to limit the file read amount
    // // (need to change the parameters in Types.hpp to match these when using this data set
    // FileData<Constants::General::FILE_DATA_TYPE> files("../Data/MITBIH");
    // files.ReadCSVFiles(1);


    // BONN: 12 bit data-width, UNIPOLAR data, can remove the limiter on file read amount
    // (need to change the parameters in Types.hpp to match these when using this data set
    FileData<Constants::General::FILE_DATA_TYPE> files("../Data/BONN/Healthy", "../Data/BONN/SeizureEpisode", "../Data/BONN/SeizureFree");
    files.ReadTXTFiles();

    GeneticAlgorithm ga(files);
    ga.Run();
}
