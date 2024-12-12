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
#include <random>
#include <vector>
class BandConfig
{
public:
    BandConfig()=delete;
    BandConfig(std::vector<uint32_t>&& band_config, std::reference_wrapper<GenPar const> gp);
    [[nodiscard]]
    std::vector<uint32_t> const& GetBandConfig() const{return band_config_;}
    [[nodiscard]]
    std::vector<uint32_t> const& GetHeaderConfig() const{return header_config_;}
    void ShuffleHeaders(std::mt19937& rng);
    [[nodiscard]] FitnessScore GetFitnessScore() const;
    void SetFitnessScore(FitnessScore fitness_score);
    void ResetFitnessScore();
    void Print() const;
    [[nodiscard]]
    std::string PrintShort() const;
private:
    std::vector<uint32_t> band_config_;
    std::vector<uint32_t> header_config_;
    std::optional<FitnessScore> fitness_score_;
    std::reference_wrapper<GenPar const> gen_par_;
};
#endif //BANDCONFIG_HPP
