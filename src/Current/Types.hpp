//
// Created by malikt on 12/6/24.
//

#ifndef TYPES_HPP
#define TYPES_HPP


#include <cmath>

#include "Utils.hpp"
#include <cstdint>
#include <format>
#include <functional>
#include <type_traits>


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
        inline constexpr size_t NUM_BANDS = BIT_WIDTH - 1;
        inline constexpr size_t DATA_RANGE = 2048;
        inline constexpr size_t FILE_DATA_READ_LIMIT = 20'000;
        inline constexpr size_t MAX_BANDS = std::numeric_limits<size_t>::max();
        inline constexpr HeaderType HEADER_TYPE = HeaderType::Truncated;
        inline constexpr FileDataType FILE_DATA_TYPE = FileDataType::Unipolar;
    }

    /**
     * This namespace is used for helpers to the BinString class.
     *
     * A binary string of length N, where N is BIT_WIDTH * 2, will store:
     *  - 1 bit at the top to indicate whether a configuration has a zero state
     *
     *  - (N/2) bits in the middle to store the header layout. A header bit
     *         will be 0 if the header has the value of: ceil(log2(BAND_COUNT))
     *         and will where the value is: floor(log2(BAND_COUNT))
     *
     *  - (N/2 - 1) bits will be for storing band separators. the size of a band
     *          will be the distance between the separators.
     *
     *
     *   For a bit_width of 3, an uint8_t will be used for storage.
     *   Let:
     *      Z = the zero bit
     *      H = a header bit
     *      B = a band bit
     *      X = an unused bit
     *
     *  The internal representation will appear as follows:
     *      ZHHH XXBB
     *  Where Z is the MSB.
     *
     *  The order for the data should be inserted is that a higher bit corresponds to
     *  the first element.
     *  In which a band configuration vector v, with the value: {3,1,1,7}
     *  will be stored *in the band bit section* with the following value:
     *      001 1100 0000
     */
    namespace BinaryString
    {
        /*
         * currently supports storing as arrays where the input type size requires it
         * but does not support returning arrays in instances where the band separators
         * would take up more than 64 bits.
        */
        namespace _
        {
            inline constexpr size_t BITS_NEEDED = ::Constants::General::BIT_WIDTH * 2;
            inline constexpr size_t VAR_COUNT =  (BITS_NEEDED <= 64) ? 1 : (BITS_NEEDED + 63) / 64;
            using OneVarT = std::conditional_t<(BITS_NEEDED <= 8), uint8_t,
                            std::conditional_t<(BITS_NEEDED <= 16), uint16_t,
                            std::conditional_t<(BITS_NEEDED <= 32), uint32_t,
                            std::conditional_t<(BITS_NEEDED <= 64), uint64_t,
                            void>>>>;
            inline constexpr size_t TOP_BIT_LOC = (sizeof(OneVarT) * 8) - 1;
            inline constexpr size_t BANDS_LOC = 0;
            inline constexpr size_t BANDS_SIZE = ::Constants::General::NUM_BANDS;
            inline constexpr size_t HEADERS_LOC = BANDS_SIZE;
            inline constexpr size_t HEADERS_SIZE = ::Constants::General::BIT_WIDTH;
            inline constexpr bool NEEDS_ARRAY = VAR_COUNT > 1;
            inline constexpr size_t HEADER_MASK = Utils::GenMask<size_t, _::HEADERS_SIZE>();
            inline constexpr size_t HEADER_MASK_INPLACE = HEADER_MASK << HEADERS_LOC;
            inline constexpr size_t BAND_MASK = Utils::GenMask<size_t, _::BANDS_SIZE>();
            inline constexpr size_t BAND_MASK_INPLACE = BAND_MASK << BANDS_LOC;


            // upon further consideration, cannot do something like this as the size of a header
            // depends on the number of bands which is configuration specific.
            // inline constexpr uint64_t UNIFORM_HEADER_VALUE = std::ceil(std::log2());
        }
        //for simplicity’s sake, for now will not support arrays of words
        static_assert(!_::NEEDS_ARRAY, "USED UNSUPPORTED FEATURE: BIT STRING ARRAYS (BIT WIDTH WAS >= 33)");

        using Type =    std::conditional_t<(_::NEEDS_ARRAY),
                        std::array<uint64_t, _::VAR_COUNT>,
                        _::OneVarT>;

        using ViewParamType = std::conditional_t<(_::NEEDS_ARRAY), Type const&, Type const>;

        constexpr uint64_t GetBandSeparators(ViewParamType t) noexcept
        {
            return (t >> _::BANDS_LOC) & Utils::GenMask<size_t, _::BANDS_SIZE>();
        }
        constexpr void SetBandSeparators(Type& t, ViewParamType value) noexcept
        {
            assert(((void)"Value passed to set bands is too large", (value & _::BAND_MASK) == value));

            t &= ~_::BAND_MASK_INPLACE;
            t |= value << _::BANDS_LOC;
        }

        /**
         *  When using truncated binary, we will only ever have 2 lengths of data (K, K-1),
         *  therefore the headers within this binary string will store 0 if they are of length K
         *  and 1 if they're of length K-1. (Might be possible to de
         *
         *
         *
         */
        constexpr uint64_t GetHeaders(ViewParamType t, size_t const num_headers) noexcept
        {
            return (t >> _::HEADERS_LOC) & Utils::GenMask(num_headers);
            // return (t >> _::HEADERS_LOC) & _::HEADER_MASK;
        }

        /**
         * Should not be used directly.
         */
        constexpr void SetHeaders(Type& t, ViewParamType value) noexcept
        {

            assert(((void)"Value passed to set headers is too large", (value & _::HEADER_MASK) == value));

            t &= ~_::HEADER_MASK_INPLACE;
            t |= value << _::HEADERS_LOC;
        }
        /*
        *
        *   If n is a power of two, then the coded value for 0 ≤ x < n is the simple binary code for x of length log2(n).
        *   Otherwise let k = floor(log2(n)), such that 2^k < n < 2^k+1 and let u = 2^k+1 − n.
        *
        * Truncated binary encoding assigns the first u symbols codewords of length k and then
        *   assigns the remaining n − u symbols the last n − u codewords of length k + 1.
        * Because all the codewords of length k + 1 consist of an unassigned codeword of length k
        *   with a "0" or "1" appended, the resulting code is a prefix code.
        *
        *
        * src: https://en.wikipedia.org/wiki/Truncated_binary_encoding
        */
        /*
        *
        uint32_t const count = band_config_.size() + 1;
        // if 1 then n is power of 2, should never be zero
        bool const can_truncate = std::popcount(count) > 1;
        if (can_truncate)
        {
            uint32_t const k = Utils::FindK(count);
            uint32_t const u = ((1 << (k + 1)) - count);
            std::fill_n(header_config_.begin(), u, k);
            std::fill_n(header_config_.begin() + u, count - u - 1, k + 1);
        }
        else
        {
            std::ranges::fill(header_config_, static_cast<uint32_t>(std::log2(count)));
        }
         */
        //This entire function could do with a few cycles of verification
        constexpr void CalculateHeaders(Type& t, size_t const header_count) noexcept
        {
            if constexpr (::Constants::General::HEADER_TYPE == HeaderType::Uniform)
            {
                //fill all with 0s
                SetHeaders(t, 0);
            }
            else
            {
//        *   Otherwise let k = floor(log2(n)), such that 2^k < n < 2^k+1 and let u = 2^k+1 − n.
                bool const can_truncate = std::popcount(header_count) > 1;
                // if 1 then n is power of 2, should never be zero
                if (can_truncate)
                {
                    //finds 2^(k+1) - n
                    Type const U = static_cast<Type>(std::ceil(std::log2(header_count))) - header_count;
                    // Generates a mask of U bits, which would be same as adding a 1 U times in a for loop
                    Type const Umask = Utils::GenMask(U);
                    // Shifts the mask to the correct place, as per my specification of early element corresponding
                    // to higher bits
                    Type const UMaskShifted = Umask << (header_count - U);
                    // Set the calculated headers
                    SetHeaders(t, UMaskShifted);
                }
                else
                {
                    SetHeaders(t, 0);
                }
            }
        }
        constexpr bool HasZeroState(ViewParamType t) noexcept
        {
            return static_cast<bool>(t & ~(1ULL << _::TOP_BIT_LOC));
        }
        constexpr void SetZeroState(Type& t, bool const b) noexcept
        {
            t = (t & ~(1ULL << _::TOP_BIT_LOC)) | (static_cast<int>(b) << _::TOP_BIT_LOC);
        }
    }
}
#endif //TYPES_HPP
