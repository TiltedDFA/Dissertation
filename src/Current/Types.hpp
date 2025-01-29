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
/*
 *
 *
*    config.Set(GenPar::Params::BitWidth, 11);
    config.Set(GenPar::Params::Quantisation, 11);
    config.Set(GenPar::Params::DataRange, 2048);
    // config.Set(GenPar::Params::FileDataReadLimit, std::numeric_limits<int64_t>::max());
    config.Set(GenPar::Params::FileDataReadLimit, 20000);
    config.Set(GenPar::Params::MaxBands, std::numeric_limits<int64_t>::max());
    config.Set(GenPar::Params::HeaderType, static_cast<int64_t>(HeaderType::Truncated));
    //
 *
 */
namespace Constants
{
    namespace Genetic
    {
        inline constexpr double INITIAL_TEMPERATURE = 100.0;
        inline constexpr double TEMPERATURE_COOLING_RATE = 0.95;
        inline constexpr size_t BOLTZMANN_TOURNAMENT_SIZE = 3;
        inline constexpr double MUTATION_CHANCE = 0.4;
        inline constexpr size_t ELITE_COUNT = 2;
        inline constexpr size_t POPULATION_SIZE = 200;
        inline constexpr size_t RANDOM_IMMIGRATION_COUNT = 2;
        inline constexpr size_t NUMBER_OF_RUNS = 300;
    }
    namespace General
    {
        inline constexpr size_t BIT_WIDTH = 11;
        inline constexpr size_t QUANTISATION = 11;
        inline constexpr size_t DATA_RANGE = 2048;
        inline constexpr size_t FILE_DATA_READ_LIMIT = 20'000;
        inline constexpr size_t MAX_BANDS = std::numeric_limits<size_t>::max();
        inline constexpr HeaderType HEADER_TYPE = HeaderType::Truncated;
        inline constexpr FileDataType FILE_DATA_TYPE = FileDataType::Unipolar;
    }
}
#endif //TYPES_HPP
