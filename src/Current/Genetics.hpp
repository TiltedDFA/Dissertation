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
    explicit GeneticAlgorithm(FileData<FILE_DATA_TYPE>& data):
        mt_(std::random_device{}()),
        bands_(InitBands()),
        data_(data),
        temperature_(Constants::Genetic::INITIAL_TEMPERATURE)
    {
        static_assert(POPULATION_SIZE > 5, "Population size must be greater than 0");
    }
    void EvaluatePopulation()
    {
        std::ranges::for_each(bands_,[this](BandConfig& band){band.SetFitnessScore(FindCompressionRatio(data_,band));});
        std::ranges::sort(bands_,[](BandConfig const& band1, BandConfig const& band2) -> bool {return band1.GetFitnessScore() > band2.GetFitnessScore();});
    }

    [[nodiscard]]
    std::vector<double> CalcBoltzmannProbablities(std::vector<BandConfig const*> const& selected_participants) const
    {
        std::vector<double> boltzmannProbabilities(Constants::Genetic::BOLTZMANN_TOURNAMENT_SIZE);
        double total{};
        for (auto const selected_participant : selected_participants)
        {
            total += std::exp(selected_participant->GetFitnessScore() / temperature_);
        }
        for (size_t i = 0; i < selected_participants.size(); ++i)
        {
            boltzmannProbabilities[i] = std::exp(selected_participants[i]->GetFitnessScore() / temperature_) / total;
        }
        return boltzmannProbabilities;
    }
    //can't mark const since it changes the mersenne twister value
    std::pair<BandConfig const&, BandConfig const&> PickPairFromBoltzmannTournamentSelection()
    {
        std::uniform_int_distribution<size_t> participant_picker(0,POPULATION_SIZE-1);
        std::vector<BandConfig const*> participants;
        for (size_t i = 0; i < Constants::Genetic::BOLTZMANN_TOURNAMENT_SIZE; ++i)
        {
            participants.emplace_back(&bands_[participant_picker(mt_)]);
        }
        std::vector<double> const boltzmannProbabilities = CalcBoltzmannProbablities(participants);
        std::discrete_distribution<> parent_picker(boltzmannProbabilities.cbegin(), boltzmannProbabilities.cend());
        return {*participants[parent_picker(mt_)], *participants[parent_picker(mt_)]};
    }

    /**
     *
     * To start with will disregard optimality and just try to make something that works.
     */
    void GenerateNewPopulation()
    {
        std::vector<BandConfig> new_population;
        // std::vector<BandConfig> old_population(bands_.cbegin(), bands_.cend());
        //assumes sorted population

        // auto idx_seq = std::make_index_sequence<POPULATION_SIZE> { };
        for (size_t i = 0; i < Constants::Genetic::ELITE_COUNT; ++i)
        {
            //doing a "dumb" implementation for now, might wanna order by asc order
            //to later be able to do vector.back() and .pop()
            auto max_elem =
                std::max_element(bands_.cbegin(), bands_.cend(),
                    [](BandConfig const& band1, BandConfig const& band2)
                        {return band1.GetFitnessScore() < band2.GetFitnessScore();}
                        );
            new_population.emplace_back(*max_elem);
            // old_population.erase(max_elem);
        }
        std::uniform_int_distribution<> random_picker(0, POPULATION_SIZE - 1);
        while (new_population.size() < POPULATION_SIZE - Constants::Genetic::RANDOM_IMMIGRATION_COUNT)
        {
            auto const [fst, snd] = PickPairFromBoltzmannTournamentSelection();
            auto&& _ = CrossOver({fst}, {snd});
            // auto&& _ = CrossOver({bands_[random_picker(mt_)], gen_par_}, {bands_[random_picker(mt_)], gen_par_});
            new_population.emplace_back(Mutate(std::move(_)).Destruct());
        }
        for (size_t i = 0; i < Constants::Genetic::RANDOM_IMMIGRATION_COUNT; ++i)
        {
            new_population.emplace_back(GenerateRandomBand());
        }
        // if ((HeaderType)gen_par_.get().Get(GenPar::Params::HeaderType) == HeaderType::Truncated)
        if (true)
        {
            std::for_each(new_population.begin() + Constants::Genetic::ELITE_COUNT, new_population.end(),[this](BandConfig& band){band.ShuffleHeaders(mt_);});
        }
        for (size_t i = 0; i < new_population.size(); ++i)
        {
            bands_[i] = new_population[i];
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
    [[nodiscard]]
    BinString CrossOver(BinString const& a, BinString const& b)
    {
        auto const active_bits = Constants::General::BIT_WIDTH - 1;
        auto const cross_over_point = std::uniform_int_distribution<> {1U, static_cast<int>(active_bits)}(mt_);
        // uint64_t const cross_over_mask = (1U << cross_over_point) - 1;
        uint64_t const active_mask = Utils::GenMask(active_bits);
        uint64_t const cross_over_lower_mask = Utils::GenMask(cross_over_point);
        // uint64_t const cross_over_upper_mask = active_mask ^ cross_over_point;
        uint64_t const cross_over_upper_mask = active_mask ^ cross_over_lower_mask;
        uint64_t const result = (a.GetData() & cross_over_lower_mask) | (b.GetData() & cross_over_upper_mask);
        return {result};
    }

    /**
     * Starting with simple mutation strategy
     * @param bs
     */
    BinString&& Mutate(BinString&& bs)
    {
        auto const active_bits_zero_indexed = Constants::General::BIT_WIDTH - 2;
        std::uniform_real_distribution<double> do_mutate(0.0, 1.0);
        std::uniform_int_distribution<> gen_mutate_point{0U, static_cast<int>(active_bits_zero_indexed)};
        uint64_t& bs_data = bs.GetData();
        while (do_mutate(mt_) <= Constants::Genetic::MUTATION_CHANCE)
        {
            auto const mutation_point = gen_mutate_point(mt_);
            bs_data ^= 1ULL << mutation_point;
        }
        return std::move(bs);
    }
    void Run()
    {
        //population inited by ctor
        // std::vector<BinString> new_bands(POPULATION_SIZE);
        // // auto&& new_pop = SelectFromPopulation();
        // for (auto const& band : bands_)
        // {
        //     new_bands.emplace_back(band, gen_par_.get());
        // }
        EvaluatePopulation();
        for (size_t i{}; i < Constants::Genetic::NUMBER_OF_RUNS; ++i)
        {
            // CrossOver(new_pop.first, new_pop.second);
            // Mutate();
            uint64_t time;
            {
                ScopedTimer<std::chrono::microseconds> timer(&time);
                GenerateNewPopulation();
                EvaluatePopulation();
            }
            std::cout << (std::format("[{:7}] completed in {:5.5} seconds, best fitness {:2.5} \t{}\t\t{}\ttemp: {:3.2}", i + 1, static_cast<double>(time)/static_cast<double>(1e6), bands_[0].GetFitnessScore(), bands_[0].PrintShort(), bands_[2].PrintShort(), temperature_)) << std::endl;
            std::cout << std::format("[{:7}] fitnesses {:2.5} {:2.5} {:2.5} {:2.5} {:2.5} {:2.5}\n", i + 1, bands_[1].GetFitnessScore(), bands_[2].GetFitnessScore(), bands_[3].GetFitnessScore(), bands_[4].GetFitnessScore(), bands_[5].GetFitnessScore(), bands_[6].GetFitnessScore()) << std::endl;
            temperature_ *= Constants::Genetic::TEMPERATURE_COOLING_RATE;
        }
        // PRINTNL(std::format("Evolution completed.\n"));
        std::cout << "Evolution completed.\n" << std::endl;
        EvaluatePopulation();
        bands_[0].Print();
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
        auto const bit_width = Constants::General::BIT_WIDTH;
        for (size_t i = 0; i < bit_width - 1; ++i)
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
        return {std::move(bands)};
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
    std::array<BandConfig, POPULATION_SIZE> bands_;
    FileData<FILE_DATA_TYPE>& data_;
    double temperature_;
};


#endif //GENETICS_HPP
