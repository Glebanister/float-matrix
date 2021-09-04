//
// Created by Gleb Marin on 04.09.2021.
//

#include <sstream>

#include <doctest.h>

#include "CooMatrix.hpp"

namespace floatMatrix {

    using FloatMatrix = floatMatrix::CooMatrix<float>;
    using Compressed = typename FloatMatrix::Compressed;

    TEST_SUITE("CooMatrixOverall") {

        bool eqFloat(float a, float b) {
            return std::abs(a - b) <= 1e-7;
        }

        TEST_CASE("Serialize & Deserialize") {
            FloatMatrix mtx(std::vector<FloatMatrix::CoordCell>{
                    {{0, 0}, 1.0},
                    {{1, 4}, 6.0},
                    {{2, 3}, 0.0},
                    {{8, 0}, 9.0},
                    {{3, 1}, 1.1},
                    {{4, 2}, 13.4},
            });
            std::stringstream ss;
            ss << mtx;
            FloatMatrix deserialized;
            ss >> deserialized;
            CHECK(eqFloat(deserialized.get(0, 0), 1.0));
            CHECK(eqFloat(deserialized.get(1, 4), 6.0));
            CHECK(eqFloat(deserialized.get(2, 3), 0.0));
            CHECK(eqFloat(deserialized.get(6, 7), 0.0));
            CHECK(eqFloat(deserialized.get(8, 0), 9.0));
            CHECK(eqFloat(deserialized.get(3, 1), 1.1));
            CHECK(eqFloat(deserialized.get(4, 2), 13.4));
        }

        TEST_CASE("Simple Addition") {
            FloatMatrix A({
                {{0, 0}, 1.1},
                {{0, 1}, 2.2},
                {{0, 2}, 3.3},

                {{1, 0}, 4.4},
                {{1, 1}, 5.5},
                {{1, 2}, 6.6},

                {{2, 0}, 7.7},
                {{2, 1}, 8.8},
                {{2, 2}, 9.9},
                });
            FloatMatrix B({
                {{0, 0}, 10.1},
                {{0, 1}, 20.2},
                {{0, 2}, 30.3},

                {{1, 0}, 40.4},
                {{1, 1}, 50.5},
                {{1, 2}, 60.6},

                {{2, 0}, 70.7},
                {{2, 1}, 80.8},
                {{2, 2}, 90.9},
                });

            FloatMatrix AplusB({
                {{0, 0}, 11.2},
                {{0, 1}, 22.4},
                {{0, 2}, 33.6},

                {{1, 0}, 44.8},
                {{1, 1}, 56.0},
                {{1, 2}, 67.2},

                {{2, 0}, 78.4},
                {{2, 1}, 89.6},
                {{2, 2}, 100.8},
                });

            CHECK_EQ(A += B, AplusB);
        }
    }


} // namespace floatMatrix
