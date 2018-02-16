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
struct array_dims<blaze::DynamicVector<T, TF>>
{
    static const std::size_t value = 1 + array_dims<T>::value;
};

template <typename T, bool AF, bool TF>
struct array_dims<blaze::CustomVector<T, AF, TF>>
{
    static const std::size_t value = 1 + array_dims<T>::value;
};

template <typename T, bool SO>
struct array_dims<blaze::DynamicMatrix<T, SO>>
{
    static const std::size_t value = 2 + array_dims<T>::value;
};

template <typename T, bool AF, bool PF, bool SO>
struct array_dims<blaze::CustomMatrix<T, AF, PF, SO>>
{
    static const std::size_t value = 2 + array_dims<T>::value;
};

///////////////////////////////////////////////////////////////////////////////
template <typename T, bool TF>
struct type_of_array<blaze::DynamicVector<T, TF>>
{
    typedef typename type_of_array<T>::type type;
};

template <typename T, bool AF, bool TF>
struct type_of_array<blaze::CustomVector<T, AF, TF>>
{
    typedef typename type_of_array<T>::type type;
};

template <typename T, bool SO>
struct type_of_array<blaze::DynamicMatrix<T, SO>>
{
    typedef typename type_of_array<T>::type type;
};

template <typename T, bool AF, bool PF, bool SO>
struct type_of_array<blaze::CustomMatrix<T, AF, PF, SO>>
{
    typedef typename type_of_array<T>::type type;
};

///////////////////////////////////////////////////////////////////////////////
template <typename T, bool TF>
struct data_converter<blaze::DynamicVector<T, TF>, void>
{
    typedef typename blaze::DynamicVector<T, TF> Vector;

    inline data_converter(Vector& vector, DataSpace& space, std::size_t dim = 0)
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

    std::vector<std::size_t> _dims;
};

template <typename T, bool AF, bool TF>
struct data_converter<blaze::CustomVector<T, AF, TF>, void>
{
    typedef typename blaze::CustomVector<T, AF, TF> Vector;

    inline data_converter(Vector& vector, DataSpace& space, std::size_t dim = 0)
            : _dims(space.getDimensions())
    {
        assert(_dims.size() == 1);
        (void) dim;
        (void) vector;
    }

    inline typename type_of_array<T>::type* transform_read(Vector& vector)
    {
        throw std::runtime_error("can't read into blaze::CustomVector");
    }

    inline typename type_of_array<T>::type* transform_write(Vector& vector)
    {
        return vector.data();
    }

    inline void process_result(Vector& vector)
    {
        (void) vector;
    }

    std::vector<std::size_t> _dims;
};

template <typename T, bool SO>
struct data_converter<blaze::DynamicMatrix<T, SO>, void>
{
    typedef typename blaze::DynamicMatrix<T, SO> Matrix;

    inline data_converter(Matrix& matrix, DataSpace& space, std::size_t dim = 0)
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

    std::vector<std::size_t> _dims;
};

template <typename T, bool AF, bool PF, bool SO>
struct data_converter<blaze::CustomMatrix<T, AF, PF, SO>, void>
{
    typedef typename blaze::CustomMatrix<T, AF, PF, SO> Matrix;

    inline data_converter(Matrix& matrix, DataSpace& space, std::size_t dim = 0)
      : _dims(space.getDimensions())
    {
        assert(_dims.size() == 2);
        (void) dim;
        (void) matrix;
    }

    inline typename type_of_array<T>::type* transform_read(Matrix& matrix)
    {
        throw std::runtime_error("can't read into blaze::CustomMatrix");
    }

    inline typename type_of_array<T>::type* transform_write(Matrix& matrix)
    {
        return matrix.data();
    }

    inline void process_result(Matrix& matrix)
    {
        (void) matrix;
    }

    std::vector<std::size_t> _dims;
};

}}

#endif //PHYLANX_UTIL_DETAIL_BLAZE_HIGHFIVE_FEB_06_2017_0134PM

