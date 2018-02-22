//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/determinant.hpp>
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

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_determinant(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("determinant");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const determinant::match_data =
    {
        hpx::util::make_tuple("determinant",
            std::vector<std::string>{"determinant(_1)"},
            &create_determinant, &create_primitive<determinant>)
    };

    ///////////////////////////////////////////////////////////////////////////
    determinant::determinant(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct determinant : std::enable_shared_from_this<determinant>
        {
            determinant(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        private:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        generate_error_message(
                            "the determinant primitive requires"
                                "exactly one operand",
                            name_, codename_));
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        generate_error_message(
                            "the determinant primitive requires that the "
                                "argument given by the operands array is "
                                "valid",
                            name_, codename_));
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_argument_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        switch (dims)
                        {
                        case 0:
                            return this_->determinant0d(std::move(ops));

                        case 2:
                            return this_->determinant2d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "determinant::eval",
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

        protected:
            primitive_argument_type determinant0d(operands_type && ops) const
            {
                return primitive_argument_type{std::move(ops[0])};       // no-op
            }

            primitive_argument_type determinant2d(operands_type && ops) const
            {
                double d = blaze::det(ops[0].matrix());
                return primitive_argument_type{operand_type(d)};
            }
        };
    }

    hpx::future<primitive_argument_type> determinant::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::determinant>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::determinant>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
