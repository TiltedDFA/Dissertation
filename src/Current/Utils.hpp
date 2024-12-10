//
// Created by malikt on 12/6/24.
//

#ifndef UTILS_HPP
#define UTILS_HPP

#define LTWOSB_DEBUG 1


#if LTWOSB_DEBUG == 1
#define PRINTNL(x) std::cout << (x) << std::endl
#define PRINTNLF(x, y) std::cout << std::format((x),(y)) << std::endl
#define ERRNL(x) std::cerr << (x) << std::endl
#define AT(x) .at((x))
//print bitset || binary string
#define PRINTBS(x) (std::cout << std::bitset<sizeof((x))*8>(x) << std::endl)
#else
#define NDEBUG
#define PRINTNL(x)
#define PRINTNLF(x, y)
#define ERRNL(x)
#define AT(x) [(x)]
#define PRINTBS(X)
#endif

#include <cassert>
#include <type_traits>

#include "Types.hpp"

constexpr double ByteToMB = 1024.0 * 1024.0;

template<typename T>
requires std::is_integral_v<T>
constexpr uint8_t FindMS1B(T inp)
{
    uint8_t count{};
    while (inp >>= 1)
        ++count;

    return count;
}
#endif //UTILS_HPP
