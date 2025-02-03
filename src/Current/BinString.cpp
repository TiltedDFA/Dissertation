//
// Created by Malik T on 30/01/2025.
//
#include "BinString.hpp"

BinString::BinString(std::mt19937 &rng):
        BinString()
{
    for (size_t i = 0; i < Constants::General::NUM_BANDS; ++i)
    {
        data_ |= (rng() & 1ULL) << i;
    }

    //ensuring that there is no more bands than the permitted amount
    assert((void)"",
        data_ ==
        (data_ & GenMask<uint64_t, Constants::General::NUM_BANDS>())
        );
}