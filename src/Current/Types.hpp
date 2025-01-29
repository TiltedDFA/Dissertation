//
// Created by malikt on 12/6/24.
//

#ifndef TYPES_HPP
#define TYPES_HPP


#include "Utils.hpp"
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
    namespace BinaryString
    {
        /*
         * currently supports storing as arrays where the input type size requires it
         * but does not support returning arrays in instances where the band separators
         * would take up more than 64 bits.
        */
        namespace _
        {
            inline constexpr size_t BITS_NEEDED = (::Constants::General::BIT_WIDTH * 2) - 1;
            inline constexpr size_t VAR_COUNT =  (BITS_NEEDED <= 64) ? 1 : (BITS_NEEDED + 63) / 64;
            using OneVarT = std::conditional_t<(BITS_NEEDED <= 8), uint8_t,
                            std::conditional_t<(BITS_NEEDED <= 16), uint16_t,
                            std::conditional_t<(BITS_NEEDED <= 32), uint32_t,
                            std::conditional_t<(BITS_NEEDED <= 64), uint64_t,
                            void>>>>;
            inline constexpr size_t TOP_BIT_LOC = (sizeof(OneVarT) * 8) - 1;
            inline constexpr size_t BANDS_LOC = 0;
            inline constexpr size_t BANDS_SIZE = ::Constants::General::BIT_WIDTH - 1;
            inline constexpr size_t HEADERS_LOC = BANDS_SIZE;
            inline constexpr size_t HEADERS_SIZE = ::Constants::General::BIT_WIDTH;
            inline constexpr bool NEEDS_ARRAY = VAR_COUNT > 1;
        }
        using Type =    std::conditional_t<(_::NEEDS_ARRAY),
                        std::array<uint64_t, _::VAR_COUNT>,
                        _::OneVarT>;

        using ViewParamType = std::conditional_t<(_::NEEDS_ARRAY), Type const&, Type const>;

        constexpr uint64_t GetBandSeparators(ViewParamType t) noexcept
        {
            return (t >> _::BANDS_LOC) & GenMask<size_t, _::BANDS_SIZE>();
        }
        constexpr uint64_t GetHeaders(ViewParamType t) noexcept
        {
            return (t >> _::HEADERS_LOC) & GenMask<size_t, _::HEADERS_SIZE>();
        }
        constexpr bool HasZeroState(ViewParamType t) noexcept
        {
            return static_cast<bool>(t & ~(1ULL << _::TOP_BIT_LOC));
        }
    }
}
#endif //TYPES_HPP
