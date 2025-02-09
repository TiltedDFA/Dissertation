//
// Created by malikt on 12/9/24.
//

#ifndef BINSTRING_HPP
#define BINSTRING_HPP

#include <bitset>
#include <cstdint>
#include <random>
#include "BandConfig.hpp"

class BinString
{
public:
    using type = Constants::BinaryString::Type;
    using view_type = Constants::BinaryString::ViewParamType;

    //its cool but hard to say whether strictly necessary, might remove later.
    //also might mess with shuffling headers in mutation...
    class ScopedRawBandReference
    {
    public:
        ScopedRawBandReference()=delete;
        ScopedRawBandReference(BinString& p):
            parent_(p),
            band_data_(parent_.GetBands())
        {}
        ~ScopedRawBandReference()
        {
            parent_.SetBands(band_data_);
        }
        type& operator*(){return band_data_;}
    private:
        BinString& parent_;
        type band_data_;
    };

    constexpr BinString(): data_(), num_bands_(0) {}

    //if this constructor is used, will generate a random band configuration
    explicit BinString(std::mt19937& rng);
    explicit BinString(std::vector<uint8_t> const& configurations);
    void ShuffleHeaders(std::mt19937& rng);
    uint64_t GetUniformHeaderSize()const;

    ScopedRawBandReference GetBandsScoped()
    {
        return {*this};
    }
private:
    void SetBands(view_type band){Constants::BinaryString::SetBandSeparators(data_, band);}
    type GetBands()const{return Constants::BinaryString::GetBandSeparators(data_); }
private:
    type data_;
    //A configuration's number of bands doesn't necessarily == max_bands, storing for optimisation
    size_t num_bands_;
};
#endif //BINSTRING_HPP
