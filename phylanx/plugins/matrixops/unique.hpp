//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_UNIQUE)
#define PHYLANX_PRIMITIVES_UNIQUE

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
    /// \brief Implementation of unique as a Phylanx primitive.
    /// Returns the sorted unique elements of array a.
    /// This implementation is intended to behave like [NumPy implementation of unique]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.unique.html).
    /// \param a an array

    class unique
      : public primitive_component_base
      , public std::enable_shared_from_this<unique> {
    protected:
        hpx::future<primitive_argument_type> eval(
                primitive_arguments_type const &operands,
                primitive_arguments_type const &args,
                eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        unique() = default;

        unique(primitive_arguments_type &&operands, std::string const &name,
               std::string const &codename);

    private:
        primitive_argument_type unique0d(primitive_arguments_type&& args) const;
        primitive_argument_type unique1d(primitive_arguments_type&& args) const;
        primitive_argument_type unique2d(primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type unique0d(ir::node_data<T>&& args) const;

        template <typename T>
        primitive_argument_type unique1d(ir::node_data<T>&& args) const;

        template <typename T>
        primitive_argument_type unique2d_flatten(ir::node_data<T>&& arg) const;

        template <typename T>
        primitive_argument_type unique2d_x_axis(ir::node_data<T>&& arg) const;

        template <typename T>
        primitive_argument_type unique2d_y_axis(ir::node_data<T>&& arg) const;

        template <typename T>
        primitive_argument_type unique2d(std::size_t numargs,
            ir::node_data<T>&& arg, std::int64_t axis) const;
    };

    inline primitive create_unique(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "unique", std::move(operands), name, codename);
    }
}}}

#endif
