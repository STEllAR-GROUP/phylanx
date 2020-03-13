//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/format.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    tiling_span::tiling_span(ir::range const& data, std::string const& name,
        std::string const& codename)
    {
        if (data.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_span::tiling_span",
                util::generate_error_message(
                    "unexpected number of 'tile' annotation data elements",
                    name, codename));
        }

        auto it = data.begin();
        start_ = extract_scalar_integer_value(*it, name, codename);
        stop_ = extract_scalar_integer_value(*++it, name, codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        tiling_span extract_span(annotation const& ann, char const* key,
            std::string const& name, std::string const& codename)
        {
            annotation key_ann;
            if (!ann.get_if(key, key_ann, name, codename) &&
                !ann.find(key, key_ann, name, codename))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tiling_annotations_1d::tiling_annotations_1d",
                    util::generate_error_message(
                        hpx::util::format(
                            "annotation type not given '{}'", key),
                        name, codename));
            }
            return tiling_span(key_ann.get_data(), name, codename);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_information::tiling_information(annotation const& ann,
        std::string const& name, std::string const& codename)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "tile")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotations::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format(
                        "unexpected annotation type ({})", key),
                    name, codename));
        }

        for (std::size_t i = PHYLANX_MAX_DIMENSIONS; i != 0; --i)
        {
            annotation tile_ann;
            if (ann.find(get_span_name(i - 1), tile_ann, name, codename))
            {
                spans_.push_back(detail::extract_span(
                    tile_ann, get_span_name(i - 1), name, codename));
            }
        }
    }

    tiling_information::tiling_information(std::size_t dim,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
        spans_.reserve(dim);
        for (std::size_t i = 0; i != dim; ++i)
        {
            spans_.emplace_back(0, dims[i]);
        }
        for (std::size_t i = dim; i != PHYLANX_MAX_DIMENSIONS; ++i)
        {
            spans_.emplace_back(0, 0);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_information_1d::tiling_information_1d(
            annotation const& ann, std::string const& name,
            std::string const& codename)
      : type_(ann.has_key("columns", name, codename) ? columns : rows)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "tile" && key != "tile1d")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_1d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type ({})", key),
                    name, codename));
        }
        span_ = detail::extract_span(ann, tile1d_typename(type_), name, codename);
    }

    tiling_information_1d::tiling_information_1d(
            tiling_information const& tile, std::string const& name,
            std::string const& codename)
      : type_(columns)
    {
        if (tile.spans_.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_1d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type"),
                    name, codename));
        }

        if (tile.spans_[0].is_valid())
        {
            span_ = tile.spans_[0];
        }
        else if (tile.spans_.size() >= 2 && tile.spans_[1].is_valid())
        {
            type_ = rows;
            span_ = tile.spans_[1];
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_1d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type"),
                    name, codename));
        }
    }

    annotation tiling_information_1d::as_annotation(
        std::string const& name, std::string const& codename) const
    {
        return annotation{ir::range("tile",
            ir::range(tile1d_typename(type_), span_.start_, span_.stop_))};
    }

    void tiling_information_1d::transpose()
    {
        type_ = (type_ == columns) ? rows : columns;
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_information_2d::tiling_information_2d(annotation const& ann,
        std::string const& name, std::string const& codename)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "tile" && key != "tile2d")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_2d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type ({})", key),
                    name, codename));
        }

        spans_[0] = detail::extract_span(ann, "rows", name, codename);
        spans_[1] = detail::extract_span(ann, "columns", name, codename);
    }

    tiling_information_2d::tiling_information_2d(tiling_information const& tile,
        std::string const& name, std::string const& codename)
    {
        if (tile.spans_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_2d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type"),
                    name, codename));
        }

        spans_[1] = tile.spans_[1];
        spans_[0] = tile.spans_[0];
    }

    annotation tiling_information_2d::as_annotation(
        std::string const& name, std::string const& codename) const
    {
        return annotation{ir::range("tile",
            ir::range("rows", spans_[0].start_, spans_[0].stop_),
            ir::range("columns", spans_[1].start_, spans_[1].stop_))};
    }

    void tiling_information_2d::transpose()
    {
        std::swap(spans_[0], spans_[1]);
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_information_3d::tiling_information_3d(annotation const& ann,
        std::string const& name, std::string const& codename)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "tile" && key != "tile3d")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_3d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type ({})", key),
                    name, codename));
        }

        spans_[0] = detail::extract_span(ann, "pages", name, codename);
        spans_[1] = detail::extract_span(ann, "rows", name, codename);
        spans_[2] = detail::extract_span(ann, "columns", name, codename);
    }

    tiling_information_3d::tiling_information_3d(tiling_information const& tile,
        std::string const& name, std::string const& codename)
    {
        if (tile.spans_.size() < 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotation_3d::tiling_annotations",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type"),
                    name, codename));
        }

        spans_[2] = tile.spans_[2];
        spans_[1] = tile.spans_[1];
        spans_[0] = tile.spans_[0];
    }

    annotation tiling_information_3d::as_annotation(
        std::string const& name, std::string const& codename) const
    {
        return annotation{ir::range("tile",
            ir::range("pages", spans_[0].start_, spans_[0].stop_),
            ir::range("rows", spans_[1].start_, spans_[1].stop_),
            ir::range("columns", spans_[2].start_, spans_[2].stop_))};
    }

    void tiling_information_3d::transpose(
        std::int64_t const* data, std::size_t count)
    {
        HPX_ASSERT(count == 3);
        tiling_span spans[3] = {
            spans_[data[0]], spans_[data[1]], spans_[data[2]]};

        spans_[0] = spans[0];
        spans_[1] = spans[1];
        spans_[2] = spans[2];
    }
}}

