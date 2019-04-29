// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/ndim.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const ndim::match_data =
    {
        hpx::util::make_tuple("ndim",
            std::vector<std::string>{"ndim(_1)"},
            &create_ndim, &create_primitive<ndim>, R"(
            a
            Args:

                a (array type): array to extract the number of dimensions for

            Returns:

            Number of array dimensions.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    ndim::ndim(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> ndim::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "ndim::eval",
                generate_error_message(
                    "the ndim primitive requires exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "ndim::eval",
                generate_error_message(
                    "the ndim primitive requires that the "
                     "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return value_operand(operands[0], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& arg)
                -> primitive_argument_type
                {
                    return primitive_argument_type{std::int64_t(
                        extract_numeric_value_dimension(
                            arg.get(), this_->name_, this_->codename_))};
                });
    }
}}}
