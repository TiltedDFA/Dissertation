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
#include <bitset>
#include <ranges>


template<size_t population_size, FileDataType file_data_type>
class GeneticAlgorithm
{
public:
    explicit GeneticAlgorithm(std::reference_wrapper<GenPar const>&& gp, FileData<file_data_type>& data):
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
    std::vector<std::pair<BinString, BinString>>&& SelectFromPopulation()
    {
        //population_size >> 1 approximates half population for the reserve
        std::vector<std::pair<BinString, BinString>> results(population_size >> 1);
        //elitism, using saving best 2.
        results.emplace_back({{bands_[0], gen_par_},{bands_[1], gen_par_}});

    }

    /**
     * rough sketch of the pipeline
     * BC = BandConfig
     * BS = BinString
     * Init():BC ->
     * ::_LOOP
     * Eval(BC):BC ->
     * Selection(BC):BS ->
     * CrossOver(BS):BS ->
     * Mutation(BS):BS ->
     * ConvertAll(BS):BC ->
     * GOTO _LOOP
     *
     */
    void CrossOver(BinString& a, BinString& b)
    {
        auto const active_bits = gen_par_.get().Get(GenPar::Params::BitWidth) - 1;
        auto const cross_over_point = std::uniform_int_distribution<> {1U, active_bits}(mt_);
        // uint64_t const cross_over_mask = (1U << cross_over_point) - 1;
        uint64_t const cross_over_mask = (1ULL << cross_over_point) - 1;
        PRINTBS(cross_over_mask);
        uint64_t& a_data = a.GetData();
        uint64_t& b_data = b.GetData();
        uint64_t const difference_mask = a_data ^ b_data;
        PRINTBS(difference_mask);
        a_data ^= difference_mask & cross_over_mask;
        b_data ^= difference_mask & cross_over_mask;
    }
    void Mutate(BinString& bs)
    {

    }
    void Run()
    {
        //population inited by ctor
        auto&& new_pop = SelectFromPopulation();
        while (true)
        {
            // CrossOver(new_pop.first, new_pop.second);
            Mutate();
            EvaluatePopulation();
            new_pop = SelectFromPopulation();
        }
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
            if (mt_() & 1ULL)
            {
                bands.push_back(1);
            }
            else
            {
                ++bands[bands.size()-1];
            }
        }
        return {std::move(bands), gen_par_};
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
    std::mt19937 mt_;
    std::reference_wrapper<GenPar const> gen_par_;
    std::array<BandConfig, population_size> bands_;
    FileData<file_data_type>& data_;
};


#endif //GENETICS_HPP
