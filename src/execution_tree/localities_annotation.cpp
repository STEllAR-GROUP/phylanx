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

        std::size_t dim = 0;
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            // make sure that all tiles have the same dimensionality
            if (dim)
            {
                if (tiles_[i].spans_[0].is_valid())
                {
                    if (tiles_[i].dimension() != dim)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "localities_information::localities_information",
                            util::generate_error_message(
                                "inconsistent dimensionalities in tiles", name,
                                codename));
                    }
                }
            }

            if (tiles_[i].spans_[0].is_valid() ||
                (tiles_[i].spans_.size() > 1 && tiles_[i].spans_[1].is_valid()))
            {
                dim = tiles_[i].dimension();
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
            result[0] = 0;
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
                [&](tiling_information const& v,
                    tiling_information const& smallest)
                {
                    if ((N < v.spans_.size() && v.spans_[N].is_valid()))
                        return v.spans_[N].start_ < smallest.spans_[N].start_;

                    return false;
                });

            auto it_max = std::max_element(tiles.begin(), tiles.end(),
                [&](tiling_information const& largest,
                    tiling_information const& v)
                {
                    if (N < v.spans_.size() && v.spans_[N].is_valid())
                        return largest.spans_[N].stop_ < v.spans_[N].stop_;

                    return false;
                });

            HPX_ASSERT(it_min->spans_[N].start_ == 0);
            return it_max->spans_[N].stop_;
        }

        template <std::size_t N>
        bool dim_tiled(
            std::vector<tiling_information> const& tiles, std::size_t dim_size)
        {
            HPX_ASSERT(!tiles.empty());
            return std::all_of(
                tiles.begin(), tiles.end(), [&](tiling_information v)
                {
                    if (N < v.spans_.size() && v.spans_[N].is_valid())
                        return dim_size == v.spans_[N].size();
                    return true;
                });
        }
    }

    // Is this helpful? It may only introduce confusion
    std::size_t localities_information::size() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            if (tiles_[i].spans_[0].is_valid())
            {
                return detail::dimension<0>(tiles_);
            }
            else if (tiles_[i].spans_.size() > 1 &&
                tiles_[i].spans_[1].is_valid())
            {
                // a row-wise vector
                return detail::dimension<1>(tiles_);
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::size",
            util::generate_error_message("the given array does not have a "
                                         "valid span on any of localities"));
    }

    // we assume that all tiles have the same number of dimension
    std::size_t localities_information::quats() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 4:
                return detail::dimension<0>(tiles_);

            case 1: HPX_FALLTHROUGH;
            case 2: HPX_FALLTHROUGH;
            case 3: HPX_FALLTHROUGH;
            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::quats",
                util::generate_error_message(
                    "unexpected dimensionality for calling quats()"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::quats",
            util::generate_error_message(
                "cannot call quats() when all tiles are empty"));
    }

    std::size_t localities_information::pages() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 3:
                return detail::dimension<0>(tiles_);

            case 4:
                return detail::dimension<1>(tiles_);

            case 1: HPX_FALLTHROUGH;
            case 2: HPX_FALLTHROUGH;
            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::pages",
                util::generate_error_message(
                    "unexpected dimensionality for calling pages()"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::pages",
            util::generate_error_message(
                "cannot call pages() when all tiles are empty"));
    }

    std::size_t localities_information::rows() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 1:
                // assertion that it's a row vector
                HPX_ASSERT(tiles_[i].spans_[1].is_valid());
                return detail::dimension<1>(tiles_);

            case 2:
                return detail::dimension<0>(tiles_);

            case 3:
                return detail::dimension<1>(tiles_);

            case 4:
                return detail::dimension<2>(tiles_);

            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::rows",
                util::generate_error_message(
                    "unexpected dimensionality for calling rows()"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "localities_information::rows",
            util::generate_error_message(
                "cannot call rows() when all tiles are empty"));
    }

    std::size_t localities_information::columns() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 1:
                // it should be a column vector
                HPX_ASSERT(tiles_[i].spans_[0].is_valid());
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
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "localities_information::columns",
            util::generate_error_message(
                "cannot call columns() when all tiles are empty"));
    }

    ////////////////////////////////////////////////////////////////////////////
    bool localities_information::has_span(std::size_t dim) const
    {
        HPX_ASSERT(locality_.locality_id_ < tiles_.size());
        if (num_dimensions() != 0)
        {
            HPX_ASSERT(dim < num_dimensions() ||
                (dim == 1 && dim == num_dimensions()));
        }

        return std::any_of(
            tiles_.begin(), tiles_.end(), [dim](tiling_information tile) {
                return tile.spans_[dim].is_valid();
            });
    }

    tiling_span localities_information::get_span(std::size_t dim) const
    {
        HPX_ASSERT(locality_.locality_id_ < tiles_.size());
        if (num_dimensions() != 0)
        {
            HPX_ASSERT(dim < num_dimensions() ||
                (dim == 1 && dim == num_dimensions()));
        }

        return tiles_[locality_.locality_id_].spans_[dim];
    }

    // project given (global) span onto local span
    tiling_span localities_information::project_coords(
        std::uint32_t loc, std::size_t dim, tiling_span const& span) const
    {
        HPX_ASSERT(loc < tiles_.size());
        if (num_dimensions() != 0)
        {
            HPX_ASSERT(dim < num_dimensions() ||
                (dim == 1 && dim == num_dimensions()));
        }

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

    bool localities_information::is_row_tiled() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 2:
                return detail::dim_tiled<1>(tiles_, columns());

            case 1: HPX_FALLTHROUGH;
            case 3: HPX_FALLTHROUGH;
            case 4: HPX_FALLTHROUGH;
            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::is_row_tiled",
                util::generate_error_message(
                    "unexpected dimensionality for calling is_row_tiled()"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "localities_information::is_row_tiled",
            util::generate_error_message(
                "cannot call is_row_tiled() when all tiles are empty"));
    }

    bool localities_information::is_column_tiled() const
    {
        for (std::size_t i = 0; i != tiles_.size(); ++i)
        {
            switch (tiles_[i].dimension())
            {
            case 0:
                continue;

            case 2:
                return detail::dim_tiled<0>(tiles_, rows());

            case 1: HPX_FALLTHROUGH;
            case 3: HPX_FALLTHROUGH;
            case 4: HPX_FALLTHROUGH;
            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "localities_information::is_column_tiled",
                util::generate_error_message(
                    "unexpected dimensionality for calling is_column_tiled()"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "localities_information::is_column_tiled",
            util::generate_error_message(
                "cannot call is_column_tiled() when all tiles are empty"));
    }
}}


