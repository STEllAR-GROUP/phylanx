//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/make_vector.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_make_vector(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("make_vector");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const make_vector::match_data =
    {
        hpx::util::make_tuple("make_vector",
            std::vector<std::string>{"make_vector(__1)"},
            &create_make_vector, &create_primitive<make_vector>)
    };

    ///////////////////////////////////////////////////////////////////////////
    make_vector::make_vector(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> make_vector::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "make_vector::eval",
                execution_tree::generate_error_message(
                    "the make_vector primitive requires that "
                        "the arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t vec_size = args.size();
                blaze::DynamicVector<double> temp(vec_size);

                for (std::size_t i = 0; i != vec_size; ++i)
                {
                    if (args[i].num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "make_vector::eval",
                            execution_tree::generate_error_message(
                                "the make_vector primitive requires that "
                                    "all arguments evaluate to zero-"
                                    "dimensional values",
                                this_->name_, this_->codename_));
                    }
                    temp[i] = args[i].scalar();
                }

                return primitive_argument_type{
                    ir::node_data<double>{storage1d_type{std::move(temp)}}};
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> make_vector::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
