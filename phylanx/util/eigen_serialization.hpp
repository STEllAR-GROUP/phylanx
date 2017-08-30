//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_EIGEN_SERIALIZATION_HPP)
#define PHYLANX_UTIL_EIGEN_SERIALIZATION_HPP

#include <phylanx/config.hpp>

#include <hpx/include/serialization.hpp>

#include <Eigen/Dense>

namespace hpx { namespace serialization
{
    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    void load(input_archive& ar,
        Eigen::Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>& m,
        unsigned)
    {
        std::ptrdiff_t rows = 0;
        std::ptrdiff_t cols = 0;
        ar >> rows >> cols;
        m = Eigen::Matrix<T, Rows, Cols>(rows, cols);
        ar >> make_array(m.data(), rows * cols);
    }

    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    void save(output_archive& ar,
        Eigen::Matrix<T, Rows, Cols, Options, MaxRows, MaxCols> const& m,
        unsigned)
    {
        std::ptrdiff_t rows = m.rows();
        std::ptrdiff_t cols = m.cols();
        ar << rows << cols << make_array(m.data(), rows * cols);
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, int Rows, int Cols, int Options,
            int MaxRows, int MaxCols>),
        (Eigen::Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>));
}}

#endif
