//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_IDENTITY_JAN_08_2018_0243PM)
#define PHYLANX_PRIMITIVES_IDENTITY_JAN_08_2018_0243PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class identity
      : public primitive_component_base
      , public std::enable_shared_from_this<identity>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using matrix_type = blaze::IdentityMatrix<double>;
        using operands_type = std::vector<operand_type>;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        identity() = default;

        identity(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type identity_helper(std::int64_t&& op) const;
        primitive_argument_type identity_nd(std::int64_t&& op) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_identity(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "identity", std::move(operands), name, codename);
    }
}}}

#endif
