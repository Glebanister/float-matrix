//
// Created by Gleb Marin on 04.09.2021.
//

#include <sstream>

#include <doctest.h>

#include "CooMatrix.hpp"

namespace floatMatrix {

    using FloatMatrix = floatMatrix::CooMatrix;
    using Cell = FloatMatrix::Cell;
    using DeviceCells = FloatMatrix::DeviceCells;

    TEST_SUITE("CooMatrixOverall") {

        bool eqFloat(float a, float b) {
            return std::abs(a - b) <= 1e-1;
        }

        TEST_CASE("Serialize & Deserialize") {
            FloatMatrix mtx(DeviceCells{
                Cell{0, 0, 1.0},
                Cell{1, 4, 6.0},
                Cell{2, 3, 0.0},
                Cell{8, 0, 9.0},
                Cell{3, 1, 1.1},
                Cell{4, 2, 13.4},
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
            FloatMatrix A(DeviceCells{
                Cell{0, 0, 1.1},
                Cell{0, 1, 2.2},
                Cell{0, 2, 3.3},

                Cell{1, 0, 4.4},
                Cell{1, 1, 5.5},
                Cell{1, 2, 6.6},

                Cell{2, 0, 7.7},
                Cell{2, 1, 8.8},
                Cell{2, 2, 9.9},
            });

            FloatMatrix B(DeviceCells{
                Cell{0, 0, 10.1},
                Cell{0, 1, 20.2},
                Cell{0, 2, 30.3},

                Cell{1, 0, 40.4},
                Cell{1, 1, 50.5},
                Cell{1, 2, 60.6},

                Cell{2, 0, 70.7},
                Cell{2, 1, 80.8},
                Cell{2, 2, 90.9},
            });

            FloatMatrix AplusB(DeviceCells{
                Cell{0, 0, 11.2},
                Cell{0, 1, 22.4},
                Cell{0, 2, 33.6},

                Cell{1, 0, 44.8},
                Cell{1, 1, 56.0},
                Cell{1, 2, 67.2},

                Cell{2, 0, 78.4},
                Cell{2, 1, 89.6},
                Cell{2, 2, 100.8},
            });

            CHECK_EQ(A.add(B), AplusB);
        }

        TEST_CASE("Add OpenCL Compilation") {
            FloatMatrix A(DeviceCells{
                Cell{0, 0, 1.1},
                Cell{0, 2, 3.3},

                Cell{1, 0, 4.4},
                Cell{1, 1, 5.5},
            });

            boost::compute::device device = boost::compute::system::default_device();
            boost::compute::context context(device);
            boost::compute::command_queue queue(context, device);

            A.add(A, queue);

            try {
                A.add(A, queue);
            } catch (const boost::compute::program_build_failure& e) {
                std::cerr << e.build_log() << std::endl;
                FAIL(e.build_log());
            }
        }
    }


} // namespace floatMatrix
