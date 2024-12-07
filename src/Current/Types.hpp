//
// Created by malikt on 12/6/24.
//

#ifndef TYPES_HPP
#define TYPES_HPP
#include <cstdint>
#include <format>
#include <functional>
using RawDataType   = uint32_t;
using FitnessScore  = double;
// using RawDataType = int64_t;
enum class FileDataType
{
    Unipolar = 0,
    Bipolar
};
enum class HeaderType
{
    Uniform,
    Truncated
};
#endif //TYPES_HPP
