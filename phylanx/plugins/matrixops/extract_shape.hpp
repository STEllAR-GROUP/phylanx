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
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        extract_shape() = default;

        extract_shape(primitive_arguments_type&& params,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type shape0d() const;
        primitive_argument_type shape0d(std::int64_t index) const;

        primitive_argument_type shape1d(std::int64_t size) const;
        primitive_argument_type shape1d(
            std::int64_t size, std::int64_t index) const;

        primitive_argument_type shape2d(
            std::int64_t rows, std::int64_t columns) const;
        primitive_argument_type shape2d(
            std::int64_t rows, std::int64_t columns, std::int64_t index) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type shape3d(
            std::int64_t pages, std::int64_t rows, std::int64_t columns) const;
        primitive_argument_type shape3d(std::int64_t pages, std::int64_t rows,
            std::int64_t columns, std::int64_t index) const;
#endif
    };

    inline primitive create_extract_shape(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "shape", std::move(operands), name, codename);
    }
}}}

#endif


