//
// Created by malikt on 12/6/24.
//

#ifndef GENETICS_HPP
#define GENETICS_HPP

#include "FitnessFunction.hpp"



#include <execution>
#include <random>
#include <algorithm>
#include <ranges>


template<size_t population_size, FileDataType file_data_type>
class GeneticAlgorithm
{
public:
    explicit GeneticAlgorithm(HeaderType const ht, std::reference_wrapper<GenPar const>&& gp, FileData<file_data_type>& data):
        header_type_(ht),
        mt_(std::random_device{}()),
        gen_par_(gp),
        bands_(InitBands()),
        data_(data)
    {}
    void EvaluatePopulation()
    {
        std::ranges::for_each(bands_,[this](BandConfig& band){band.SetFitnessScore(FindCompressionRatio(data_,band,gen_par_));});
        std::ranges::sort(bands_,[](BandConfig const& band1, BandConfig const& band2) -> bool {return band1.GetFitnessScore() > band2.GetFitnessScore();});
    }
    void PrintPopulation()
    {
        for (auto const& band : bands_){band.Print();}
    }
private:
    //Generates values uniformly from the probability space
    constexpr BandConfig GenerateRandomBand()
    {
        std::vector<uint32_t> bands{1};
        auto const bit_width = gen_par_.get().Get(GenPar::Params::BitWidth);
        for (int i = 0; i < bit_width - 1; ++i)
        {
            if (mt_() & 1U)
            {
                ++bands[bands.size()-1];
            }
            else
            {
                bands.push_back(1);
            }
        }
        return {std::move(bands), header_type_, gen_par_};
    }

    template <std::size_t... Indices>
    std::array<BandConfig, population_size> InitBandsImpl(std::index_sequence<Indices...>) {
        return { (static_cast<void>(Indices), GenerateRandomBand())... };
    }

    std::array<BandConfig, population_size> InitBands() {
        return InitBandsImpl(std::make_index_sequence<population_size>{});
    }
private:
    HeaderType const header_type_;
    std::mt19937 mt_;
    std::reference_wrapper<GenPar const> gen_par_;
    std::array<BandConfig, population_size> bands_;
    FileData<file_data_type>& data_;
};


#endif //GENETICS_HPP
