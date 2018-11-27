// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return argument as a vector, scalars are properly broadcast
    template <typename T>
    void extract_value_vector(
        typename ir::node_data<T>::storage1d_type& result,
        ir::node_data<T>&& rhs, std::size_t size, std::string const& name,
        std::string const& codename)
    {
        using storage1d_type = typename ir::node_data<T>::storage1d_type;

        switch (rhs.num_dimensions())
        {
        case 0:
            {
                result.resize(size);
                result = rhs.scalar();
                return;
            }
            return;

        case 1:
            {
                // vectors of size one can be broadcast into any other vector
                if (rhs.size() == 1)
                {
                    result.resize(size);
                    result = rhs[0];
                    return;
                }

                if (size != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_vector",
                        util::generate_error_message(
                            "cannot broadcast a vector into a vector of "
                                "different size",
                            name, codename));
                }

                if (rhs.is_ref())
                {
                    result = rhs.vector();
                }
                else
                {
                    result = std::move(rhs.vector_non_ref());
                }
                return;
            }

        case 2:
            {

                // matrices of size one can be broadcast into any vector
                if (rhs.size() == 1)
                {
                    result.resize(size);
                    result = rhs[0];
                    return;
                }

                // matrices with one column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && size == rhs.dimension(1))
                {
                    result.resize(size);

                    auto m = rhs.matrix();
                    result = blaze::trans(blaze::row(m, 0));
                    return;
                }

                // matrices with one row can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(1) == 1 && size == rhs.dimension(0))
                {
                    result.resize(size);

                    auto m = rhs.matrix();
                    result = blaze::column(m, 0);
                    return;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::extract_value_vector",
                    util::generate_error_message(
                        "cannot broadcast a matrix of arbitrary size into "
                        "a vector",
                        name, codename));
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            {
                // tensors of size one can be broadcast into any vector
                if (rhs.size() == 1)
                {
                    result.resize(size);

                    result = rhs.at(0, 0, 0);
                    return;
                }

                // tensors with one column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == size &&
                    rhs.dimension(2) == 1)
                {
                    auto t = rhs.tensor();
                    result = blaze::column(blaze::pageslice(t, 0), 0);
                    return;
                }

                // tensors with one row can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == size)
                {
                    auto t = rhs.tensor();
                    result = blaze::trans(blaze::row(blaze::pageslice(t, 0), 0));
                    return;
                }

                // tensors with one page-column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == size && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1)
                {
                    auto t = rhs.tensor();
                    result = blaze::column(blaze::rowslice(t, 0), 0);
                    return;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::extract_value_vector",
                    util::generate_error_message(
                        "cannot broadcast a tensor of arbitrary size into "
                        "a vector",
                        name, codename));
            }
#endif

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_vector",
            util::generate_error_message(
                "primitive_argument_type does not hold a scalar or vector "
                    "numeric value type",
                name, codename));
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(ir::node_data<T>&& arg,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage1d_type result;
        extract_value_vector(result, std::move(arg), size, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        return extract_value_vector(
            extract_node_data<T>(val, name, codename),
            size, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        return extract_value_vector(
            extract_node_data<T>(std::move(val), name, codename),
            size, name, codename);
    }

    // explicitly instantiate necessary functions
    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_vector<double>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_vector<std::int64_t>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_vector<std::uint8_t>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_vector<double>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_vector<std::int64_t>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_vector<std::uint8_t>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);
}}
