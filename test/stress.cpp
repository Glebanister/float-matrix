//
// Created by Gleb Marin on 04.09.2021.
//

#include <random>

#include <boost/compute/device.hpp>

#include <doctest.h>

#include "CooMatrix.hpp"

TEST_SUITE("CooMatrix Stress") {

    using FloatMatrix = floatMatrix::CooMatrix<float>;
    static std::mt19937 rand{std::random_device{}()};

    template <typename T>
    std::size_t getRandInRange(const T& begin, const T& end) {
        return rand() % (end - begin + 1) + begin;
    }

    float getRandFloat() {
        return std::uniform_real_distribution(0.f, 1000000000.f)(rand);
    }

    FloatMatrix generateMatrix(std::size_t rows, std::size_t cols, std::size_t values) {
        FloatMatrix mtx;
        for (std::size_t i = 0; i < values; ++i) {
            mtx.set(getRandInRange(0UL, rows), getRandInRange(0UL, cols), getRandFloat());
        }
        return mtx;
    }

    struct MatrixPairs {
        FloatMatrix a, b, aCopy, bCopy;
    };

    MatrixPairs generateMatrixPairs(std::size_t rows, std::size_t cols, std::size_t values) {
        FloatMatrix a = generateMatrix(rows, cols, values);
        FloatMatrix b = generateMatrix(rows, cols, values);
        return {a, b, a, b};
    }

    TEST_CASE("Small") {
        auto [a, b, aCopy, bCopy] = generateMatrixPairs(100, 100, 150);

        boost::compute::device device = boost::compute::system::default_device();
        boost::compute::context context(device);
        boost::compute::command_queue queue(context, device);

        a.add(b);
        aCopy.add(bCopy, queue);

        CHECK_EQ(a, aCopy);
    }
}
