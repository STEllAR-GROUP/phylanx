// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/reshape_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const reshape_operation::match_data =
    {
        hpx::util::make_tuple("reshape",
        std::vector<std::string>{"reshape(_1,_2)"},
        &create_reshape_operation, &create_primitive<reshape_operation>,
        "")
    };

    ///////////////////////////////////////////////////////////////////////////
    reshape_operation::reshape_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
        , dtype_(extract_dtype(name_))
    {}



    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> reshape_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "the reshape_operation primitive requires exactly two "
                    "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    util::generate_error_message(
                        "the reshape_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                primitive_arguments_type&& args)
                ->primitive_argument_type {



                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    util::generate_error_message("operand a has an invalid "
                        "number of dimensions",
                        this_->name_, this_->codename_));

        }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
