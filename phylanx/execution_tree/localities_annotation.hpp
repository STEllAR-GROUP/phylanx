//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LOCALITIES_ANNOTATION_JUL_01_2019_1050AM)
#define PHYLANX_PRIMITIVES_LOCALITIES_ANNOTATION_JUL_01_2019_1050AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    struct PHYLANX_EXPORT localities_information
    {
        localities_information() = default;

        localities_information(primitive_argument_type const& arg,
            annotation const& ann, std::string const& name,
            std::string const& codename);

        localities_information(std::size_t dim,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims);

        // extract dimensionality and sizes
        std::size_t num_dimensions() const;
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dimensions(
            std::string const& name, std::string const& codename) const;

        std::size_t size() const;

        std::size_t quats() const;
        std::size_t pages() const;
        std::size_t rows() const;
        std::size_t columns() const;

        // extract local span
        bool has_span(std::size_t dim) const;
        tiling_span get_span(std::size_t dim) const;

        // project given (global) span onto local span
        tiling_span project_coords(
            std::uint32_t loc, std::size_t dim, tiling_span const& span) const;

//         annotation as_annotation(
//             std::string const& name, std::string const& codename) const;

        locality_information locality_;
        annotation_information annotation_;
        std::vector<tiling_information> tiles_;
    };

    PHYLANX_EXPORT localities_information extract_localities_information(
        primitive_argument_type const& arg,
        std::string const& name, std::string const& codename);
}}

#endif

