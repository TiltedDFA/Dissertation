//
// Created by malikt on 12/6/24.
//

#ifndef GENETICS_HPP
#define GENETICS_HPP

#include "BinString.hpp"
#include "FitnessFunction.hpp"
#include "StackVector.hpp"
#include "Types.hpp"

#include <execution>
#include <random>
#include <algorithm>
#include <bitset>
#include <ranges>

using FileDataMan = FileData<Constants::General::FILE_DATA_TYPE>;
using BoltTourProb = StackVector<double, Constants::Genetic::BOLTZMANN_TOURNAMENT_SIZE>;
using BoltTourPart = StackVector<size_t, Constants::Genetic::BOLTZMANN_TOURNAMENT_SIZE>;
using NewPopVec = StackVector<BinString, Constants::Genetic::POPULATION_SIZE>;

class GeneticAlgorithm
{
public:
    explicit GeneticAlgorithm(FileDataMan& data):
        mt_(std::random_device{}()),
        bands_(InitBands()),
        data_(data),
        temperature_(Constants::Genetic::INITIAL_TEMPERATURE),
        best_performer_(mt_),
        best_performer_count_(0)
    {
        static_assert(Constants::Genetic::POPULATION_SIZE > 5, "Population size must be greater than 0");
    }
    void EvaluatePopulation()
    {
        std::ranges::for_each(bands_,[this](BinString& band){band.SetFitnessScore(FindCompressionRatio(data_, band));});
        std::ranges::sort(bands_, [](BinString const& band1, BinString const& band2) -> bool {return band1.GetFitnessScore() > band2.GetFitnessScore();});
    }

    [[nodiscard]]
    BoltTourProb CalcBoltzmannProbablities(BoltTourPart const& selected_participants) const
    {
        BoltTourProb boltzmannProbabilities;
        double total{};
        for (auto const selected_participant : selected_participants)
        {
            total += std::exp(bands_[selected_participant].GetFitnessScore() / temperature_);
        }
        for (auto const selected_participant : selected_participants)
        {
            boltzmannProbabilities.push_back(std::exp(bands_[selected_participant].GetFitnessScore() / temperature_) / total);
        }
        return boltzmannProbabilities;
    }
    //can't mark const since it changes the mersenne twister value
    std::pair<size_t const, size_t const> PickPairFromBoltzmannTournamentSelection()
    {
        std::uniform_int_distribution<size_t> participant_picker(0, Constants::Genetic::POPULATION_SIZE - 1);
        BoltTourPart participants;

        for (size_t i = 0; i < Constants::Genetic::BOLTZMANN_TOURNAMENT_SIZE; ++i)
        {
            participants.emplace_back(participant_picker(mt_));
        }

        BoltTourProb const boltzmannProbabilities = CalcBoltzmannProbablities(participants);
        std::discrete_distribution<> parent_picker(boltzmannProbabilities.cbegin(), boltzmannProbabilities.cend());

        return {participants[parent_picker(mt_)], participants[parent_picker(mt_)]};
    }

    /**
     *
     * To start with will disregard optimality and just try to make something that works.
     */
    void GenerateNewPopulation()
    {
        NewPopVec new_population;
        // std::vector<BandConfig> old_population(bands_.cbegin(), bands_.cend());
        //assumes sorted population

        // auto idx_seq = std::make_index_sequence<POPULATION_SIZE> { };
        for (size_t i = 0; i < Constants::Genetic::ELITE_COUNT; ++i)
        {
            //doing a "dumb" implementation for now, might wanna order by asc order
            //to later be able to do vector.back() and .pop()
            auto max_elem =
                std::max_element(bands_.cbegin(), bands_.cend(),
                    [](BinString const& band1, BinString const& band2)
                        {return band1.GetFitnessScore() < band2.GetFitnessScore();}
                        );
            new_population.emplace_back(*max_elem);
            // old_population.erase(max_elem);
        }
        std::uniform_int_distribution<> random_picker(0, Constants::Genetic::POPULATION_SIZE - 1);
        while (new_population.size() < Constants::Genetic::POPULATION_SIZE - (Constants::Genetic::RANDOM_IMMIGRATION_COUNT + Constants::Genetic::ELITE_COUNT))
        {
            auto const [fst, snd] = PickPairFromBoltzmannTournamentSelection();
            BinString new_band1{bands_[fst]}, new_band2{bands_[snd]};

            CrossOver(new_band1, new_band2);
            Mutate(new_band1);
            Mutate(new_band2);
            new_band1.AdjustToChange();
            new_band2.AdjustToChange();
            new_population.emplace_back(new_band1);
            new_population.emplace_back(new_band2);
        }
        for (size_t i = 0; i < Constants::Genetic::RANDOM_IMMIGRATION_COUNT; ++i)
        {
            //gens new random binstrings using the random constructor.
            new_population.emplace_back(mt_);
        }
        // if constexpr (Constants::General::HEADER_TYPE == HeaderType::Truncated)
        // {
        //     std::for_each(new_population.begin() + Constants::Genetic::ELITE_COUNT, new_population.end(),[this](BinString& band){band.ShuffleHeaders(mt_);});
        // }
        // for (size_t i = 0; i < new_population.size(); ++i)
        // {
        //     bands_[i] = new_population[i];
        // }
        // std::ranges::copy(std::as_const(new_population), bands_.begin());
        // std::copy(new_population.begin(), new_population.end(), bands_.begin());
        // auto band_it = bands_.begin();
        for (size_t i{}; i < new_population.size(); ++i)
        {
            bands_[i] = new_population[i];
        }
        // bands_ = std::move(new_population);
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
        // auto const active_bits = Constants::General::BIT_WIDTH - 1;
        // auto const cross_over_point = std::uniform_int_distribution<> {1U, static_cast<int>(active_bits)}(mt_);
        // // uint64_t const cross_over_mask = (1U << cross_over_point) - 1;
        // uint64_t const active_mask = Utils::GenMask(active_bits);
        // uint64_t const cross_over_lower_mask = Utils::GenMask(cross_over_point);
        // // uint64_t const cross_over_upper_mask = active_mask ^ cross_over_point;
        // uint64_t const cross_over_upper_mask = active_mask ^ cross_over_lower_mask;
        // uint64_t const result = (a.GetData() & cross_over_lower_mask) | (b.GetData() & cross_over_upper_mask);
        // return result;
        BinString::ScopedRawBandReference data_a = a.GetBandsScoped(), data_b = b.GetBandsScoped();

        std::uniform_int_distribution<uint8_t> point_picker(1, sizeof(BinString::type) * 8 - 1);

        BinString::type const mask = Utils::GenMask(static_cast<BinString::type>(point_picker(mt_)));

        BinString::type const a_upper = *data_a & ~mask;
        BinString::type const b_upper = *data_b & ~mask;

        *data_a = (*data_a & mask) | b_upper;
        *data_b = (*data_b & mask) | a_upper;
    }

