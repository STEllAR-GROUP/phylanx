//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM)
#define PHYLANX_PRIMITIVES_ALL_OPERATION_FEB_25_2018_1307PM

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
    class all_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<all_operation>
    {
    protected:
        using arg_type = ir::node_data<std::uint8_t>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        all_operation() = default;

        all_operation(std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        template <typename T>
        primitive_argument_type all0d(T&& arg) const;

        template <typename T>
        primitive_argument_type all1d(T&& arg) const;

        template <typename T>
        primitive_argument_type all2d(T&& arg) const;

        template <typename T>
        primitive_argument_type all_nd(T&& arg) const;
    };

    inline primitive create_all_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "all", std::move(operands), name, codename);
    }
}}}

#endif
