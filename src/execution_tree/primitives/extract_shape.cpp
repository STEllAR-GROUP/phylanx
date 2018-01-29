//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/extract_shape.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::extract_shape>
    extract_shape_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    extract_shape_type, phylanx_extract_shape_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(extract_shape_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const extract_shape::match_data =
    {
        hpx::util::make_tuple("shape",
            std::vector<std::string>{"shape(_1, _2)", "shape(_1)"},
            &create<extract_shape>)
    };

    ///////////////////////////////////////////////////////////////////////////
    extract_shape::extract_shape(
            std::vector<primitive_argument_type> && operands)
      : base_primitive(std::move(operands))
    {}

    namespace detail
    {
        struct shape : std::enable_shared_from_this<shape>
        {
            shape() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.empty() || operands.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        "the extract_shape primitive requires one or two "
                            "operands");
                }

                if (!valid(operands[0]) ||
                    (operands.size() == 2 && !valid(operands[1])))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        "the extract_shape primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type && args) -> primitive_result_type
                    {
                        auto dims = args[0].dimensions();
                        if (args.size() == 1)
                        {
                            // return a list of numbers representing the
                            // dimensions of the first argument
                            std::vector<primitive_result_type> result{
                                std::int64_t(dims[0]), std::int64_t(dims[1])
                            };
                            return primitive_result_type{std::move(result)};
                        }

                        return primitive_result_type{
                            std::int64_t(dims[std::size_t(args[1][0])])};
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_result_type> extract_shape::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::shape>()->eval(args, noargs);
        }

        return std::make_shared<detail::shape>()->eval(operands_, args);
    }
}}}
