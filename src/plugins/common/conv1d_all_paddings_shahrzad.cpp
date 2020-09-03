// Copyright (c) 2019-2020 Bita Hasheminezhad
// Copyright (c) 2019-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/conv1d_all_paddings_shahrzad.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/parallel_for_loop.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_valid_shahrzad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        std::size_t batch = a.pages();
        std::size_t filter_length = k.columns();
        std::size_t in_channels = a.rows();
        std::size_t out_channels = k.pages();
        std::size_t result_length = a.columns() - filter_length + 1;

        blaze::DynamicTensor<double> result(batch, out_channels, result_length);
        hpx::parallel::for_loop(hpx::parallel::execution::par, std::size_t(0),
            batch, [&](std::size_t p) {
                for (std::size_t l = 0; l < k.pages(); ++l)
                {
                    for (std::size_t m = 0; m < result_length; ++m)
                    {
                        auto x = blaze::subtensor(
                            a, p, 0, m, 1, a.rows(), k.columns());
                        auto y = blaze::subtensor(
                            k, l, 0, 0, 1, k.rows(), k.columns());
                        auto w = blaze::sum(x % y);
                        blaze::subtensor(result, p, l, m, 1, 1, 1) = w;
                    }
                }
            });
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    /////////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_all_paddings_shahrzad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::string const& name,
        std::string const& codename)
    {
        if (execution_tree::extract_numeric_value_dimensions(
                arg, name, codename)[1] !=
            execution_tree::extract_numeric_value_dimensions(
                kernel, name, codename)[1])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv1d_all_paddings::conv1d_all_paddings",
                util::generate_error_message(
                    "Number of input channels is not the same", name,
                    codename));
        }

        return conv1d_valid_shahrzad(std::move(arg), std::move(kernel));
    }
}}    // namespace phylanx::common
