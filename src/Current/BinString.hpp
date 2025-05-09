//
// Created by malikt on 12/9/24.
//

#ifndef BINSTRING_HPP
#define BINSTRING_HPP

#include <bitset>
#include <cstdint>
#include <random>
#include "StackVector.hpp"
#include "Types.hpp"


class BinString
{
public:
    using type = Constants::BinaryString::Type;
    using view_type = Constants::BinaryString::ViewParamType;
    using svec = StackVector<size_t, Constants::General::BIT_WIDTH+1>;

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
            parent_.CalculateNumBands();
        }
        type& operator*(){return band_data_;}
        type operator*()const{return band_data_;}
    private:
        BinString& parent_;
        type band_data_;
    };

    constexpr BinString():
        data_(),
        num_bands_(0),
        fitness_score_(0)
    {}
    BinString(BinString const& p)=default;
    BinString(BinString&& p)=default;
    BinString& operator=(BinString const& p)=default;
    BinString& operator=(BinString&& p)=default;

    bool operator==(BinString const& other) const
    {
        return data_ == other.data_;
    }
    //if this constructor is used, will generate a random band configuration
    explicit BinString(std::mt19937& rng);
    explicit BinString(size_t band_sep_str, bool has_zero_state);
    explicit BinString(std::vector<uint8_t> const& configurations);
    void ShuffleHeaders(std::mt19937& rng);

    [[nodiscard]]
    uint64_t GetUniformHeaderSize()const;

    void CalculateNumBands()
    {
        num_bands_ = std::popcount(GetBands()) + 1 + HasZeroBand();
    }
    [[nodiscard]]
    size_t GetNumBands() const
    {
        return num_bands_;
    }
    void SetZeroBitState(bool const v)
    {
        Constants::BinaryString::SetZeroState(data_, v);
    }
    [[nodiscard]]
    bool HasZeroBand()const
    {
        return Constants::BinaryString::HasZeroState(data_);
    }
    [[nodiscard]]
    type GetBands()const noexcept
    {
        auto const ret = Constants::BinaryString::GetBandSeparators(data_);
        return ret;
    }
    [[nodiscard]]
    type GetHeaders()const
    {
        return Constants::BinaryString::GetHeaders(data_, num_bands_);
    }
    ScopedRawBandReference GetBandsScoped()
    {
        return {*this};
    }

    [[nodiscard]]
    FitnessScore GetFitnessScore()const { return fitness_score_;}

    void SetFitnessScore(FitnessScore const fitness_score) { fitness_score_ = fitness_score;}

    [[nodiscard]]
    svec GenHeaderVec()const
    {
        svec headers;
        auto const reg_header_size = GetUniformHeaderSize();
        auto const headers_sub = GetHeaders();
        for (size_t i = 0; i < num_bands_; ++i)
            headers.push_back(reg_header_size - ((headers_sub >> i) & 1));
        return headers;
    }

    void AdjustToChange()
    {
        CalculateNumBands();
        Constants::BinaryString::CalculateHeaders(data_, num_bands_);
    }



    [[nodiscard]]
    svec GenBandsVec()const
    {
        auto const bands = GetBands();
        if (bands == 0) return {Constants::General::BIT_WIDTH};
        svec ret;

        if (Constants::BinaryString::HasZeroState(data_))
        {
            ret.push_back(0);
        }

        type tmp{1};
        for (uint64_t i = 0; i < Constants::BinaryString::Util::BAND_BIT_END_VAL; ++i)
        {
            if ((bands >> i) & 1)
            {
                ret.push_back(tmp);
                tmp = 1;
            }
            else
            {
                ++tmp;
            }
        }
        if (tmp)
            ret.push_back(tmp);
        return ret;
    }
    [[nodiscard]]
    std::pair<svec, svec> GetAll() const
    {
        auto const bands = GenBandsVec();
        auto const headers = GenHeaderVec();
        // assert(((void)"Header and band generated vectors do not have the same size", bands.size() == headers.size()));
        return {svec(bands), svec(headers)};
    }
    void Print()const
    {
        auto const [bands, headers] = GetAll();
        std::string final{"BAND AND HEADER CONFIGURATION:\nUsing "};
        final += (Constants::General::HEADER_TYPE == HeaderType::Uniform ? "uniform" : "truncated");
        final += " headers\n";

        final += std::format("Fitness score: {:3.5}\n", GetFitnessScore());

        final += std::format("Band length of: {}\n", bands.size());
        final += "Bands: \n";
        for (auto i = bands.rbegin(); i != std::prev(bands.rend()); ++i)
        {
            final += std::to_string(*i) + " | ";
        }
        final += std::to_string(*std::prev(bands.rend())) + '\n';
        final += "Headers: \n";
        for (auto i = headers.rbegin(); i != std::prev(headers.rend()); ++i)
        {
            final += std::to_string(*i) + " | ";
        }
        final += std::to_string(*std::prev(headers.rend())) + '\n';
        std::cout << final << std::endl;
    }
    [[nodiscard]]
    std::string PrintShort() const
    {
        auto const [bands, headers] = GetAll();
        std::string final{"B:("};
        for (auto i = bands.rbegin(); i != std::prev(bands.rend()); ++i)
        {
            final += std::to_string(*i) + "|";
        }
        final += std::to_string(*std::prev(bands.rend())) + ')';
        final += " H:(";
        for (auto i = headers.rbegin(); i != std::prev(headers.rend()); ++i)
        {
            final += std::to_string(*i) + "|";
        }
        final += std::to_string(*std::prev(headers.rend())) + ')';
        final += (HasZeroBand() ? "\t HAS_Z" : "\t NO_Z");
        return final;
    }
private:
    void SetBands(view_type band){Constants::BinaryString::SetBandSeparators(data_, band);}
private:
    type data_;
    //A configuration's number of bands doesn't necessarily == max_bands, storing for optimisation
    size_t num_bands_;
    FitnessScore fitness_score_;
};
#endif //BINSTRING_HPP
