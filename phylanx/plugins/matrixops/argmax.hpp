// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMAX)
#define PHYLANX_PRIMITIVES_ARGMAX

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Implementation of argmax as a Phylanx primitive.
    /// Returns the index of the largest element of argument a.
    /// This implementation is intended to behave like [NumPy implementation of argmax]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.argmax.html).
    /// \param a It may be a scalar value, vector, or matrix
    /// \param axis The dimension along which the indices of the max value(s) should be found
    class argmax
      : public primitive_component_base
      , public std::enable_shared_from_this<argmax>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        argmax() = default;

        argmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type argmax0d(primitive_arguments_type&& args) const;
        primitive_argument_type argmax1d(primitive_arguments_type&& args) const;
        primitive_argument_type argmax2d(primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type argmax0d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type argmax1d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type argmax2d(std::size_t numargs,
            ir::node_data<T>&& args, std::int64_t axis) const;

        template <typename T>
        primitive_argument_type argmax2d_flatten(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argmax2d_x_axis(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type argmax2d_y_axis(ir::node_data<T>&& arg) const;
    };

    inline primitive create_argmax(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "argmax", std::move(operands), name, codename);
    }
}}}

#endif
