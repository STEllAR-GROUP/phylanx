//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_UTIL_DETAIL_BLAZE_HIGHFIVE_FEB_06_2017_0134PM
#define PHYLANX_UTIL_DETAIL_BLAZE_HIGHFIVE_FEB_06_2017_0134PM

#include <blaze/Blaze.h>

#include <highfive/bits/H5Utils.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace HighFive { namespace details {

///////////////////////////////////////////////////////////////////////////////
template <typename T, bool TF>
struct inspector<blaze::DynamicVector<T, TF>>
{
    static const std::size_t recursive_ndim = 1 + inspector<T>::recursive_ndim;
    typedef typename inspector<T>::base_type base_type;
};

template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF, bool TF,
    typename RT>
struct inspector<blaze::CustomVector<T, AF, PF, TF, RT>>
{
    static const std::size_t recursive_ndim = 1 + inspector<T>::recursive_ndim;
    typedef typename inspector<T>::base_type base_type;
};

template <typename T, bool SO>
struct inspector<blaze::DynamicMatrix<T, SO>>
{
    static const std::size_t recursive_ndim = 2 + inspector<T>::recursive_ndim;
    typedef typename inspector<T>::base_type base_type;
};

template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF, bool SO,
    typename RT>
struct inspector<blaze::CustomMatrix<T, AF, PF, SO, RT>>
{
    static const std::size_t recursive_ndim = 2 + inspector<T>::recursive_ndim;
    typedef typename inspector<T>::base_type base_type;
};

///////////////////////////////////////////////////////////////////////////////
template <typename T, bool TF>
struct data_converter<blaze::DynamicVector<T, TF>, void>
{
    typedef typename blaze::DynamicVector<T, TF> Vector;

    inline data_converter(DataSpace const& space)
            : _dims(space.getDimensions())
    {
        assert(_dims.size() == 1);
    }

    inline typename inspector<T>::type* transform_read(Vector& vector) const
    {
        if (_dims[0] != vector.size())
        {
            vector.resize(_dims[0]);
        }
        return vector.data();
    }

    inline typename inspector<T>::type const* transform_write(
        Vector const& vector) const noexcept
    {
        return vector.data();
    }

    inline void process_result(Vector& vector) const
    {
        (void) vector;
    }

    std::vector<std::size_t> _dims;
};

template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF, bool TF,
    typename RT>
struct data_converter<blaze::CustomVector<T, AF, PF, TF, RT>, void>
{
    typedef typename blaze::CustomVector<T, AF, PF, TF, RT> Vector;

    inline data_converter(DataSpace const& space)
            : _dims(space.getDimensions())
    {
        assert(_dims.size() == 1);
    }

    inline typename inspector<T>::type* transform_read(Vector& vector) const
    {
        throw std::runtime_error("can't read into blaze::CustomVector");
    }

    inline typename inspector<T>::type const* transform_write(
        Vector const& vector) const noexcept
    {
        return vector.data();
    }

    inline void process_result(Vector& vector) const
    {
        (void) vector;
    }

    std::vector<std::size_t> _dims;
};

template <typename T, bool SO>
struct data_converter<blaze::DynamicMatrix<T, SO>, void>
{
    typedef typename blaze::DynamicMatrix<T, SO> Matrix;

    inline data_converter(DataSpace const& space)
      : _dims(space.getDimensions())
    {
        assert(_dims.size() == 2);
    }

    inline typename inspector<T>::type* transform_read(Matrix& matrix) const
    {
        if (_dims[0] != matrix.rows() || _dims[1] != matrix.columns())
        {
            matrix.resize(_dims[0], _dims[1]);
        }
        return matrix.data();
    }

    inline typename inspector<T>::type const* transform_write(
        Matrix const& matrix) const noexcept
    {
        return matrix.data();
    }

    inline void process_result(Matrix& matrix) const
    {
        (void) matrix;
    }

    std::vector<std::size_t> _dims;
};

template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF, bool SO,
    typename RT>
struct data_converter<blaze::CustomMatrix<T, AF, PF, SO, RT>, void>
{
    typedef typename blaze::CustomMatrix<T, AF, PF, SO, RT> Matrix;

    inline data_converter(DataSpace const& space)
      : _dims(space.getDimensions())
    {
        assert(_dims.size() == 2);
    }

    inline typename inspector<T>::type* transform_read(
        Matrix& matrix) const
    {
        throw std::runtime_error("can't read into blaze::CustomMatrix");
    }

    inline typename inspector<T>::type const* transform_write(
        Matrix const& matrix) const noexcept
    {
        return matrix.data();
    }

    inline void process_result(Matrix& matrix) const
    {
        (void) matrix;
    }

    std::vector<std::size_t> _dims;
};

}}

#endif //PHYLANX_UTIL_DETAIL_BLAZE_HIGHFIVE_FEB_06_2017_0134PM

