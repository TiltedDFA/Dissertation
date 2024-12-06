//
// Created by malikt on 12/6/24.
//

#ifndef GENERAL_PARAMETERS_HPP
#define GENERAL_PARAMETERS_HPP

#include <unordered_map>

#include "Utils.hpp"

//Short for GeneralParameters, shortened as it was getting rather verbose
class GenPar
{
public:
    enum class Params
    {
        BitWidth,
        DataRange,
        Quantisation,
        FileDataReadLimit, // previously known as data_limit
        MaxBands,
    };
public:
    GenPar();
    static void Set(Params param, int64_t value);
    static int64_t Get(Params param);
private:
    static GenPar* instance_;
    std::unordered_map<Params, int64_t> config_{};
};



#endif //GENERAL_PARAMETERS_HPP
