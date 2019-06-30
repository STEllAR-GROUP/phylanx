//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TILING_ANNOTATIONS_JUN_19_2019_1002AM)
#define PHYLANX_PRIMITIVES_TILING_ANNOTATIONS_JUN_19_2019_1002AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/ranges.hpp>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>

namespace phylanx { namespace dist_matrixops
{
    ////////////////////////////////////////////////////////////////////////////
    struct tiling_span
    {
        tiling_span() = default;

        tiling_span(ir::range const& rhs, std::string const& name,
            std::string const& codename);

        std::int64_t start_ = 0;
        std::int64_t stop_ = 0;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_annotations_1d
    {
        enum tile1d_type
        {
            columns = 0,
            rows = 1
        };

        static char const* tile1d_typename(tile1d_type t)
        {
            return t == columns ? "columns" : "rows";
        }

        tiling_annotations_1d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        void transpose();

        tile1d_type type_;    // column or row span
        tiling_span span_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_annotations_2d
    {
        tiling_annotations_2d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        void transpose();

        tiling_span row_span_;
        tiling_span column_span_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_annotations_3d
    {
        tiling_annotations_3d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        void transpose(std::int64_t const* data, std::size_t count);
        void transpose(std::initializer_list<std::int64_t> data)
        {
            transpose(data.begin(), data.size());
        }

        tiling_span spans_[3];
        tiling_span column_span_;
    };
}}

#endif
