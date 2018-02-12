//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/identity.hpp>
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
    primitive create_identity(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("identity");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const identity::match_data =
    {
        hpx::util::make_tuple("identity",
            std::vector<std::string>{"identity(_1)"},
            &create_identity, &create_primitive<identity>)
    };

    ///////////////////////////////////////////////////////////////////////////
    identity::identity(std::vector<primitive_argument_type> && operands)
      : primitive_component_base(std::move(operands))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        struct identity : std::enable_shared_from_this<identity>
        {
            identity() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using matrix_type = blaze::IdentityMatrix<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type identity_nd(operands_type&& ops) const
            {
                if (ops[0].num_dimensions() != 0)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "identity::identity_nd",
                        "input should be a scalar");

                std::size_t dim = static_cast<std::size_t>(ops[0].scalar());
                return primitive_argument_type{
                    operand_type{blaze::IdentityMatrix<double>(dim)}};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() > 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "identity::eval",
                        "the identity primitive requires"
                        "at most one operand");
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "identity::eval",
                        "the identity primitive requires that the "
                        "arguments given by the operands array are valid");
                }
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& op0) -> primitive_argument_type
                    {
                        return this_->identity_nd(std::move(op0));
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> identity::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::identity>()->eval(args, noargs);
        }

        return std::make_shared<detail::identity>()->eval(operands_, args);
    }
}}}
