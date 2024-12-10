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


template<size_t POPULATION_SIZE, FileDataType FILE_DATA_TYPE>
class GeneticAlgorithm
{
public:
    explicit GeneticAlgorithm(std::reference_wrapper<GenPar const>&& gp, FileData<FILE_DATA_TYPE>& data):
        mt_(std::random_device{}()),
        gen_par_(gp),
        bands_(InitBands()),
        data_(data),
        //using the top 5 % of a population as the elites
        elite_count_(static_cast<uint64_t>(std::ceil(static_cast<double>(POPULATION_SIZE) * 0.05))),
        temperature_cooling_rate_(0.95),
        temperature_(10.0),
        boltzmann_tournament_size_(2)
    {
        static_assert(POPULATION_SIZE > 5, "Population size must be greater than 0");
    }
    void EvaluatePopulation()
    {
        std::ranges::for_each(bands_,[this](BandConfig& band){band.SetFitnessScore(FindCompressionRatio(data_,band,gen_par_));});
        std::ranges::sort(bands_,[](BandConfig const& band1, BandConfig const& band2) -> bool {return band1.GetFitnessScore() > band2.GetFitnessScore();});
    }

    std::vector<double>&& CalcBoltzmannProbablities(std::vector<BandConfig const*> const& selected_participants) const
    {
        std::vector<double> boltzmannProbabilities(boltzmann_tournament_size_);
        double total{};
        for (size_t i = 0; i < selected_participants.size(); ++i)
        {
            total += std::exp(selected_participants[i]->GetFitnessScore() / temperature_);
        }
        for (size_t i = 0; i < selected_participants.size(); ++i)
        {
            boltzmannProbabilities[i] = std::exp(selected_participants[i]->GetFitnessScore() / temperature_) / total;
        }
        return std::move(boltzmannProbabilities);
    }
    //can't mark const since it changes the mersenne twister value
    std::pair<BandConfig const&, BandConfig const&>&& PickPairFromBoltzmannTournamentSelection()
    {
        std::uniform_int_distribution<size_t> participant_picker(0,POPULATION_SIZE-1);
        std::vector<BandConfig const*> participants;
        for (size_t i = 0; i < boltzmann_tournament_size_; ++i)
        {
            participants.emplace_back(&bands_[participant_picker(i)]);
        }
        std::vector<double> const boltzmannProbabilities = CalcBoltzmannProbablities(participants);
        std::discrete_distribution<> parent_picker(boltzmannProbabilities.cbegin(), boltzmannProbabilities.cend());
        return {*participants[parent_picker(mt_)], *participants[parent_picker(mt_)]};
    }

    /**
     * TODO: Store elites in new pop and remove from selection, fill new pop while < current_pop.size() with
     * TODO: offspring made by parents selected from the boltzmann method
     * TODO: then move over the values in new pop to current pop
     */
    void GenerateNewPopulation()
    {
        std::vector<BandConfig> NewPopulation;
        //assumes sorted population

        auto idx_seq = std::make_index_sequence<POPULATION_SIZE> { };
        for (size_t i = 0; i < elite_count_; ++i)
        {
            //doing a "dumb" implamentation for now, might wanna order by asc order
            //to later be able to do vector.back() and .pop()
            NewPopulation.emplace_back(std::move(*std::ranges::max_element(bands_)));
        }

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
    /**
     * Single-point cross over
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

    /**
     * Starting with simple mutation strategy
     * @param bs
     */
    void Mutate(BinString& bs)
    {
        auto const active_bits_zero_indexed = gen_par_.get().Get(GenPar::Params::BitWidth) - 2;
        auto const mutation_point = std::uniform_int_distribution<> {0U, active_bits_zero_indexed}(mt_);
        uint64_t& bs_data = bs.GetData();
        bs_data ^= 1ULL << mutation_point;
    }
    void Run()
    {
        //population inited by ctor
        std::vector<BinString> new_bands(POPULATION_SIZE);
        // auto&& new_pop = SelectFromPopulation();
        for (auto const& band : bands_)
        {
            new_bands.emplace_back(band, gen_par_.get());
        }
        while (true)
        {
            // CrossOver(new_pop.first, new_pop.second);
            Mutate();
            EvaluatePopulation();
            GenerateNewPopulation();
            temperature_ *= temperature_cooling_rate_;
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
    std::array<BandConfig, POPULATION_SIZE> InitBandsImpl(std::index_sequence<Indices...>)
    {
        return { (static_cast<void>(Indices), GenerateRandomBand())... };
    }

    std::array<BandConfig, POPULATION_SIZE> InitBands()
    {
        return InitBandsImpl(std::make_index_sequence<POPULATION_SIZE>{});
    }
private:
    std::mt19937 mt_;
    std::reference_wrapper<GenPar const> gen_par_;
    std::array<BandConfig, POPULATION_SIZE> bands_;
    FileData<FILE_DATA_TYPE>& data_;
    uint64_t const elite_count_;
    double const temperature_cooling_rate_;
    double temperature_;
    size_t const boltzmann_tournament_size_;
};


#endif //GENETICS_HPP
