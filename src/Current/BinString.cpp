//
// Created by Malik T on 30/01/2025.
//
#include "BinString.hpp"

BinString::BinString(std::mt19937 &rng):
        BinString()
{
    using Constants::BinaryString::SetBandSeparators;
    using Constants::BinaryString::SetZeroState;
    using Constants::BinaryString::CalculateHeaders;

    type temp{};
    for (size_t i = 0; i < Constants::General::NUM_BANDS; ++i)
    {
        temp |= (rng() & 1ULL) << i;
    }

    bool const has_zero_state = rng() & 1ULL;
    SetBandSeparators(data_, temp);
    SetZeroState(data_, has_zero_state);

    num_bands_ = std::popcount(temp) + 1 + has_zero_state;

    CalculateHeaders(data_, num_bands_);
    //ensuring that there is no more bands than the permitted amount
    assert(((void)"",
        data_ ==
        (data_ & Utils::GenMask<uint64_t, Constants::General::NUM_BANDS>()))
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
    using Constants::BinaryString::SetZeroState;
    using Constants::BinaryString::CalculateHeaders;
    using Constants::BinaryString::SetBandSeparators;
    // {3, 3, 2}        |   {2,1,1,3,1}
    // XXX XXX XX       |   XX X X XXX X
    //  0010010         |    0111001
    num_bands_ = configurations.size();
    type temp{};
    for (view_type band : configurations)
    {
        temp = (temp << band) | static_cast<bool>(band);//skip 0 config
        // temp = (temp << band) | 1;
    }
    temp >>= 1;
    SetZeroState(data_, configurations.back() == 0);
    SetBandSeparators(data_, temp);
    CalculateHeaders(data_, num_bands_);

}

void BinString::ShuffleHeaders(std::mt19937& rng)
{
    using Constants::BinaryString::GetHeaders;
    using Constants::BinaryString::SetHeaders;

    type const headers = GetHeaders(data_, num_bands_);
    size_t const num_ones = std::popcount(headers);
    std::uniform_int_distribution<size_t> dist(0, num_bands_ - 1);
    type new_headers{};
    // for (size_t i = 0; i < num_ones; ++i)
    while (std::popcount(new_headers) != num_ones)
        new_headers |= static_cast<type>(1) << dist(rng);
    SetHeaders(data_, new_headers);
}

uint64_t BinString::GetUniformHeaderSize() const
{
    return static_cast<uint64_t>(std::ceil(std::log2(num_bands_)));
}
