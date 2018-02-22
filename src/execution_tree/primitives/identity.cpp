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
    namespace detail {
        struct identity : std::enable_shared_from_this<identity>
        {
            identity(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using operand_type = ir::node_data<double>;
            using matrix_type = blaze::IdentityMatrix<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type identity_nd(operands_type&& ops) const
            {
                if (ops[0].num_dimensions() != 0)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "identity::identity_nd",
                        generate_error_message(
                            "input should be a scalar",
                            name_, codename_));

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
                        generate_error_message(
                            "the identity primitive requires"
                                "at most one operand",
                            name_, codename_));
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "identity::eval",
                        generate_error_message(
                            "the identity primitive requires that the "
                                "arguments given by the operands array "
                                "are valid",
                            name_, codename_));
                }
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& op0) -> primitive_argument_type
                    {
                        return this_->identity_nd(std::move(op0));
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name_, codename_));
            }
        };
    }

    hpx::future<primitive_argument_type> identity::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::identity>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::identity>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
