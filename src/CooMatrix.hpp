//
// Created by Gleb Marin on 03.09.2021.
//

#ifndef FLOAT_MATRIX_COOMATRIX_HPP
#define FLOAT_MATRIX_COOMATRIX_HPP

#include <unordered_map>
#include <vector>
#include <ostream>
#include <istream>

#include "Utility.hpp"

namespace floatMatrix {
    template <typename T>
    class CooMatrix {
    public:
        struct Coord {
            std::size_t row, col;
        };

        struct CoordCell {
            Coord crd;
            T data;
        };

        struct Compressed {
            std::vector<std::size_t> row, col;
            std::vector<T> data;

            Compressed& sort() {
                std::vector<CoordCell> coordData(data.size());
                for (std::size_t i = 0; i < coordData.size(); ++i) {
                    coordData[i] = {{row[i], col[i]}, data[i]};
                }
                std::sort(coordData.begin(), coordData.end(), [](const CoordCell& a, const CoordCell& b) {
                    return a.data < b.data;
                });
                for (std::size_t i = 0; i < coordData.size(); ++i) {
                    row[i] = coordData[i].crd.row;
                    col[i] = coordData[i].crd.col;
                    data[i] = coordData[i].data;
                }
                return *this;
            }

            friend std::ostream& operator<<(std::ostream& os, const Compressed& comp) {
                os << comp.data.size() << '\n';
                for (std::size_t r : comp.row) {
                    os << r << ' ';
                }
                os << '\n';
                for (std::size_t c : comp.col) {
                    os << c << ' ';
                }
                os << '\n';
                for (const T& d : comp.data) {
                    os << d << ' ';
                }
                return os;
            }

            friend std::istream& operator>>(std::istream& is, typename CooMatrix<T>::Compressed& comp) {
                std::size_t elems;
                is >> elems;
                comp.data.resize(elems);
                comp.row.resize(elems);
                comp.col.resize(elems);
                for (std::size_t i = 0; i < elems; ++i) {
                    is >> comp.row[i];
                }
                for (std::size_t i = 0; i < elems; ++i) {
                    is >> comp.col[i];
                }
                for (std::size_t i = 0; i < elems; ++i) {
                    is >> comp.data[i];
                }
                return is;
            }
        };

        static constexpr T ZERO = static_cast<T>(0);

    private:
        using Row = std::unordered_map<std::size_t, T>;

    public:
        CooMatrix() = default;

        explicit CooMatrix(const std::vector<CoordCell>& elems) {
            for (std::size_t elemI = 0; elemI < elems.size(); ++elemI) {
                auto[crd, data] = elems[elemI];
                matrix_[crd.row][crd.col] = data;
            }
        }

        explicit CooMatrix(const Compressed& compressed) {
            for (std::size_t i = 0; i < compressed.data.size(); ++i) {
                matrix_[compressed.row[i]][compressed.col[i]] = compressed.data[i];
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

        CooMatrix operator+=(const CooMatrix& other) {
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

        Compressed toCompressed() const {
            Compressed compressed;
            for (const auto&[rowId, row] : matrix_) {
                for (const auto&[colId, value] : row) {
                    if (value == ZERO) {
                        continue;
                    }
                    compressed.row.push_back(rowId);
                    compressed.col.push_back(colId);
                    compressed.data.push_back(value);
                }
            }
            return compressed;
        }

        bool operator==(const CooMatrix& other) const {
            return isEqSubset(other) && other.isEqSubset(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const CooMatrix<T>& comp) {
            os << comp.toCompressed() << '\n';
            return os;
        }

        friend std::istream& operator>>(std::istream& is, CooMatrix<T>& comp) {
            Compressed compressed;
            is >> compressed;
            comp = CooMatrix(compressed);
            return is;
        }

    private:
        bool isEqSubset(const CooMatrix& other) const {
            for (const auto& [rowId, row] : matrix_) {
                for (const auto& [colId, value] : row) {
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
