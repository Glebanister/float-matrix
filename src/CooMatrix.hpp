//
// Created by Gleb Marin on 03.09.2021.
//

#ifndef FLOAT_MATRIX_COOMATRIX_HPP
#define FLOAT_MATRIX_COOMATRIX_HPP

#include <unordered_map>
#include <vector>
#include <ostream>
#include <istream>

#include <boost/compute/algorithm/sort.hpp>
#include <boost/compute/algorithm/merge.hpp>
#include <boost/compute/algorithm/reduce_by_key.hpp>

#include "Utility.hpp"

#define BOOST_COMPUTE_FUNCTION_CUSTOM(return_type, name, arguments, actual_arguments, ...) \
    ::boost::compute::function<return_type arguments> name = \
    ::boost::compute::detail::make_function_impl<return_type arguments>( \
            #name, #actual_arguments, #__VA_ARGS__ \
        )

namespace floatMatrix {
    namespace cell {
        using Cell = boost::compute::float4_;

        template <typename T>
        static Cell make(std::size_t row, std::size_t col, T data) {
            return {
                    static_cast<float>(row),
                    static_cast<float>(col),
                    static_cast<float>(data),
                    0.f
            };
        }

        static std::size_t row(const Cell& cell) {
            return static_cast<std::size_t>(cell[0]);
        }

        static std::size_t col(const Cell& cell) {
            return static_cast<std::size_t>(cell[1]);
        }

        template <typename T>
        static std::size_t data(const Cell& cell) {
            return static_cast<T>(cell[2]);
        }

        BOOST_COMPUTE_FUNCTION(
            bool, compareCellCoords,
            (boost::compute::float4_ a, boost::compute::float4_ b), {
                if (a[0] == b[0]) {
                    return a[1] < b[1];
                }
                return a[0] < b[1];
            }
        );

        BOOST_COMPUTE_FUNCTION(
            boost::compute::float4_, reduceSameCells,
            (boost::compute::float4_ a, boost::compute::float4_ b), {
                return (float4)(a[0], a[1], a[2] + b[2], 0.f);
            }
        );

        BOOST_COMPUTE_FUNCTION(
            bool, equalCellCoords,
            (boost::compute::float4_ a, boost::compute::float4_ b), {
                return a[0] == b[0] && a[1] == b[1];
            }
        );
    }

    template <typename T>
    class CooMatrix {
    public:
        template <typename E>
        using BcVector = boost::compute::vector<E>;
        using CommandQueue = boost::compute::command_queue;
        using Cell = cell::Cell;

        class DeviceCells : boost::compute::vector<Cell> {
        public:
            explicit DeviceCells(std::size_t size, CommandQueue& queue)
                    : BcVector<cell::Cell>(size, queue.get_context()) {}

            using BcVector<Cell>::begin;
            using BcVector<Cell>::end;
            using BcVector<Cell>::size;
            using BcVector<Cell>::resize;
            using BcVector<Cell>::operator[];

            void sortOnDevice(CommandQueue& queue = boost::compute::system::default_queue()) {
                boost::compute::sort(begin(), end(), cell::compareCellCoords, queue);
            }
        };

        static constexpr T ZERO = static_cast<T>(0);

    private:
        using Row = std::unordered_map<std::size_t, T>;

    public:
        CooMatrix() = default;

        explicit CooMatrix(const DeviceCells& cells) {
            for (const auto& cell : cells) {
                matrix_[cell::row(cell)][cell::col(cell)] = cell::data<T>(cell);
            }
        }

        explicit CooMatrix(const std::vector<Cell>& cells) {
            for (const auto& cell : cells) {
                matrix_[cell::row(cell)][cell::col(cell)] = cell::data<T>(cell);
            }
        }

        void set(std::size_t row, std::size_t col, const T& value) noexcept {
            matrix_[row][col] = value;
        }

        T get(std::size_t rowId, std::size_t colId) const noexcept {
            auto rowIt = matrix_.find(rowId);
            if (rowIt == matrix_.cend()) {
                return ZERO;
            }
            const Row& row = rowIt->second;
            auto valIt = row.find(colId);
            if (valIt == row.cend()) {
                return ZERO;
            }
            return valIt->second;
        }

        void clear() noexcept {
            matrix_.clear();
        }

        CooMatrix& add(const CooMatrix& other) {
            for (const auto&[rowId, row] : other.matrix_) {
                for (const auto&[colId, value] : row) {
                    matrix_[rowId][colId] += value;
                    if (matrix_[rowId][colId] == ZERO) {
                        matrix_[rowId].erase(colId);
                    }
                }
            }
            return *this;
        }

        CooMatrix& add(const CooMatrix& other, boost::compute::command_queue& queue) {
            DeviceCells thisCells = toDeviceCells(queue);
            DeviceCells otherCells = other.toDeviceCells(queue);
            thisCells.sortOnDevice(queue);
            otherCells.sortOnDevice(queue);
            DeviceCells merged(thisCells.size() + otherCells.size(), queue);

            boost::compute::merge(
                    thisCells.begin(), thisCells.end(),
                    otherCells.begin(), otherCells.end(),
                    merged.begin(),
                    cell::compareCellCoords,
                    queue
            );

            DeviceCells reducedValues(merged.size(), queue);
            boost::compute::reduce_by_key(
                    merged.begin(), merged.end(), merged.begin(), // inputs
                    boost::compute::discard_iterator(), reducedValues.begin(), // outputs
                    cell::reduceSameCells, // reduce function
                    cell::equalCellCoords, // compare function
                    queue
            );
            return *this = CooMatrix(reducedValues);
        }

        [[nodiscard]] std::vector<Cell> toCellsList() const {
            std::vector<Cell> cells;
            for (const auto&[rowId, row] : matrix_) {
                for (const auto&[colId, value] : row) {
                    if (value == ZERO) {
                        continue;
                    }
                    cells.push_back(cell::make<T>(rowId, colId, value));
                }
            }
            return cells;
        }

        DeviceCells toDeviceCells(CommandQueue& queue) const {
            std::vector<Cell> cells = toCellsList();
            DeviceCells deviceCells(cells.size(), queue);
            boost::compute::copy(cells.begin(), cells.end(), deviceCells.begin(), queue);
            return deviceCells;
        }

        bool operator==(const CooMatrix& other) const {
            return isEqSubset(other) && other.isEqSubset(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const CooMatrix<T>& comp) {
            std::vector<Cell> cells = comp.toCellsList();
            os << cells.size() << '\n';
            for (const Cell& cell : cells) {
                os << cell::row(cell) << ' ';
                os << cell::row(cell) << ' ';
                os << cell::data<T>(cell) << ' ';
            }
            return os;
        }

        friend std::istream& operator>>(std::istream& is, CooMatrix<T>& comp) {
            std::size_t elems;
            is >> elems;
            std::unordered_map<std::size_t, Row> rows(elems);
            for (std::size_t i = 0; i < elems; ++i) {
                std::size_t row, col;
                T data;
                is >> row >> col >> data;
                rows[row][col] = data;
            }
            comp.matrix_ = rows;
            return is;
        }

    private:
        bool isEqSubset(const CooMatrix& other) const {
            for (const auto&[rowId, row] : matrix_) {
                for (const auto&[colId, value] : row) {
                    if (!utility::isEq(other.get(rowId, colId), value)) {
                        return false;
                    }
                }
            }
            return true;
        }

        std::unordered_map<std::size_t, Row> matrix_;
    };
}

#endif //FLOAT_MATRIX_COOMATRIX_HPP