    /**
     * Starting with simple mutation strategy
     * @param bs
     */
    void Mutate(BinString& bs)
    {
        constexpr auto active_bits_zero_indexed = Constants::General::BIT_WIDTH - 2;
        std::uniform_real_distribution<double> do_mutate(0.0, 1.0);
        std::uniform_int_distribution<> gen_mutate_point{0, active_bits_zero_indexed};
        BinString::ScopedRawBandReference bs_data = bs.GetBandsScoped();
        if (do_mutate(mt_) <= Constants::Genetic::MUTATION_CHANCE)
        {
            auto const mutation_point = gen_mutate_point(mt_);
            *bs_data ^= 1ULL << mutation_point;
        }
        bs.SetZeroBitState(do_mutate(mt_) <= 0.5);
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
        uint64_t total_time;
        {
            ScopedTimer<std::chrono::microseconds> total_timer(&total_time);
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
                if (bands_[0] == best_performer_)
                {
                    ++best_performer_count_;
                }
                else
                {
                    best_performer_ = bands_[0];
                    best_performer_count_ = 1;
                }
                std::cout << (std::format("[{:7}] completed in {:5.5} seconds, best fitness {:2.5} \t{}\t\t{}\ttemp: {:3.2}\t\tNumericValue: {}\t\tNumBands: {}", i + 1, static_cast<double>(time)/static_cast<double>(1e6), bands_[0].GetFitnessScore(), bands_[0].PrintShort(), bands_[2].PrintShort(), temperature_, static_cast<uint64_t>(bands_[0].GetBandsScoped().operator*()), bands_[0].GetNumBands())) << std::endl;
                std::cout << std::format("[{:7}] fitnesses {:2.5} {:2.5} {:2.5} {:2.5} {:2.5} {:2.5}\n", i + 1, bands_[1].GetFitnessScore(), bands_[2].GetFitnessScore(), bands_[3].GetFitnessScore(), bands_[4].GetFitnessScore(), bands_[5].GetFitnessScore(), bands_[6].GetFitnessScore()) << std::endl;
                temperature_ *= Constants::Genetic::TEMPERATURE_COOLING_RATE;
            }
        }
        // PRINTNL(std::format("Evolution completed.\n"));
        std::cout << std::format("Evolution completed.\nCompleted {} iterations in {:5.5} seconds ({:5.2} minutes)\n", Constants::Genetic::NUMBER_OF_RUNS, static_cast<double>(total_time)/static_cast<double>(1e6), static_cast<double>(total_time)/static_cast<double>(1e6 * 60)) << std::endl;
        EvaluatePopulation();
        bands_[0].Print();
        std::cout << std::format("Best performer found in {} iterations. fitness: {:5.5} seconds. Average time per iteration: {}",Constants::Genetic::NUMBER_OF_RUNS - best_performer_count_ ,best_performer_.GetFitnessScore() , static_cast<double>(total_time)/static_cast<double>(1e6 * Constants::Genetic::NUMBER_OF_RUNS) ) << std::endl;
    }
    void PrintPopulation() const
    {
        for (auto const& band : bands_){band.Print();}
    }
private:
    std::array<BinString, Constants::Genetic::POPULATION_SIZE> InitBands()
    {
        return [this]<size_t... i>(std::index_sequence<i...>)
        {
            return std::array<BinString, Constants::Genetic::POPULATION_SIZE>{(static_cast<void>(i), BinString(mt_))...};
        }(std::make_index_sequence<Constants::Genetic::POPULATION_SIZE>{});
    }
private:
    std::mt19937 mt_;
    std::array<BinString, Constants::Genetic::POPULATION_SIZE> bands_;
    FileDataMan& data_;
    double temperature_;
    BinString best_performer_;
    size_t best_performer_count_;
};


#endif //GENETICS_HPP
