//
// Created by malikt on 12/9/24.
//

#ifndef BINSTRING_HPP
#define BINSTRING_HPP
#include <bitset>
#include <cstdint>
#include "BandConfig.hpp"

class BinString
{
public:
    constexpr BinString():
        data_(),
        active_bits_(Constants::General::BIT_WIDTH - 1)
    {}
    constexpr BinString(BandConfig const& bc):
        BinString()
    {
        Construct(bc);
    }
    constexpr BinString(uint64_t const bits):
        BinString()
    {
        data_ = bits;
    }
    constexpr void Construct(BandConfig const& bc)
    {
        auto const& bands = bc.GetBandConfig();
        data_ = 0;
        for (auto const& band : bands)
        {
            // PRINTBS(data_);
            auto const make_space = (data_ << band);
            // PRINTBS(make_space);
            auto const mask = (1ULL << band) - 1;
            // PRINTBS(mask);
            auto const new_data_val = make_space | mask;
            // PRINTBS(new_data_val);
            data_ = new_data_val << 1;
            // data_ = ((data_ << band) | ((1ULL << band) - 1) << 1);
        }
    }
    uint64_t& GetData() { return data_; }
    [[nodiscard]]
    uint64_t GetData() const { return data_; }
    [[nodiscard]]
    constexpr BandConfig Destruct()const
    {
        std::vector<uint32_t> bands{1};
        for (int i = 0; i < active_bits_; ++i)
        {
            if ((data_ >> i) & 1ULL)
            {
                bands.push_back(1);
            }
            else
            {
                ++bands[bands.size()-1];
            }
        }
        return {std::move(bands)};
    }
private:
    uint64_t data_;
    uint8_t active_bits_;
};
#endif //BINSTRING_HPP
