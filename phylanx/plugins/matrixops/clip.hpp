// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CLIP)
#define PHYLANX_PRIMITIVES_CLIP

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
    /// \brief Implementation of clip as a Phylanx primitive.
    /// Given an interval, values outside the interval are clipped to the interval edges.
    /// This implementation is intended to behave like [NumPy implementation of clip]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.clip.html).
    /// \param a It may be a scalar value, vector, or matrix
    /// \param a_min Minimum value, it may be scalar or array_like or None
    /// \param a_max Maximum valus, it may be scalar or array_like or None

    class clip
      : public primitive_component_base
      , public std::enable_shared_from_this<clip>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        clip() = default;

        clip(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type clipnd(ir::node_data<T>&& arg,
            ir::node_data<T>&& min, ir::node_data<T>&& max) const;

        template <typename T>
        primitive_argument_type clip0d(ir::node_data<T>&& arg,
            ir::node_data<T>&& min, ir::node_data<T>&& max) const;

        template <typename T>
        primitive_argument_type clip1d(ir::node_data<T>&& arg,
            ir::node_data<T>&& min, ir::node_data<T>&& max) const;

        template <typename T>
        primitive_argument_type clip2d(ir::node_data<T>&& arg,
            ir::node_data<T>&& min, ir::node_data<T>&& max) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type clip3d(ir::node_data<T>&& arg,
            ir::node_data<T>&& min, ir::node_data<T>&& max) const;
#endif

        template <typename T>
        primitive_argument_type clip_helper(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_clip(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "clip", std::move(operands), name, codename);
    }
}}}

#endif
