//   Copyright (c) 2020 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/indices.hpp>
#include <phylanx/plugins/controls/indices.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const indices::match_data = {
        match_pattern_type{"indices", std::vector<std::string>{R"(
                indices(_1, __arg(_2_dtype, "int"), __arg(_3_sparse, false))
            )"},
            &create_indices, &create_primitive<indices>, R"(
            dimensions, dtype, sparse
            Args:

                dimensions (list of ints) : the shape of the grid
                dtype (optional, string) : the data-type of the returned array,
                  defaults to 'int'.
                sparse (optional, bool) : return a sparse representation of the
                  grid instead of a dense representation, defaults to 'false'

            Returns:

            One array or tuple of arrays.)"}};

    ///////////////////////////////////////////////////////////////////////////
    indices::indices(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type indices::generate_indices(
        ir::range&& shape, node_data_type dtype, eval_context ctx) const
    {
        switch (shape.size())
        {
        case 0:
            return common::indices0d(
                std::move(shape), name_, codename_, std::move(ctx));

        case 1:
            return common::indices1d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 2:
            return common::indices2d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 3:
            return common::indices3d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 4:
            return common::indices4d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "indices::generate_indices",
            generate_error_message(
                "unsupported dimensionality of shape argument",
                std::move(ctx)));
    }

    primitive_argument_type indices::generate_sparse_indices(
        ir::range&& shape, node_data_type dtype, eval_context ctx) const
    {
        switch (shape.size())
        {
        case 0:
            return common::sparse_indices0d(
                std::move(shape), name_, codename_, std::move(ctx));

        case 1:
            return common::sparse_indices1d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 2:
            return common::sparse_indices2d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 3:
            return common::sparse_indices3d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));

        case 4:
            return common::sparse_indices4d(
                std::move(shape), dtype, name_, codename_, std::move(ctx));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "indices::generate_sparse_indices",
            generate_error_message(
                "unsupported dimensionality of shape argument",
                std::move(ctx)));
    }

    hpx::future<primitive_argument_type> indices::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2 &&
            operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "indices::eval",
                generate_error_message(
                    "the indices primitive requires at most two operands",
                    std::move(ctx)));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "indices::eval",
                generate_error_message(
                    "the indices primitive requires that the "
                    "first argument given is valid",
                    std::move(ctx)));
        }

        auto arg0 =
            list_operand_strict(operands[0], args, name_, codename_, ctx);
        auto this_ = this->shared_from_this();

        if (operands.size() == 1)
        {
            return hpx::dataflow(
                hpx::launch::sync,
                [this_ = std::move(this_), ctx = std::move(ctx)](
                    hpx::future<ir::range>&& farg) mutable {
                    return this_->generate_indices(
                        farg.get(), node_data_type_int64, std::move(ctx));
                },
                std::move(arg0));
        }

        auto arg1 =
            string_operand_strict(operands[1], args, name_, codename_, ctx);
        if (operands.size() == 2)
        {
            if (!valid(operands[1]) && !is_explicit_nil(operands[1]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "indices::eval",
                    generate_error_message(
                        "the indices primitive requires that the "
                        "second argument is either valid or 'nil'"));
            }

            return hpx::dataflow(
                hpx::launch::sync,
                [this_ = std::move(this_), ctx = std::move(ctx)](
                    hpx::future<ir::range>&& farg,
                    hpx::future<std::string>&& fdtype) mutable {
                    return this_->generate_indices(
                        farg.get(), map_dtype(fdtype.get()), std::move(ctx));
                },
                std::move(arg0), std::move(arg1));
        }

        if (!valid(operands[2]) && !is_explicit_nil(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "indices::eval",
                generate_error_message(
                    "the indices primitive requires that the "
                    "third argument is either valid or 'nil'"));
        }

        auto arg2 =
            scalar_boolean_operand(operands[2], args, name_, codename_, ctx);
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<ir::range>&& farg,
                hpx::future<std::string>&& fdtype,
                hpx::future<std::uint8_t>&& fsparse) mutable {
                if (fsparse.get())
                {
                    return this_->generate_sparse_indices(
                        farg.get(), map_dtype(fdtype.get()), std::move(ctx));
                }
                return this_->generate_indices(
                    farg.get(), map_dtype(fdtype.get()), std::move(ctx));
            },
            std::move(arg0), std::move(arg1), std::move(arg2));
    }
}}}    // namespace phylanx::execution_tree::primitives
