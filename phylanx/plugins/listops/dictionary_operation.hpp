// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DICTIONARY_OPERATION)
#define PHYLANX_PRIMITIVES_DICTIONARY_OPERATION

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
    /// \brief
    /// \param
    class dict_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<dict_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using arg_type = phylanx::execution_tree::primitive_argument_type;
        using args_type = std::vector<arg_type>;

    public:
        static match_pattern_type const match_data;

        dict_operation() = default;

        dict_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type generate_dict(
            std::vector<ir::range, arguments_allocator<ir::range>>&& args) const;
    };

    inline primitive create_dict_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "dict", std::move(operands), name, codename);
    }
}}}

#endif
