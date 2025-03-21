//
// Created by Malik T on 10/02/2025.
//

#ifndef STACKVECTOR_HPP
#define STACKVECTOR_HPP

#include "Types.hpp"
#include <array>

template<class T, size_t N>
class StackVector
{
public:
    using arr_it = typename std::array<T, N>::iterator;
    using carr_it = typename std::array<T, N>::const_iterator;
    using cret_val = std::conditional_t<std::is_integral_v<T>, T, T const&>;

    StackVector()=default;

    template<typename... Args>
    StackVector(Args&&... a)
        :   StackVector()
    {
        emplace_back(std::forward<Args>(a)...);
    }


    void push_back(cret_val v)
    {
        data_[idx_++] = v;
    }
    cret_val back() const
    {
        return data_[idx_ - 1];
    }
    T& back()
    {
        return data_[idx_ - 1];
    }
    template<class... Args>
    void emplace_back(Args&&... v)
    {
        std::construct_at(&data_[idx_++], std::forward<Args>(v)...);
    }
    cret_val operator[](size_t i) const
    {
        return data_[i];
    }
    T& operator[](size_t i)
    {
        return data_[i];
    }
    arr_it begin()
    {
        return data_.begin();
    }
    carr_it begin() const
    {
        return data_.cbegin();
    }
    carr_it cbegin() const
    {
        return begin();
    }
    arr_it end()
    {
        return data_.begin() + idx_;
    }
    carr_it end() const
    {
        return data_.cbegin() + idx_;
    }
    carr_it cend() const
    {
        return end();
    }
    [[nodiscard]]
    size_t size() const
    {
        return idx_;
    }
    std::reverse_iterator<arr_it> rbegin()
    {
        return std::reverse_iterator(end());
    }
    std::reverse_iterator<carr_it> rbegin() const
    {
        return std::reverse_iterator(end());
    }
    std::reverse_iterator<carr_it> crbegin() const
    {
        return rbegin();
    }
    std::reverse_iterator<arr_it> rend()
    {
        return std::reverse_iterator(begin());
    }
    std::reverse_iterator<carr_it> rend() const
    {
        return std::reverse_iterator(begin());
    }
    std::reverse_iterator<carr_it> crend() const
    {
        return rend();
    }
private:
    std::array<T, N> data_{};
    size_t idx_{};
};

#endif //STACKVECTOR_HPP
