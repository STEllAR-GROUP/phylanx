// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CONSTANT_OCT_10_2017_0243PM)
#define PHYLANX_PRIMITIVES_CONSTANT_OCT_10_2017_0243PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class constant
      : public primitive_component_base
      , public std::enable_shared_from_this<constant>
    {
    public:
        enum class operation
        {
            constant,
            constant_like,
            full,
            full_like
        };

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        hpx::future<primitive_argument_type> eval_full(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const;

        hpx::future<primitive_argument_type> eval_full_like(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const;

        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static std::vector<match_pattern_type> const match_data;

        constant() = default;

        constant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type constant_nd(std::size_t numdims,
            primitive_argument_type&& value,
            operand_type::dimensions_type const& dims, node_data_type dtype,
            bool implements_like_, std::string const& name_,
            std::string const& codename_) const;

    private:
        operation operation_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_constant(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "constant", std::move(operands), name, codename);
    }
}}}

#endif
