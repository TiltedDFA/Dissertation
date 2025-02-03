//
// Created by malikt on 12/6/24.
//

#ifndef UTILS_HPP
#define UTILS_HPP

#define LTWOSB_DEBUG 0


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
#include <chrono>
#include <iostream>
#include <type_traits>

// #include "Types.hpp"

namespace Utils
{
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

    /**
     * Generates a bitmask up to the input value
     *
     * E.g. 3 returns 0b111
     * @param inp non-zero int type value
     * @return bit mask
     */
    template<typename T>
    requires std::is_integral_v<T>
    constexpr T GenMask(T inp)
    {
        return (static_cast<T>(1) << inp) - 1;
    }
    template<typename T, T inp>
    requires std::is_integral_v<T>
    consteval T GenMask()
    {
        return (static_cast<T>(1) << inp) - 1;
    }

    [[nodiscard]]
    constexpr uint64_t PlaceBit(size_t where) noexcept {return 1ULL << where;}
}


template<typename T>
class ScopedTimer
{
public:
    constexpr ScopedTimer()
    :start_(std::chrono::high_resolution_clock::now()), time_out(nullptr){}
    
    constexpr explicit ScopedTimer(uint64_t* ptr)
    :start_(std::chrono::high_resolution_clock::now()), time_out(ptr) {}

    constexpr ~ScopedTimer()
    {
        const std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
        if(time_out)
        {
            *time_out = uint64_t(std::chrono::duration_cast<T>(end-start_).count());
        }
        else
        {
            std::cout << "Timer lasted: " <<  std::chrono::duration_cast<T>(end-start_).count() << std::endl;
        }
    }
private:
    const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    uint64_t* time_out;
};
#endif //UTILS_HPP
