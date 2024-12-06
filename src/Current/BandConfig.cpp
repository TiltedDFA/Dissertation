//
// Created by malikt on 12/6/24.
//
#include "BandConfig.hpp"

BandConfig::BandConfig(std::vector<uint32_t>&& band_config, HeaderType const header_type):
        band_config_(band_config.cbegin(), band_config.cend()),
        header_config_(band_config.size()),
        header_type_(header_type)
{
    uint32_t const header_bit_width = static_cast<uint32_t>(std::ceil(std::log2(band_config.size() + 1)));
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
    assert(((void)"Configuration band number exceeds the maximum", band_config_.size() <= GenPar::Get(GenPar::Params::MaxBands)));
    assert(
        ((void)"Mismatched band number-bit size count",
        std::accumulate(band_config_.cbegin(), band_config_.cend(), 0U) ==
        GenPar::Get(GenPar::Params::BitWidth))
        );
}
void BandConfig::Print() const
{
    std::string final{"\n\nBAND AND HEADER CONFIGURATION:\nUsing "};
    final += (header_type_ == HeaderType::Uniform ? "uniform" : "truncated");
    final += " headers\n";
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
