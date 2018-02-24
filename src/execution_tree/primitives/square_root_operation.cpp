//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/square_root_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_square_root_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("square_root");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const square_root_operation::match_data =
    {
        hpx::util::make_tuple("square_root",
            std::vector<std::string>{"square_root(_1, _2)"},
            &create_square_root_operation,
            &create_primitive<square_root_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    square_root_operation::square_root_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct square_root : std::enable_shared_from_this<square_root>
        {
            square_root() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type square_root_0d(operands_type && ops) const
            {
                ops[0] = std::sqrt(ops[0].scalar());
                return primitive_argument_type{std::move(ops[0])};
            }

            primitive_argument_type square_root_1d(operands_type && ops) const
            {
                ops[0] = blaze::sqrt(ops[0].vector());

                return primitive_argument_type{std::move(ops[0])};
            }

            primitive_argument_type square_root_2d(operands_type && ops) const
            {
                ops[0] = blaze::sqrt(ops[0].matrix());

                return primitive_argument_type{std::move(ops[0])};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args,
                std::string const& name, std::string const& codename)
            {
                if (operands.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "square_root_operation::eval",
                        generate_error_message(
                            "the square_root_operation primitive "
                                "requires exactly one operand",
                            name, codename));
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "square_root_operation::eval",
                        generate_error_message(
                            "the square_root_operation primitive "
                                "requires that the argument given "
                                "by the operands array is valid",
                            name, codename));
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_, name, codename](operands_type&& ops)
                    -> primitive_argument_type
                    {

                        switch (ops[0].num_dimensions())
                        {
                        case 0:
                            return this_->square_root_0d(std::move(ops));

                        case 1:
                            return this_->square_root_1d(std::move(ops));

                        case 2:
                            return this_->square_root_2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "square_root_operation::eval",
                                generate_error_message(
                                    "left hand side operand has unsupported "
                                        "number of dimensions",
                                    name, codename));
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name, codename));
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> square_root_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::square_root>()->eval(
                args, noargs, name_, codename_);
        }
        return std::make_shared<detail::square_root>()->eval(
            operands_, args, name_, codename_);
    }
}}}
