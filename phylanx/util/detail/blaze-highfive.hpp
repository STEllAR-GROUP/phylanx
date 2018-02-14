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

namespace HighFive { namespace details {

template <typename T>
struct array_dims<blaze::DynamicVector<T, false>>
{
    static const size_t value = 1 + array_dims<T>::value;
};

template <typename T>
struct array_dims<blaze::DynamicMatrix<T, false>>
{
    static const size_t value = 2 + array_dims<T>::value;
};

template <typename T>
struct type_of_array<blaze::DynamicVector<T, false>>
{
    typedef typename type_of_array<T>::type type;
};

template <typename T>
struct type_of_array<blaze::DynamicMatrix<T, false>>
{
    typedef typename type_of_array<T>::type type;
};

template <typename T>
struct data_converter<blaze::DynamicVector<T, false>, void>
{
    typedef typename blaze::DynamicVector<T, false> Vector;

    inline data_converter(Vector& vector, DataSpace& space, size_t dim = 0)
            : _dims(space.getDimensions())
    {
        assert(_dims.size() == 1);
        (void) dim;
        (void) vector;
    }

    inline typename type_of_array<T>::type* transform_read(Vector& vector)
    {
        if (_dims[0] != vector.size())
        {
            std::cout << "resizing, fun!" << std::endl;
            vector.resize(_dims[0]);
        }
        return vector.data();
    }

    inline typename type_of_array<T>::type* transform_write(Vector& vector)
    {
        return vector.data();
    }

    inline void process_result(Vector& vector)
    {
        (void) vector;
    }

    std::vector<size_t> _dims;
};

template <typename T>
struct data_converter<blaze::DynamicMatrix<T, false>, void>
{
    typedef typename blaze::DynamicMatrix<T, false> Matrix;

    inline data_converter(Matrix& matrix, DataSpace& space, size_t dim = 0)
      : _dims(space.getDimensions())
    {
        assert(_dims.size() == 2);
        (void) dim;
        (void) matrix;
    }

    inline typename type_of_array<T>::type* transform_read(Matrix& matrix)
    {
        if (_dims[0] != matrix.rows() || _dims[1] != matrix.columns())
        {
            std::cout << "resizing, fun!" << std::endl;
            matrix.resize(_dims[0], _dims[1]);
        }
        return matrix.data();
    }

    inline typename type_of_array<T>::type* transform_write(Matrix& matrix)
    {
        return matrix.data();
    }

    inline void process_result(Matrix& matrix)
    {
        (void) matrix;
    }

    std::vector<size_t> _dims;
};

}}
#endif //PHYLANX_UTIL_DETAIL_BLAZE_HIGHFIVE_FEB_06_2017_0134PM

