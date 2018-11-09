// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_SUM)
#define PHYLANX_PRIMITIVES_SUM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Sums the values of the elements of a vector or a matrix or
    ///        returns the value of the scalar that was given to it.
    /// \param a         The scalar, vector, or matrix to perform sum over
    /// \param axis      Optional. If provided, sum is calculated along the
    ///                  provided axis and a vector of results is returned.
    ///                  \p keep_dims is ignored if \p axis present. Must be
    ///                  nil if \p keep_dims is set
    /// \param keep_dims Optional. Whether the sum value has to have the same
    ///                  number of dimensions as \p a. Ignored if \p axis is
    ///                  anything except nil.
    class sum_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<sum_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using val_type = double;
        using arg_type = ir::node_data<val_type>;
        using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;

    public:
        static match_pattern_type const match_data;

        sum_operation() = default;

        sum_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type sum0d(arg_type&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        primitive_argument_type sum1d(arg_type&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        primitive_argument_type sum2d(arg_type&& arg,
            hpx::util::optional<std::int64_t> axis, bool keep_dims) const;
        primitive_argument_type sum2d_flat(
            arg_type&& arg, bool keep_dims) const;
        primitive_argument_type sum2d_axis0(arg_type&& arg) const;
        primitive_argument_type sum2d_axis1(arg_type&& arg) const;
    };

    inline primitive create_sum_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "sum", std::move(operands), name, codename);
    }
}}}

#endif
