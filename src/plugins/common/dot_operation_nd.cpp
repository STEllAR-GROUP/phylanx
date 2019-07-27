//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/dot_operation_nd_impl.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <cstdint>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type dot0d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot0d(
                extract_boolean_value(std::move(lhs), name, codename),
                extract_boolean_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_int64:
            return dot0d(
                extract_integer_value(std::move(lhs), name, codename),
                extract_integer_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot0d(
                extract_numeric_value(std::move(lhs), name, codename),
                extract_numeric_value(std::move(rhs), name, codename),
                name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "common::dot0d",
            util::generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types", name, codename));
    }

    execution_tree::primitive_argument_type dot1d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot1d(
                extract_boolean_value(std::move(lhs), name, codename),
                extract_boolean_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_int64:
            return dot1d(
                extract_integer_value(std::move(lhs), name, codename),
                extract_integer_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot1d(
                extract_numeric_value(std::move(lhs), name, codename),
                extract_numeric_value(std::move(rhs), name, codename),
                name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "common::dot1d",
            util::generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types", name, codename));
    }

    execution_tree::primitive_argument_type dot2d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot2d(
                extract_boolean_value(std::move(lhs), name, codename),
                extract_boolean_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_int64:
            return dot2d(
                extract_integer_value(std::move(lhs), name, codename),
                extract_integer_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot2d(
                extract_numeric_value(std::move(lhs), name, codename),
                extract_numeric_value(std::move(rhs), name, codename),
                name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "common::dot2d",
            util::generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types", name, codename));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    execution_tree::primitive_argument_type dot3d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot3d(
                extract_boolean_value(std::move(lhs), name, codename),
                extract_boolean_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_int64:
            return dot3d(
                extract_integer_value(std::move(lhs), name, codename),
                extract_integer_value(std::move(rhs), name, codename),
                name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot3d(
                extract_numeric_value(std::move(lhs), name, codename),
                extract_numeric_value(std::move(rhs), name, codename),
                name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "common::dot3d",
            util::generate_error_message(
                "the dot primitive requires for all arguments to "
                "be numeric data types", name, codename));
    }
#endif
}}
