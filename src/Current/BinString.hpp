//
// Created by malikt on 12/9/24.
//

#ifndef BINSTRING_HPP
#define BINSTRING_HPP
#include <cstdint>
#include "BandConfig.hpp"

class BinString
{
public:
    explicit BinString(uint8_t active_bits):
        data_(),
        active_bits_(active_bits)
    {}
    void Construct(BandConfig&& bc)
    {

    }
    uint64_t& GetData() { return data_; }
    [[nodiscard]]
    BandConfig&& Destruct()const
    {

    }
private:
    uint64_t data_;
    uint8_t active_bits_;

};
#endif //BINSTRING_HPP
