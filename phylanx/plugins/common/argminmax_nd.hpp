// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_ARGMINMAX_2020_MAY_21_0326PM)
#define PHYLANX_COMMON_ARGMINMAX_2020_MAY_21_0326PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <string>

namespace phylanx { namespace common {

    template <typename Operation>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type argminmax0d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

    template <typename Operation>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type argminmax1d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename,
        execution_tree::primitive_argument_type* value = nullptr);

    template <typename Operation>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type argminmax2d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

    template <typename Operation>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type argminmax3d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename);

}}    // namespace phylanx::common

#endif
