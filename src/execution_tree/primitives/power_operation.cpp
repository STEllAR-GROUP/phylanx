//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/power_operation.hpp>
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
    primitive create_power_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("power");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const power_operation::match_data =
    {
        hpx::util::make_tuple("power",
            std::vector<std::string>{"power(_1, _2)"},
            &create_power_operation, &create_primitive<power_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    power_operation::power_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct power : std::enable_shared_from_this<power>
        {
            power(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type power0d(operands_type && ops) const
            {
                ops[0] = double(std::pow(ops[0].scalar(), ops[1][0]));
                return primitive_argument_type{std::move(ops[0])};
            }

            primitive_argument_type power1d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                lhs = blaze::pow(lhs.vector(), rhs[0]);

                return primitive_argument_type{std::move(lhs)};
            }

            primitive_argument_type power2d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                lhs = blaze::pow(lhs.matrix(), rhs[0]);

                return primitive_argument_type{std::move(lhs)};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        generate_error_message(
                            "the power_operation primitive requires "
                                "exactly two operands",
                            name_, codename_));
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        generate_error_message(
                            "the power_operation primitive requires "
                                "that the arguments given by the operands "
                                "array are valid",
                            name_, codename_));
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_argument_type
                    {
                        if (ops[1].num_dimensions() != 0)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "power_operation::eval",
                                generate_error_message(
                                    "right hand side operand has to be a "
                                        "scalar value",
                                    this_->name_, this_->codename_));
                        }

                        switch (ops[0].num_dimensions())
                        {
                        case 0:
                            return this_->power0d(std::move(ops));

                        case 1:
                            return this_->power1d(std::move(ops));

                        case 2:
                            return this_->power2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "power_operation::eval",
                                generate_error_message(
                                    "left hand side operand has unsupported "
                                        "number of dimensions",
                                    this_->name_, this_->codename_));
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name_, codename_));
            }
        };
    }

    hpx::future<primitive_argument_type> power_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::power>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::power>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
