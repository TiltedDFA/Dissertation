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
        BitWidth = 0,
        DataRange,
        Quantisation,
        FileDataReadLimit, // previously known as data_limit
        MaxBands,
        ENUM_SIZE
    };
public:
    constexpr GenPar()=default;
    constexpr void Set(Params param, int64_t value)
    {
        //void casting to bypass unused error as it is intentional
        assert(((void)"Attempted to overwrite existing value", !config_.at(static_cast<size_t>(param)).has_value()));

        config_ AT(static_cast<size_t>(param))  = value;
    }
    [[nodiscard]]
    constexpr int64_t Get(Params param) const
    {
        return config_ AT(static_cast<size_t>(param)).value();
    }
private:
    std::array<std::optional<int64_t>, static_cast<size_t>(Params::ENUM_SIZE)> config_;
};



#endif //GENERAL_PARAMETERS_HPP
