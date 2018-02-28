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

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class any_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<any_operation>
    {
    protected:
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        any_operation() = default;

        any_operation(std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type any0d(args_type&& args) const;
        primitive_argument_type any1d(args_type&& args) const;
        primitive_argument_type any2d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_any_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
