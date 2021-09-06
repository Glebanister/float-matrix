//
// Created by Gleb Marin on 03.09.2021.
//

#ifndef FLOAT_MATRIX_COOMATRIX_HPP
#define FLOAT_MATRIX_COOMATRIX_HPP

#include <unordered_map>
#include <vector>
#include <ostream>
#include <istream>
#include <algorithm>

#include <boost/compute/algorithm/sort.hpp>
#include <boost/compute/algorithm/merge.hpp>
#include <boost/compute/algorithm/reduce_by_key.hpp>
#include <boost/compute/types/struct.hpp>

#include "Utility.hpp"

namespace floatMatrix {
    using T = float;

    namespace cell {
        struct Cell {
            int row;
            int col;
            float data;
        };

        static inline BOOST_COMPUTE_FUNCTION(
                bool, compareCellCoords,
                (Cell a, Cell b), {
                    if (a.row == b.row) {
                        return a.col < b.col;
                    }
                    return a.row < b.row;
                }
        );

        static inline BOOST_COMPUTE_FUNCTION(
                Cell, reduceSameCells,
                (Cell a, Cell b), {
                    Cell reduced = {.row = a.row, .col = a.col, .data = a.data + b.data};
                    return reduced;
                }
        );

        static inline BOOST_COMPUTE_FUNCTION(
                bool, equalCellCoords,
                (Cell a, Cell b), {
                    return a.col == b.col && a.row == b.row;
                }
        );
    }

    class CooMatrix {
    public:
        using CommandQueue = boost::compute::command_queue;
        using Cell = cell::Cell;
        using DeviceCells = boost::compute::vector<Cell>;

        static constexpr T ZERO = static_cast<T>(0);

    private:
        using Row = std::unordered_map<std::size_t, T>;

    public:
        CooMatrix() = default;

        explicit CooMatrix(const DeviceCells& cells) {
            for (Cell cell : cells) {
                matrix_[cell.row][cell.col] = cell.data;
            }
        }

        template <typename Iterator>
        explicit CooMatrix(Iterator begin, Iterator end, CommandQueue queue) {
            for (auto it = begin; it < end; ++it) {
                const auto& cell = it.read(queue);
                matrix_[cell.row][cell.col] = cell.data;
            }
        }

        explicit CooMatrix(const std::vector<Cell>& cells) {
            for (const auto& cell : cells) {
                matrix_[cell.row][cell.col] = cell.data;
            }
        }

        void set(std::size_t row, std::size_t col, const T& value) noexcept {
            matrix_[row][col] = value;
        }

        [[nodiscard]] T get(std::size_t rowId, std::size_t colId) const noexcept {
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
            boost::compute::sort(thisCells.begin(), thisCells.end(), cell::compareCellCoords, queue);
            boost::compute::sort(otherCells.begin(), otherCells.end(), cell::compareCellCoords, queue);
            DeviceCells merged(thisCells.size() + otherCells.size(), queue.get_context());

            boost::compute::merge(
                    thisCells.begin(), thisCells.end(),
                    otherCells.begin(), otherCells.end(),
                    merged.begin(),
                    cell::compareCellCoords,
                    queue
            );

            DeviceCells reducedValues(merged.size(), queue.get_context());
            DeviceCells reducedKeys(merged.size(), queue.get_context());
            auto [keysEnd, valuesEnd] = boost::compute::reduce_by_key(
                    merged.begin(), merged.end(), merged.begin(),
                    reducedKeys.begin(), reducedValues.begin(),
                    cell::reduceSameCells,
                    cell::equalCellCoords,
                    queue
            );

            return *this = CooMatrix(reducedValues.begin(), valuesEnd, queue);
        }

        [[nodiscard]] std::vector<Cell> toCellsList() const {
            std::vector<Cell> cells;
            for (const auto&[rowId, row] : matrix_) {
                for (const auto&[colId, value] : row) {
                    if (value == ZERO) {
                        continue;
                    }
                    cells.push_back({static_cast<int>(rowId), static_cast<int>(colId), value});
                }
            }
            return cells;
        }

        DeviceCells toDeviceCells(CommandQueue& queue) const {
            std::vector<Cell> cells = toCellsList();
            DeviceCells deviceCells(cells.size(), queue.get_context());
            boost::compute::copy(cells.begin(), cells.end(), deviceCells.begin(), queue);
            return deviceCells;
        }

        bool operator==(const CooMatrix& other) const {
            return isEqSubset(other) && other.isEqSubset(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const CooMatrix& comp) {
            std::vector<Cell> cells = comp.toCellsList();
            os << cells.size() << '\n';
            for (const Cell& cell : cells) {
                os << cell.row << ' ';
                os << cell.col << ' ';
                os << cell.data << ' ';
            }
            return os;
        }

        friend std::istream& operator>>(std::istream& is, CooMatrix& comp) {
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
        [[nodiscard]] bool isEqSubset(const CooMatrix& other) const {
            for (const auto&[rowId, row] : matrix_) {
                for (const auto&[colId, value] : row) {
                    if (!utility::isEq(other.get(rowId, colId), value)) {
                        std::cerr << "Row: " << rowId << ' ' << "Col: " << colId << std::endl;
                        return false;
                    }
                }
            }
            return true;
        }

        std::unordered_map<std::size_t, Row> matrix_;
    };
}

BOOST_COMPUTE_ADAPT_STRUCT(floatMatrix::cell::Cell, Cell, (row, col, data));

#endif //FLOAT_MATRIX_COOMATRIX_HPP
