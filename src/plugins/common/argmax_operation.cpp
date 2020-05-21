//  Copyright (c) 2017-2020 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/plugins/common/argminmax_nd.hpp>
#include <phylanx/plugins/common/argminmax_nd_impl.hpp>
#include <phylanx/plugins/common/argminmax_operations.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <string>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    struct argmax_op
    {
        template <typename T>
        static T initial()
        {
            return util::detail::numeric_limits_min<T>();
        }

        template <typename T>
        static bool compare(T lhs, T rhs)
        {
            return lhs > rhs;
        }

        template <typename Iter>
        Iter operator()(Iter begin, Iter end) const
        {
            return std::max_element(begin, end);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    argminmax0d<argmax_op>(execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    argminmax1d<argmax_op>(execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    argminmax2d<argmax_op>(execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    argminmax3d<argmax_op>(execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);
}}    // namespace phylanx::common
