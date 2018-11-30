// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_DIM)
#define PHYLANX_PRIMITIVES_ADD_DIM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Creates a vector from scalar values and a column matrix from
    /// vectors
    /// \param a may either be a scalar or a vector
    class expand_dims
      : public primitive_component_base
      , public std::enable_shared_from_this<expand_dims>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        expand_dims() = default;

        expand_dims(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type add_dim_0d(ir::node_data<T>&& arg) const;

        template <typename T>
        primitive_argument_type add_dim_1d(
            ir::node_data<T>&& arg, std::int64_t axis) const;

        primitive_argument_type add_dim_0d(primitive_arguments_type&& arg) const;
        primitive_argument_type add_dim_1d(primitive_arguments_type&& arg) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type add_dim_2d(
            ir::node_data<T>&& arg, std::int64_t axis) const;

        primitive_argument_type add_dim_2d(primitive_arguments_type&& arg) const;
#endif
    };

    inline primitive create_expand_dims(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "expand_dims", std::move(operands), name, codename);
    }
}}}

#endif
