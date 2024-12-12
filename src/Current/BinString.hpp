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
    explicit BinString(std::reference_wrapper<GenPar const> gp):
        gp_(gp),
        data_(),
        active_bits_(gp_.get().Get(GenPar::Params::BitWidth) - 1)
    {}
    BinString(BandConfig const& bc, std::reference_wrapper<GenPar const> gp):
        BinString(gp)
    {
        Construct(bc);
    }
    BinString(uint64_t const bits, std::reference_wrapper<GenPar const> gp):
        BinString(gp)
    {
        data_ = bits;
    }
    void Construct(BandConfig const& bc)
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
    BandConfig Destruct()const
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
        return {std::move(bands), gp_};
    }
private:
    std::reference_wrapper<GenPar const> gp_;
    uint64_t data_;
    uint8_t active_bits_;
};
#endif //BINSTRING_HPP
