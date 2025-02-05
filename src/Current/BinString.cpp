//
// Created by Malik T on 30/01/2025.
//
#include "BinString.hpp"

BinString::BinString(std::mt19937 &rng):
        BinString()
{
    type temp{};
    for (size_t i = 0; i < Constants::General::NUM_BANDS; ++i)
    {
        temp |= (rng() & 1ULL) << i;
    }

    bool const has_zero_state = rng() & 1ULL;
    Constants::BinaryString::SetBandSeparators(data_, temp);
    Constants::BinaryString::SetZeroState(data_, has_zero_state);

    num_bands_ = std::popcount(temp) + 1 + has_zero_state;

    //ensuring that there is no more bands than the permitted amount
    assert(((void)"",
        data_ ==
        (data_ & GenMask<uint64_t, Constants::General::NUM_BANDS>()))
        );
}
//     for (auto const& band : bands)
//     {
//         // PRINTBS(data_);
//         auto const make_space = (data_ << band);
//         // PRINTBS(make_space);
//         auto const mask = (1ULL << band) - 1;
//         // PRINTBS(mask);
//         auto const new_data_val = make_space | mask;
//         // PRINTBS(new_data_val);
//         data_ = new_data_val << 1;
//         // data_ = ((data_ << band) | ((1ULL << band) - 1) << 1);
//     }
BinString::BinString(std::vector<uint8_t> const& configurations):
    BinString()
{
    // {3, 3, 2}        |   {2,1,1,3,1}
    // XXX XXX XX       |   XX X X XXX X
    //  0010010         |    0111001
    num_bands_ = configurations.size();
    type temp{};
    for (view_type band : configurations)
    {
        temp = (temp << band) | 1;
    }
    temp >>= 1;
    Constants::BinaryString::SetZeroState(data_, configurations.back() == 0);

}