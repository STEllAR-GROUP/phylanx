//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ANY_OPERATION_FEB_25_2018_1307PM)
#define PHYLANX_PRIMITIVES_ANY_OPERATION_FEB_25_2018_1307PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class any_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<any_operation>
    {
    protected:
        using arg_type = ir::node_data<std::uint8_t>;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        any_operation() = default;

        any_operation(primitive_arguments_type&& params,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type any0d(T&& arg) const;

        template <typename T>
        primitive_argument_type any1d(T&& arg) const;

        template <typename T>
        primitive_argument_type any2d(T&& arg) const;

        template <typename T>
        primitive_argument_type any_nd(T&& arg) const;
    };

    inline primitive create_any_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "any", std::move(operands), name, codename);
    }
}}}

#endif
