//
// Created by malikt on 12/6/24.
//
#include "BandConfig.hpp"

BandConfig::BandConfig(std::vector<uint32_t>&& band_config, HeaderType const header_type, std::reference_wrapper<GenPar const> gp):
        band_config_(std::move(band_config)),
        header_config_(band_config_.size()),
        header_type_(header_type),
        gen_par_(gp)
{
    uint32_t const header_bit_width = static_cast<uint32_t>(std::ceil(std::log2(band_config_.size() + 1)));
    if (header_type == HeaderType::Uniform)
    {
        std::ranges::fill(header_config_, header_bit_width);
    }
    else
    {
        std::cerr << "Unsupported feature (Truncated headers) used" << std::endl;
        std::abort();
        //doesn't seem to quite work yet
        uint32_t const unused = static_cast<uint32_t>(std::pow(2, header_bit_width)) - band_config_.size();
        for (uint32_t i = 0; i < band_config_.size(); i++)
        {
            if (i < unused){ header_config_[i] = header_bit_width - 1; }
            else {header_config_[i] = header_bit_width; }
        }
    }
    //verify that we have a "legal" configuration
    assert(((void)"Mismatched header - band config sizes", header_config_.size() == band_config_.size()));
    assert(((void)"Configuration band number exceeds the maximum", band_config_.size() <= gen_par_.get().Get(GenPar::Params::MaxBands)));
    assert(
        ((void)"Mismatched band number-bit size count",
        std::accumulate(band_config_.cbegin(), band_config_.cend(), 0U) ==
        gen_par_.get().Get(GenPar::Params::BitWidth))
        );
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
    final += (header_type_ == HeaderType::Uniform ? "uniform" : "truncated");
    final += " headers\n";
    if (fitness_score_)
        final += std::format("Fitness score: {:3.5}\n", fitness_score_.value());
    else
        final += "Fitness score not set\n";
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
