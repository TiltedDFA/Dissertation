//
// Created by malikt on 5/9/25.
//
#include "FitnessFunction.hpp"
#include "Genetics.hpp"
#include <iostream>
#include <functional>

using DataSet = FileData<Constants::General::FILE_DATA_TYPE>;
using FunctionNamePair = std::pair<std::function<bool(DataSet const&)>, std::string>;

bool FindBestFitness(DataSet const& files)
{
    BinString best;

    //same as (2^(n-1)+1)
    constexpr size_t BandConfigRange = (1 << (Constants::General::BIT_WIDTH - 1)) + 1;

    for (size_t i{}; i < BandConfigRange;++i)
    {
        for (size_t j{}; j < 2;++j)
        {
            BinString tmp{i, bool(j)};
            tmp.SetFitnessScore(FindCompressionRatio(files, tmp));
            if (tmp.GetFitnessScore()>best.GetFitnessScore())
            {
                best = tmp;
            }
        }
    }
    // BinString test{18,true};
    //
    // // std::cout << FindCompressionRatio(files, test) << std::endl;
    // test.SetFitnessScore(FindCompressionRatio(files, test));
    // test.Print();
    // std::cout << "\n\n\n\n\n" << std::endl;
    // std::cout << std::format("Best possible score is {}\n", best.GetFitnessScore()) << std::endl;
    best.Print();
    return true;
}


int main(void)
{
    DataSet files("../Data/MITBIH");
    files.ReadCSVFiles(1);

    std::vector<FunctionNamePair> functions {FunctionNamePair(FindBestFitness, "FindBestFitness")};
    for (auto const& [fun, name] : functions)
    {
        std::cout << "Running test: " << name << "\n\n" <<std::endl;
        bool const res = fun(files);
        std::cout << "Test Completed " << (res ? "Successfully" : "Unsuccessfully") << "\n\n" << std::endl;
    }
    return 0;

}