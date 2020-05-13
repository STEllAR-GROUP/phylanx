// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM)
#define PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
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
    class extract_shape
      : public primitive_component_base
      , public std::enable_shared_from_this<extract_shape>
    {
    public:
        enum shape_mode
        {
            local_mode,
            len_mode,
            dist_mode    // shape_d
        };

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        extract_shape() = default;

        extract_shape(primitive_arguments_type&& params,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type shape0d(primitive_argument_type&& arg) const;
        primitive_argument_type shape0d(
            primitive_argument_type&& arg, std::int64_t index) const;

        primitive_argument_type shape1d(primitive_argument_type&& arg) const;
        primitive_argument_type shape1d(
            primitive_argument_type&& arg, std::int64_t index) const;

        primitive_argument_type shape2d(primitive_argument_type&& arg) const;
        primitive_argument_type shape2d(
            primitive_argument_type&& arg, std::int64_t index) const;

        primitive_argument_type shape3d(primitive_argument_type&& arg) const;
        primitive_argument_type shape3d(
            primitive_argument_type&& arg, std::int64_t index) const;

        primitive_argument_type shape4d(primitive_argument_type&& arg) const;
        primitive_argument_type shape4d(
            primitive_argument_type&& arg, std::int64_t index) const;

    private:
        shape_mode mode_;
    };

    inline primitive create_extract_shape(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "shape", std::move(operands), name, codename);
    }

    inline primitive create_extract_dist_shape(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "shape_d", std::move(operands), name, codename);
    }
}}}

#endif


