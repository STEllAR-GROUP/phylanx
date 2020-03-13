//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/format.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    localities_information::localities_information(
        primitive_argument_type const& arg, annotation const& ann,
        std::string const& name, std::string const& codename)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "localities")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::localities_information",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type ({})", key),
                    name, codename));
        }

        locality_ =
            extract_locality_information(ann, name, codename);

        // extract the globally unique name identifying this object
        annotation name_ann;
        if (!ann.find("name", name_ann, name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_transpose_operation::transpose1d",
                util::generate_error_message(
                    "locality annotation does not hold a globally unique name",
                    name, codename));
        }

        annotation_ = extract_annotation_information(
            name_ann, name, codename);

        // extract all tile information
        tiles_.reserve(locality_.num_localities_);
        for (std::size_t i = 0; i != locality_.num_localities_; ++i)
        {
            std::string meta_key = hpx::util::format("meta_{}", i);
            annotation meta;
            if (!ann.find(meta_key, meta, name, codename))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "localities_information::localities_information",
                    util::generate_error_message(
                        hpx::util::format(
                            "annotation {} missing from localities information",
                            meta_key),
                        name, codename));
            }

            annotation tile;
            if (!meta.find("tile", tile, name, codename))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "localities_information::localities_information",
                    util::generate_error_message(
                        hpx::util::format(
                            "annotation 'tile' missing from {} information",
                            meta_key),
                        name, codename));
            }
            tiles_.emplace_back(std::move(tile), name, codename);
        }

        // we should have found the information for at least one tile
        if (tiles_.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::localities_information",
                util::generate_error_message(
                    "no tile information available",
                    name, codename));
        }

        std::size_t dim = tiles_[0].dimension();
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            // make sure that all tiles have the same dimensionality
            if (tiles_[i].dimension() != dim)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "localities_information::localities_information",
                    util::generate_error_message(
                        "inconsistent dimensionalities in tiles",
                        name, codename));
            }
        }

        // make sure the tile information matches the dimensions of the given
        // array
        if (extract_numeric_value_dimension(arg, name, codename) != dim)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::localities_information",
                util::generate_error_message(
                    "inconsistent dimensionalities between data and "
                    "tile information", name, codename));
        }

        if (extract_numeric_value_dimensions(arg, name, codename) !=
            dimensions(name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::localities_information",
                util::generate_error_message(
                    "inconsistent dimensionalities between data and "
                    "tile information", name, codename));
        }
    }

    localities_information::localities_information(std::size_t dim,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
        static std::atomic<std::size_t> count(0);

        locality_ = locality_information(0, 1);
        annotation_ = annotation_information(
            hpx::util::format(
                "annotation_{}_{}", hpx::get_locality_id(), ++count),
            0ll);
        tiles_.emplace_back(dim, dims);
    }

    ////////////////////////////////////////////////////////////////////////////
    localities_information extract_localities_information(
        primitive_argument_type const& arg,
        std::string const& name, std::string const& codename)
    {
        annotation localities;
        if (!arg.get_annotation_if("localities", localities, name, codename) &&
            !arg.find_annotation("localities", localities, name, codename))
        {
            std::size_t dim = extract_numeric_value_dimension(
                arg, name, codename);
            auto dims = extract_numeric_value_dimensions(
                arg, name, codename);

            return localities_information(dim, dims);
        }
        return localities_information(arg, localities, name, codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    std::size_t localities_information::num_dimensions() const
    {
        HPX_ASSERT(tiles_.size() == locality_.num_localities_);
        return tiles_[locality_.locality_id_].dimension();
    }

    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    localities_information::dimensions(
        std::string const& name, std::string const& codename) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result{};
        auto const& local_tile = tiles_[locality_.locality_id_];
        switch (local_tile.dimension())
        {
        case 0:
            result[0] = 1;
            break;

        case 1:
            {
                if (local_tile.spans_[0].is_valid())
                {
                    result[0] = local_tile.spans_[0].size();
                }
                else
                {
                    HPX_ASSERT(local_tile.spans_[1].is_valid());
                    result[0] = local_tile.spans_[1].size();
                }
            }
            break;

        case 2:
            // (rows, columns)
            result[0] = local_tile.spans_[0].size();
            result[1] = local_tile.spans_[1].size();
            break;

        case 3:
            // (pages, rows, columns)
            result[0] = local_tile.spans_[0].size();
            result[1] = local_tile.spans_[1].size();
            result[2] = local_tile.spans_[2].size();
            break;

        case 4:
            // (quats, pages, rows, columns)
            result[0] = local_tile.spans_[0].size();
            result[1] = local_tile.spans_[1].size();
            result[2] = local_tile.spans_[2].size();
            result[3] = local_tile.spans_[3].size();
            break;

        default:
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "localities_information::dimensions",
                    util::generate_error_message(
                        "unexpected number of dimensions", name, codename));
            }
            break;
        }

        return result;
    }

    namespace detail
    {
        template <std::size_t N>
        std::size_t dimension(std::vector<tiling_information> const& tiles)
        {
            HPX_ASSERT(!tiles.empty());
            auto it_min = std::min_element(tiles.begin(), tiles.end(),
                [&](tiling_information const& v, tiling_information const& smallest)
                {
                    HPX_ASSERT(N < v.spans_.size());
                    HPX_ASSERT(v.spans_[N].is_valid());
                    return v.spans_[N].start_ < smallest.spans_[N].start_;
                });

            auto it_max = std::max_element(tiles.begin(), tiles.end(),
                [&](tiling_information const& largest, tiling_information const& v)
                {
                    HPX_ASSERT(N < v.spans_.size());
                    HPX_ASSERT(v.spans_[N].is_valid());
                    return largest.spans_[N].stop_ < v.spans_[N].stop_;
                });

            return it_max->spans_[N].stop_ - it_min->spans_[N].start_;
        }
    }

    // Is this helpful? It may only introduce confusion
    std::size_t localities_information::size() const
    {
        return detail::dimension<0>(tiles_);
    }

    // we assume that all tiles have the same number of dimension
    std::size_t localities_information::quats() const
    {
        HPX_ASSERT(tiles_[0].dimension() == 4);
        return detail::dimension<0>(tiles_);
    }

    std::size_t localities_information::pages() const
    {
        switch (tiles_[0].dimension())
        {
        case 3:
            return detail::dimension<0>(tiles_);

        case 4:
            return detail::dimension<1>(tiles_);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::pages",
            util::generate_error_message(
                "unexpected dimensionality for calling pages()"));
    }

    std::size_t localities_information::rows() const
    {
        switch (tiles_[0].dimension())
        {
        case 1:
            // assertion that it's a row vector
            return detail::dimension<0>(tiles_);

        case 2:
            return detail::dimension<0>(tiles_);

        case 3:
            return detail::dimension<1>(tiles_);

        case 4:
            return detail::dimension<2>(tiles_);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::rows",
            util::generate_error_message(
                "unexpected dimensionality for calling rows()"));
    }

    std::size_t localities_information::columns() const
    {
        switch (tiles_[0].dimension())
        {
        case 1:
            return detail::dimension<0>(tiles_);

        case 2:
            return detail::dimension<1>(tiles_);

        case 3:
            return detail::dimension<2>(tiles_);

        case 4:
            return detail::dimension<3>(tiles_);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "localities_information::columns",
            util::generate_error_message(
                "the given dimensionality is unsupported"));
    }

    ////////////////////////////////////////////////////////////////////////////
    bool localities_information::has_span(std::size_t dim) const
    {
        HPX_ASSERT(locality_.locality_id_ < tiles_.size());
        HPX_ASSERT(dim < num_dimensions());

        return tiles_[locality_.locality_id_].spans_[dim].is_valid();
    }

    tiling_span localities_information::get_span(std::size_t dim) const
    {
        HPX_ASSERT(locality_.locality_id_ < tiles_.size());
        HPX_ASSERT(dim < num_dimensions());

        return tiles_[locality_.locality_id_].spans_[dim];
    }

    // project given (global) span onto local span
    tiling_span localities_information::project_coords(
        std::uint32_t loc, std::size_t dim, tiling_span const& span) const
    {
        HPX_ASSERT(loc < tiles_.size());
        HPX_ASSERT(dim < num_dimensions());

        auto const& gspan = tiles_[loc].spans_[dim];
        tiling_span result{
            span.start_ - gspan.start_, span.stop_ - gspan.start_};
        return result;
    }

//     // convert into an annotation
//     annotation localities_information::as_annotation(
//         std::string const& name, std::string const& codename) const
//     {
//         annotation meta;
//         for (std::size_t i = 0; i != locality_.num_localities_; ++i)
//         {
//             auto tile_ann = tiles_[i].as_annotation(name, codename);
//             meta.add_annotation(hpx::util::format("meta_{}", i),
//                 tile_ann.get_data(), name, codename);
//         }
//
//         return annotation("localities",
//             locality_.as_annotation(), std::move(meta),
//             annotation_.as_annotation());
//     }
}}


