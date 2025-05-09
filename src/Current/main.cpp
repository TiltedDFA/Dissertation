//
// Created by malikt on 11/30/24.
//
#include "FitnessFunction.hpp"
#include "Genetics.hpp"
#include <iostream>

int main()
{

    FileData<Constants::General::FILE_DATA_TYPE> files("../Data/MITBIH");
    files.ReadCSVFiles(1);

    GeneticAlgorithm ga(files);
    ga.Run();
}
