// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_STATISTICS_DEC_24_2018_0227PM)
#define PHYLANX_COMMON_STATISTICS_DEC_24_2018_0227PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <hpx/datastructures/optional.hpp>

#include <cstdint>
#include <string>

namespace phylanx { namespace common {

    template <template <class T> class Op>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type statisticsnd(
        execution_tree::primitive_argument_type&& arg, ir::range&& axes,
        bool keepdims, execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);

    template <template <class T> class Op>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type statisticsnd(
        execution_tree::primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);

}}    // namespace phylanx::common

#endif
