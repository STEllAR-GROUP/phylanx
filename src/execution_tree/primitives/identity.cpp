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
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("identity");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const identity::match_data =
    {
        hpx::util::make_tuple("identity",
            std::vector<std::string>{"identity(_1)"},
            &create_identity, &create_primitive<identity>)
    };

    ///////////////////////////////////////////////////////////////////////////
    identity::identity(std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename    )
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type identity::identity_nd(operand_type&& op) const
    {
        if (op.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::identity_nd",
                execution_tree::generate_error_message(
                    "input should be a scalar",
                    name_, codename_));
        }

        std::size_t dim = static_cast<std::size_t>(op.scalar());
        return primitive_argument_type{
            operand_type{blaze::IdentityMatrix<double>(dim)}};
    }

    hpx::future<primitive_argument_type> identity::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                execution_tree::generate_error_message(
                    "the identity primitive requires"
                        "at most one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                execution_tree::generate_error_message(
                    "the identity primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operand_type&& op0) -> primitive_argument_type
            {
                return this_->identity_nd(std::move(op0));
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> identity::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
