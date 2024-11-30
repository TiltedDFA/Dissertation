//
// Created by malikt on 11/30/24.
//
#include "l2sb_real.hpp"

int main()
{
    L2SBConfig config{};
    config.SetParam(L2SBConfig::Params::Quantisation, 11);
    config.SetParam(L2SBConfig::Params::BitWidth, 12);
    config.SetParam(L2SBConfig::Params::DataRange, 2048);
    config.SetParam(L2SBConfig::Params::FileDataReadLimit, std::numeric_limits<int64_t>::max());

    FileData<FileDataType::Unipolar> files("../Data/MITBIH", config);
    files.ReadCSVFiles(1);
    std::cin.get();
}