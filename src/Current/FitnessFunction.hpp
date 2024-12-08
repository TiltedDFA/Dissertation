//
// Created by malikt on 11/26/24.
//

#ifndef L2SB_REAL_HPP
#define L2SB_REAL_HPP



#include <filesystem>
#include <string_view>
#include <vector>
#include <string>
#include <cinttypes>
#include <fstream>
#include <format>
#include <iostream>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <complex>
#include <numeric>

#include "FileData.hpp"

template <FileDataType type>
inline double FindCompressionRatio(FileData<type> const& file_data, BandConfig const& band_config, GenPar const& gen_par)
{
    if (file_data.GetFileData().empty()) return -1;
    double compression_ratio{};
    std::vector<uint32_t> const& bands   = band_config.GetBandConfig();
    std::vector<uint32_t> const& headers = band_config.GetHeaderConfig();
    std::vector<RawDataType> const& data = file_data.GetFileData();
    uint64_t const data_bit_width = gen_par.Get(GenPar::Params::BitWidth);
    uint64_t bit_count = headers.back() + data_bit_width;

    auto const init_bands_cum = [&bands]() -> std::vector<uint32_t>
    {
        std::vector<uint32_t> result(bands.size()+1);
        result[0] = 0;
        for (uint64_t i = 1; i < bands.size()+1; ++i)
        {
            result[i] = bands[i-1] + result[i - 1];
        }
        return result;
    };
    std::vector<uint32_t> const bands_cum(init_bands_cum());

    for (auto it = data.cbegin(); it != std::prev(data.cend()); ++it)
    {
        RawDataType const difference = *it ^ *(it + 1);
        uint32_t const msb_loc = FindMS1B(difference);
        size_t idx{};

        while (msb_loc > bands_cum[idx])++idx;

        bit_count += bands_cum[idx] + headers[idx - bool(msb_loc)];
    }
    compression_ratio = static_cast<double>(data_bit_width * data.size()) / static_cast<double>(bit_count);
    PRINTNLF("Raw:\t\t\t\t{}", data_bit_width * data.size());
    PRINTNLF("Compressed:\t\t\t{}", bit_count);
    return compression_ratio;
}
#endif //L2SB_REAL_HPP
