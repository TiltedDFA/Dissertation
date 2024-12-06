//
// Created by malikt on 12/6/24.
//

#ifndef BANDCONFIG_HPP
#define BANDCONFIG_HPP


#include "GeneralParameters.hpp"

#include <bits/ranges_algobase.h>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

class BandConfig
{
public:
    BandConfig()=delete;
    BandConfig(std::vector<uint32_t>&& band_config, HeaderType header_type);
    [[nodiscard]]
    std::vector<uint32_t> const& GetBandConfig() const{return band_config_;}
    [[nodiscard]]
    std::vector<uint32_t> const& GetHeaderConfig() const{return header_config_;}
    void Print() const;
private:
    std::vector<uint32_t> const band_config_;
    std::vector<uint32_t> header_config_;
    HeaderType const header_type_;
};
#endif //BANDCONFIG_HPP
