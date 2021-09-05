//
// Created by Gleb Marin on 04.09.2021.
//

#ifndef FLOAT_MATRIX_UTILITY_HPP
#define FLOAT_MATRIX_UTILITY_HPP

namespace floatMatrix::utility {
    static constexpr float FLOAT_EPS = 1e-5;

    template <typename T>
    inline bool isEq(const T& a, const T& b) {
        return a == b;
    }

    template <>
    inline bool isEq(const float& a, const float& b) {
        bool res = std::abs(a - b) < FLOAT_EPS;
        return res;
    }
} // namespace floatMatrix::utility

#endif //FLOAT_MATRIX_UTILITY_HPP
