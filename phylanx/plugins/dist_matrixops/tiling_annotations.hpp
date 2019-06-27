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

#include <cstdint>
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
        tiling_annotations_1d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        tiling_span column_span_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_annotations_2d
    {
        tiling_annotations_2d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        tiling_span row_span_;
        tiling_span column_span_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_annotations_3d
    {
        tiling_annotations_3d(execution_tree::annotation const& ann,
            std::string const& name, std::string const& codename);

        execution_tree::annotation as_annotation() const;

        tiling_span page_span_;
        tiling_span row_span_;
        tiling_span column_span_;
    };
}}

#endif
