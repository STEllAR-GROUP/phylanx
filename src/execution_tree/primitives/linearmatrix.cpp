//  Copyright (c) 2018 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/execution_tree/primitives/linearmatrix.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <blaze/Math.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::linearmatrix> linearmatrix_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(linearmatrix_type, phylanx_linearmatrix_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(linearmatrix_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const linearmatrix::match_data =
    {
        hpx::util::make_tuple("linearmatrix",
            std::vector<std::string>{"linearmatrix(_1, _2, _3, _4, _5)"},
                &create<linearmatrix>)
    };

    ///////////////////////////////////////////////////////////////////////////
    linearmatrix::linearmatrix(std::vector<primitive_argument_type>&& args)
      : base_primitive(std::move(args))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct linearmatrix : std::enable_shared_from_this<linearmatrix>
        {
            linearmatrix() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;
            using matrix_type = blaze::DynamicMatrix<double>;

            primitive_argument_type linmatrix(args_type&& args) const
            {

                if (5 != args.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "linearmatrix::linmatrix",
                        "linearmatrix primitive requires 5 arguments.");
                }

                std::size_t nx = extract_integer_value(args[0]);
                std::size_t ny = extract_integer_value(args[1]);
                double base_value = args[2].scalar();
                double dx = args[3].scalar();
                double dy = args[4].scalar();

                if (nx < 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::detail::linearmatrix",
                        "the size of matrix in dimension x must at least be one.");
                }

                if (ny < 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::detail::linearmatrix",
                        "the size of matrix in dimension y must at least be one.");
                }

                matrix_type result{nx, ny};

                for(int x = 0; x < nx; x++)
                {
                    for(int y = 0; y < ny; y++)
                    {
                        result(x, y) = base_value + dx * x + dy * y;
                    }
                }

                return arg_type{std::move(result)};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 5)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::linearmatrix",
                        "The linearmatrix primitive requires exactly five arguments.");
                }

                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linearmatrix::eval",
                            "at least one of the arguments passed to linearmatrix"
                               "is not valid.");
                    }
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_argument_type
                    {
                        return this_->linmatrix(std::move(args));
                    }),
                    detail::map_operands(operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> linearmatrix::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::linearmatrix>()->eval(args, noargs);
        }
        return std::make_shared<detail::linearmatrix>()->eval(operands_, args);
    }
}}}
