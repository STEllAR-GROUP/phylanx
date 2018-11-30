// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_append_operation_JULY_13_2018_0400PM)
#define PHYLANX_append_operation_JULY_13_2018_0400PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class append_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<append_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    private:
        primitive_argument_type handle_list_operands(
            primitive_argument_type&& op1, primitive_argument_type&& rhs) const;

    public:
        static match_pattern_type const match_data;

        append_operation() = default;

        append_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_append_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "append", std::move(operands), name, codename);
    }
}}}

#endif
