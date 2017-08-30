//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_EIGEN_RANGE_HPP)
#define PHYLANX_UTIL_EIGEN_RANGE_HPP

#include <phylanx/config.hpp>

#include <Eigen/Dense>

namespace Eigen
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    T* begin(Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>& m)
    {
        return m.data();
    }

    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    T const* begin(Matrix<T, Rows, Cols, Options, MaxRows, MaxCols> const& m)
    {
        return m.data();
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    T* end(Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>& m)
    {
        return m.data() + m.size();
    }

    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    T const* end(Matrix<T, Rows, Cols, Options, MaxRows, MaxCols> const& m)
    {
        return m.data() + m.size();
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    bool empty(Matrix<T, Rows, Cols, Options, MaxRows, MaxCols> const& m)
    {
        return m.size() == 0;
    }
}

#endif

