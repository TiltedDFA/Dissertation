//
// Created by malikt on 12/6/24.
//
#include "BandConfig.hpp"

#include <cassert>
#include <bits/ranges_algo.h>

namespace
{

}
BandConfig::BandConfig(std::vector<uint32_t>&& band_config):
        band_config_(std::move(band_config)),
        header_config_(band_config_.size()),
        fitness_score_()
{
    uint32_t const header_bit_width = static_cast<uint32_t>(std::ceil(std::log2(band_config_.size() + 1)));
    if constexpr (Constants::General::HEADER_TYPE == HeaderType::Uniform)
    {
        std::ranges::fill(header_config_, header_bit_width);
    }
    else

    {
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
    }
    //verify that we have a "legal" configuration
    assert(((void)"Mismatched header - band config sizes", header_config_.size() == band_config_.size()));
    assert(((void)"Configuration band number exceeds the maximum", band_config_.size() <= Constants::General::MAX_BANDS));
    assert(
        ((void)"Mismatched band number-bit size count",
        std::accumulate(band_config_.cbegin(), band_config_.cend(), 0U) ==
        Constants::General::BIT_WIDTH
        ));
}

void BandConfig::ShuffleHeaders(std::mt19937& rng)
{
    std::ranges::shuffle(header_config_, rng);
}

FitnessScore BandConfig::GetFitnessScore() const
{
    return fitness_score_.value();
}

void BandConfig::SetFitnessScore(const FitnessScore fitness_score)
{
    fitness_score_ = fitness_score;
}

void BandConfig::ResetFitnessScore()
{
    fitness_score_.reset();
}

void BandConfig::Print() const
{
    std::string final{"BAND AND HEADER CONFIGURATION:\nUsing "};
    final += (Constants::General::HEADER_TYPE == HeaderType::Uniform ? "uniform" : "truncated");
    final += " headers\n";
    if (fitness_score_)
        final += std::format("Fitness score: {:3.5}\n", fitness_score_.value());
    else
        final += "Fitness score not set\n";
    final += std::format("Band length of: {}\n", band_config_.size());
    final += "Bands: \n";
    for (auto i = band_config_.rbegin(); i != std::prev(band_config_.rend()); ++i)
    {
        final += std::to_string(*i) + " | ";
    }
    final += std::to_string(*std::prev(band_config_.rend())) + '\n';
    final += "Headers: \n";
    for (auto i = header_config_.rbegin(); i != std::prev(header_config_.rend()); ++i)
    {
        final += std::to_string(*i) + " | ";
    }
    final += std::to_string(*std::prev(header_config_.rend())) + '\n';
    std::cout << final << std::endl;
}

size_t BandConfig::GetSize() const
{
    return header_config_.size();
}

std::string BandConfig::PrintShort() const
{
    std::string final{"B:("};
    for (auto i = band_config_.rbegin(); i != std::prev(band_config_.rend()); ++i)
    {
        final += std::to_string(*i) + "|";
    }
    final += std::to_string(*std::prev(band_config_.rend())) + ')';
    final += " H:(";
    for (auto i = header_config_.rbegin(); i != std::prev(header_config_.rend()); ++i)
    {
        final += std::to_string(*i) + "|";
    }
    final += std::to_string(*std::prev(header_config_.rend())) + ')';
    return final;
}
