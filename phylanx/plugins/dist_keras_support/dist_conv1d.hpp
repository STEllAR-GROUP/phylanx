// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_MATRIXOPS_CONV1D_OPERATION)
#define PHYLANX_DIST_MATRIXOPS_CONV1D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/plugins/common/conv1d_all_paddings.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_keras_support { namespace primitives {

    class dist_conv1d
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_conv1d>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_conv1d() = default;

        dist_conv1d(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        execution_tree::primitive_argument_type conv1d_all_paddings(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            execution_tree::localities_information&& arg_locs,
            execution_tree::localities_information&& kernel_locs,
            std::string&& padding, std::string&& given_name) const;
        execution_tree::primitive_argument_type conv1d_all_paddings(
            execution_tree::primitive_argument_type&& arg,
            execution_tree::primitive_argument_type&& kernel,
            std::string&& padding, std::string&& given_name) const;
    };

    inline execution_tree::primitive create_dist_conv1d(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "conv1d_d", std::move(operands), name, codename);
    }
}}}

#endif
