#include <algorithm>
#include <random>

#include <boost/compute/container/vector.hpp>
#include <boost/compute/device.hpp>

#include "CooMatrix.hpp"

namespace compute = boost::compute;

using FloatMatrix = floatMatrix::CooMatrix<float>;
static std::mt19937 rnd{std::random_device{}()};

template <typename T>
std::size_t getRandInRange(const T& begin, const T& end) {
    return rnd() % (end - begin + 1) + begin;
}

float getRandFloat() {
    return std::uniform_real_distribution(0.f, 1000000000.f)(rnd);
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

int main() {


    auto [a, b, aCopy, bCopy] = generateMatrixPairs(100, 100, 150);

    boost::compute::device device = boost::compute::system::default_device();
    boost::compute::context context(device);
    boost::compute::command_queue queue(context, device);

    a.add(b);

    try {
        aCopy.add(bCopy, queue);
    } catch (const boost::compute::program_build_failure& e) {
        std::cerr << "Error: " << e.build_log() << std::endl;
    }

    return 0;
}
