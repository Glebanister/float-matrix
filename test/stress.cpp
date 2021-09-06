//
// Created by Gleb Marin on 04.09.2021.
//

#include <random>

#include <boost/compute/device.hpp>

#include <doctest.h>

#include "CooMatrix.hpp"

TEST_SUITE("CooMatrix Stress") {

    using FloatMatrix = floatMatrix::CooMatrix;
    static std::mt19937 rand{std::random_device{}()};

    template <typename T>
    std::size_t getRandInRange(const T& begin, const T& end) {
        return rand() % (end - begin) + begin;
    }

    float getRandFloat() {
        return std::uniform_real_distribution(-20.f, 20.f)(rand);
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

    void executeStress(std::size_t rows, std::size_t cols, std::size_t values, std::size_t repeats) {
        for (std::size_t it = 0; it < repeats; ++it) {
            auto [a, b, aCopy, bCopy] = generateMatrixPairs(rows, cols, values);

            boost::compute::device device = boost::compute::system::default_device();
            boost::compute::context context(device);
            boost::compute::command_queue queue(context, device);

            a.add(b);

            try {
                aCopy.add(bCopy, queue);
            } catch (const boost::compute::program_build_failure& e) {
                std::cerr << e.build_log() << std::endl;
                FAIL("OpenCL Program Build failed");
            }

            CHECK_EQ(a, aCopy);
        }
    }

    TEST_CASE("One Cell") {
        executeStress(1, 1, 1, 1);
    }

    TEST_CASE("Tiny Empty") {
        executeStress(5, 5, 10, 1);
    }

    TEST_CASE("Tiny Full") {
        executeStress(5, 5, 5, 5);
    }

    TEST_CASE("Small Empty") {
        executeStress(100, 70, 20, 5);
    }

    TEST_CASE("Small Full") {
        executeStress(100, 70, 6000, 5);
    }

    TEST_CASE("Large Empty") {
        executeStress(500, 200, 2000, 3);
    }

    TEST_CASE("Large Full") {
        executeStress(500, 200, 10000, 3);
    }
}
