//
// Created by malikt on 11/30/24.
//
#include "l2sb_real.hpp"

int main()
{
    L2SBConfig config{};
    config  .SetBitWidth(12)
            .SetDataRange(2048.0)
            .SetQuantisation(11.0);

    FileData<FileDataType::Unipolar> files("../Data/MITBIH", config);
    files.ReadCSVFiles(1);
    std::cin.get();
}