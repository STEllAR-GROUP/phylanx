// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_COUNT_NONZERO_AUG_07_2018_0427AM)
#define PHYLANX_PRIMITIVES_COUNT_NONZERO_AUG_07_2018_0427AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class count_nonzero_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<count_nonzero_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        count_nonzero_operation() = default;

        count_nonzero_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args,
            eval_mode) const override;

    private:
        primitive_argument_type count_nonzero0d(primitive_argument_type&&) const;
        primitive_argument_type count_nonzero1d(primitive_argument_type&&) const;
        primitive_argument_type count_nonzero2d(primitive_argument_type&&) const;
    };

    inline primitive create_count_nonzero_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "count_nonzero", std::move(operands), name, codename);
    }
}}}

#endif
