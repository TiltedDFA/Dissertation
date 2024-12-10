//
// Created by malikt on 12/6/24.
//

#ifndef GENETICS_HPP
#define GENETICS_HPP

#include "BinString.hpp"
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
    {
        static_assert(population_size > 5, "Population size must be greater than 0");
    }
    void EvaluatePopulation()
    {
        std::ranges::for_each(bands_,[this](BandConfig& band){band.SetFitnessScore(FindCompressionRatio(data_,band,gen_par_));});
        std::ranges::sort(bands_,[](BandConfig const& band1, BandConfig const& band2) -> bool {return band1.GetFitnessScore() > band2.GetFitnessScore();});
    }
    void SelectFromPopulation()
    {

    }
    void CrossOver(BinString a, BinString b)
    {
        auto const active_bits = gen_par_.get().Get(GenPar::Params::BitWidth) - 1;
        auto const cross_over_point = std::uniform_int_distribution<> {1U, active_bits}(mt_);
        uint64_t const cross_over_mask = (1U << cross_over_point) - 1;
        uint64_t& a_data = a.GetData();
        uint64_t& b_data = b.GetData();
        uint64_t const difference_mask = a_data ^ b_data;
        a_data ^= difference_mask & cross_over_mask;
        b_data ^= difference_mask & cross_over_mask;
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
    std::array<BandConfig, population_size> InitBandsImpl(std::index_sequence<Indices...>)
    {
        return { (static_cast<void>(Indices), GenerateRandomBand())... };
    }

    std::array<BandConfig, population_size> InitBands()
    {
        return InitBandsImpl(std::make_index_sequence<population_size>{});
    }
private:
    HeaderType const header_type_;
    std::mt19937 mt_;
    std::reference_wrapper<GenPar const> gen_par_;
    std::array<BandConfig, population_size> bands_;
    FileData<file_data_type>& data_;
    constexpr uint8_t A_B = gen_par_.get().Get(GenPar::Params::BitWidth) - 1;
};


#endif //GENETICS_HPP
