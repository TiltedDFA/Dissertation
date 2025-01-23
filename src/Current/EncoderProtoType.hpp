


/** This is a temporary self-contained prototype to test the feasability of this design
 * A potenital replacement to BitString
 *
 */
#ifndef ENCODERPROTOTYPE_HPP
#define ENCODERPROTOTYPE_HPP

#include <array>
#include <cstdint>
#include <stdexcept>
#include <cstddef>
#include <type_traits>

template <size_t N>
class BitString
{
private:
    static constexpr size_t TOTAL_BITS_NEEDED = 2*N - 1;

    static constexpr size_t WORD_COUNT =
        (TOTAL_BITS_NEEDED <= 64) ? 1 : (TOTAL_BITS_NEEDED + 63) / 64;

    using SingleWordT =
        std::conditional_t<(TOTAL_BITS_NEEDED <= 8),  uint8_t,
        std::conditional_t<(TOTAL_BITS_NEEDED <= 16), uint16_t,
        std::conditional_t<(TOTAL_BITS_NEEDED <= 32), uint32_t,
        std::conditional_t<(TOTAL_BITS_NEEDED <= 64), uint64_t,
                           void>>>>;

    using StorageT =
        std::conditional_t<
            (TOTAL_BITS_NEEDED <= 64),
            SingleWordT,
            std::array<uint64_t, WORD_COUNT>
        >;

    StorageT data_{};

    static bool getBit(StorageT const& storage, size_t pos) noexcept
    {
        if constexpr (TOTAL_BITS_NEEDED <= 64)
        {
            using U64 = uint64_t;
            return (static_cast<U64>(storage) >> pos) & 1ULL;
        }
        else
        {
            size_t wordIndex = pos / 64;
            size_t bitIndex  = pos % 64;
            return (storage[wordIndex] >> bitIndex) & 1ULL;
        }
    }

    static void setBit(StorageT &storage, size_t pos, bool value) noexcept
    {
        if constexpr (TOTAL_BITS_NEEDED <= 64)
        {
            using U64 = uint64_t;
            U64 mask = (U64{1} << pos);
            U64 temp = static_cast<U64>(storage);
            if (value)
            {
                temp |= mask;
            }
            else
            {
                temp &= ~mask;
            }
            storage = static_cast<SingleWordT>(temp);
        }
        else
        {
            size_t wordIndex = pos / 64;
            size_t bitIndex  = pos % 64;
            uint64_t mask = (uint64_t{1} << bitIndex);
            if (value) {
                storage[wordIndex] |= mask;
            } else {
                storage[wordIndex] &= ~mask;
            }
        }
    }

public:
    
    class BitRef
    {
    public:
        BitRef(StorageT &st, size_t pos)
            : storage_(st), pos_(pos)
        {}

        explicit operator bool() const
        {
            return getBit(storage_, pos_);
        }

        BitRef& operator=(bool value)
        {
            setBit(storage_, pos_, value);
            return *this;
        }

        BitRef& operator=(const BitRef &other)
        {
            return operator=(bool(other));
        }

    private:
        StorageT &storage_;
        size_t pos_;
    };

    class BitRange
    {
    public:
        BitRange(StorageT &st, size_t offset, size_t count)
            : storage_(st), offset_(offset), size_(count)
        {}

        BitRef operator[](size_t i)
        {
            if (i >= size_) {
                throw std::out_of_range("BitRange index out of range");
            }
            return BitRef(storage_, offset_ + i);
        }

        bool operator[](size_t i) const
        {
            if (i >= size_) {
                throw std::out_of_range("BitRange index out of range");
            }
            return getBit(storage_, offset_ + i);
        }

        size_t size() const { return size_; }

    private:
        StorageT &storage_;
        size_t offset_;
        size_t size_;
    };

    BitString() = default;

    BitRef Top()
    {
        return BitRef(data_, TOTAL_BITS_NEEDED - 1);
    }
    
    bool Top() const
    {
        return getBit(data_, TOTAL_BITS_NEEDED - 1);
    }

    BitRange Lower()
    {
        return BitRange(data_, 0, N - 1);
    }

    bool LowerBit(size_t i) const
    {
        if (i >= (N - 1)) {
            throw std::out_of_range("LowerBit out of range");
        }
        return getBit(data_, i);
    }

    BitRange Upper()
    {
        return BitRange(data_, N - 1, N - 1);
    }

    bool UpperBit(size_t i) const
    {
        if (i >= (N - 1)) {
            throw std::out_of_range("UpperBit out of range");
        }
        return getBit(data_, (N - 1) + i);
    }
};




#endif 
