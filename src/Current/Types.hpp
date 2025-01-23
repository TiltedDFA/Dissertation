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

namespace Constants
{
    namespace Genetic
    {
        constexpr double INITIAL_TEMPERATURE = 100.0;
        constexpr double TEMPERATURE_COOLING_RATE = 0.95;
        constexpr size_t BOLTZMANN_TOURNAMENT_SIZE = 3;
        constexpr double MUTATION_CHANCE = 0.4;
        constexpr size_t ELITE_COUNT = 2;
        constexpr size_t POPULATION_SIZE = 200;
        constexpr size_t RANDOM_IMMIGRATION_COUNT = 2;
        constexpr size_t NUMBER_OF_RUNS = 300;
    }
}
#endif //TYPES_HPP
