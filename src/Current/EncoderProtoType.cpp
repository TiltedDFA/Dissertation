//
// Created by Malik T on 20/01/2025.
//

#include "EncoderProtoType.hpp"
/**
 *  N = 2 * BitWidth
 *  B = BitWidth - 1 ; Bands represented by divisors, 1 = present 0 otherwise.
 *  Z = TruncatedFormula(BitWidth)
 *  H = BitWidth ; Needs to show whether it stores Z or Z - 1, so will have same number of these as bands
 *               ; assert Active(H) == NumBands
 *
 *
 */