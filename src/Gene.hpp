//
// Created by malikt on 11/19/24.
//

#ifndef GENE_HPP
#define GENE_HPP
#include <string>

namespace Genetics
{

    template<size_t NUM_BITS>
    class Gene
    {
    public:
        explicit Gene(int bands):
            bands_(bands),
            band_widths_(new int[bands]{}),
            header_widths_(new int[bands]())
        {};
        ~Gene()
        {
            delete[] band_widths_,
            delete[] header_widths_;
        }
        std::string Stringify() const;
    private:
        const int bands_;
        int* band_widths_;
        int* header_widths_;
    };


} // Genetics

#endif //GENE_HPP
