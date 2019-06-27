//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/dist_matrixops/tiling_annotations.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/format.hpp>

#include <cstdint>
#include <string>

namespace phylanx { namespace dist_matrixops
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

        using execution_tree::extract_scalar_integer_value;

        auto it = data.begin();
        start_ = extract_scalar_integer_value(*it, name, codename);
        stop_ = extract_scalar_integer_value(*++it, name, codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        tiling_span extract_span(execution_tree::annotation const& ann,
            char const* key, std::string const& name,
            std::string const& codename)
        {
            execution_tree::annotation key_ann;
            if (!ann.find(key, key_ann, name, codename))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tiling_annotations_1d::tiling_annotations_1d",
                    util::generate_error_message(
                        hpx::util::format(
                            "'{}' annotation type not given", key),
                        name, codename));
            }
            return tiling_span(key_ann.get_data(), name, codename);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_annotations_1d::tiling_annotations_1d(
            execution_tree::annotation const& ann, std::string const& name,
            std::string const& codename)
      : type_(ann.has_key("columns", name, codename) ? columns : rows)
    {
        if (ann.get_type(name, codename) != "tile")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotations_1d::tiling_annotations_1d",
                util::generate_error_message(
                    "unexpected annotation type", name, codename));
        }
        span_ = detail::extract_span(ann, tile1d_typename(type_), name, codename);
    }

    execution_tree::annotation tiling_annotations_1d::as_annotation() const
    {
        return execution_tree::annotation{ir::range("tile",
            ir::range(tile1d_typename(type_), span_.start_, span_.stop_))};
    }

    void tiling_annotations_1d::transpose()
    {
        type_ = (type_ == columns) ? rows : columns;
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_annotations_2d::tiling_annotations_2d(
        execution_tree::annotation const& ann, std::string const& name,
        std::string const& codename)
    {
        if (ann.get_type(name, codename) != "tile")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotations_2d::tiling_annotations_1d",
                util::generate_error_message(
                    "unexpected annotation type", name, codename));
        }

        row_span_ = detail::extract_span(ann, "rows", name, codename);
        column_span_ = detail::extract_span(ann, "columns", name, codename);
    }

    execution_tree::annotation tiling_annotations_2d::as_annotation() const
    {
        return execution_tree::annotation{ir::range("tile",
            ir::range("rows", row_span_.start_, row_span_.stop_),
            ir::range("columns", column_span_.start_, column_span_.stop_))};
    }

    void tiling_annotations_2d::transpose()
    {
        std::swap(column_span_, row_span_);
    }

    ////////////////////////////////////////////////////////////////////////////
    tiling_annotations_3d::tiling_annotations_3d(
        execution_tree::annotation const& ann, std::string const& name,
        std::string const& codename)
    {
        if (ann.get_type(name, codename) != "tile")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tiling_annotations_3d::tiling_annotations_1d",
                util::generate_error_message(
                    "unexpected annotation type", name, codename));
        }

        page_span_ = detail::extract_span(ann, "pages", name, codename);
        row_span_ = detail::extract_span(ann, "rows", name, codename);
        column_span_ = detail::extract_span(ann, "columns", name, codename);
    }

    execution_tree::annotation tiling_annotations_3d::as_annotation() const
    {
        return execution_tree::annotation{ir::range("tile",
            ir::range("pages", page_span_.start_, page_span_.stop_),
            ir::range("rows", row_span_.start_, row_span_.stop_),
            ir::range("columns", column_span_.start_, column_span_.stop_))};
    }
}}

