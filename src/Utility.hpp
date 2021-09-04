//
// Created by Gleb Marin on 04.09.2021.
//

#ifndef FLOAT_MATRIX_UTILITY_HPP
#define FLOAT_MATRIX_UTILITY_HPP

#include <iostream>

namespace floatMatrix::utility {
    static constexpr float FLOAT_EPS = 1e-5;

    template <typename T>
    bool isEq(const T& a, const T& b) {
        return a == b;
    }

    template <>
    bool isEq(const float& a, const float& b) {
        bool res = std::abs(a - b) < FLOAT_EPS;
        std::cerr << a << ' ' << b << ' ' << (res ? "true" : "false") << std::endl;
        return res;
    }
}

#endif //FLOAT_MATRIX_UTILITY_HPP
