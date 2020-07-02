// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/mod_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/assign.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    //////////////////////////////////////////////////////////////////////////
    match_pattern_type const mod_operation::match_data =
    {
        match_pattern_type{"__mod",
            std::vector<std::string>{"_1 % _2", "__mod(_1, _2)"},
            &create_mod_operation, &create_primitive<mod_operation>, R"(
            x0, x1
            Args:

                 x0 (int): A dividend.\n"
                 x1 (int): A divisor.\n"

            Returns:

            The remainder of the division.)",
            true
        }
    };

    //////////////////////////////////////////////////////////////////////////
    mod_operation::mod_operation(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> mod_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "mod_operation::eval",
                generate_error_message("the mod_operation primitive requires "
                    "exactly two operands"));
        }

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
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "mod_operation::eval",
                generate_error_message(
                    "the mod_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }
        auto this_ = this->shared_from_this();

        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type
            {
                typedef phylanx::ir::node_data<std::int64_t> itype;
                using vector_type = blaze::DynamicVector<std::int64_t>;

                itype left_ = extract_integer_value(
                    std::move(args[0]), this_->name_, this_->codename_);
                itype right_ = extract_integer_value(
                    std::move(args[1]), this_->name_, this_->codename_);

                std::size_t left_dims = left_.num_dimensions();
                std::size_t right_dims = right_.num_dimensions();

                if(left_dims == 1 and right_dims == 0) {
                    assign_vector<itype> lvec{left_};
                    auto r = right_.scalar();
                    lvec = blaze::map(left_.vector(), [r](std::int64_t v){
                        return v % r;
                    });
                    primitive_argument_type p(std::move(left_));
                    return p;
                }
                if(right_dims == 1 and left_dims == 0) {
                    assign_vector<itype> rvec{right_};
                    auto l = left_.scalar();
                    rvec = blaze::map(right_.vector(), [l](std::int64_t v){
                        return l % v;
                    });
                    primitive_argument_type p(std::move(right_));
                    return p;
                }
                return primitive_argument_type{
                    left_.scalar() % right_.scalar()};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
