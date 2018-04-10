//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/range_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

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
    primitive create_range_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("range");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const range_operation::match_data =
    {
        hpx::util::make_tuple("range",
        std::vector<std::string>{"range(_1)", "range(_1, _2)", "range(_1, _2, _3)"},
        &create_range_operation, &create_primitive<range_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    range_operation::range_operation(
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type range_operation::generate_range(
        args_type&& args) const
    {
        switch (args.size())
        {
        case 1:
        {
            const std::int64_t stop = args[0].scalar();
            return ir::range(stop);
        }
        case 2:
        {
            const std::int64_t start = args[0].scalar();
            const std::int64_t stop = args[1].scalar();
            return ir::range(start, stop);
        }
        case 3:
        {
            const std::int64_t start = args[0].scalar();
            const std::int64_t stop = args[1].scalar();
            const std::int64_t step = args[2].scalar();
            return ir::range(start, stop, step);
        }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "range_operation::generate_range",
            execution_tree::generate_error_message(
                "range_operation needs at most three operands",
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> range_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "range_operation::eval",
                execution_tree::generate_error_message(
                    "the range_operation primitive requires exactly one, two, "
                    "or three operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "range_operation::eval",
                    execution_tree::generate_error_message(
                        "the range_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }


        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                for (auto const& i : args)
                {
                    if (i.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "range_operation::eval",
                            execution_tree::generate_error_message(
                                "all range_operation operands must be scalars",
                                this_->name_, this_->codename_));
                    }
                }
                return this_->generate_range(std::move(args));
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> range_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
