//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TILING_ANNOTATIONS_JUN_19_2019_1002AM)
#define PHYLANX_PRIMITIVES_TILING_ANNOTATIONS_JUN_19_2019_1002AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    struct tiling_span
    {
        tiling_span() = default;

        tiling_span(ir::range const& rhs, std::string const& name,
            std::string const& codename);
        tiling_span(std::int64_t start, std::int64_t stop)
          : start_(start), stop_(stop)
        {}

        bool is_valid() const { return start_ < stop_; }

        std::int64_t size() const
        {
            return stop_ - start_;
        }

        std::int64_t start_ = 0;
        std::int64_t stop_ = 0;
    };

    // find intersection of two spans
    inline bool intersect(
        tiling_span const& lhs, tiling_span const& rhs, tiling_span& result)
    {
        if (lhs.start_ < rhs.stop_ && lhs.stop_ > rhs.start_)
        {
            result.start_ = (std::max)(lhs.start_, rhs.start_);
            result.stop_ = (std::min)(lhs.stop_, rhs.stop_);
            return true;
        }
        return false;
    }

    namespace detail
    {
        tiling_span extract_span(annotation const& ann, char const* key,
            std::string const& name, std::string const& codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_information
    {
        static char const* get_span_name(std::size_t i)
        {
            static const char* const names[] = {
                "columns", "rows", "pages", "quats"};
            return names[i];
        }

        PHYLANX_EXPORT tiling_information(annotation const& ann,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT tiling_information(std::size_t dim,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims);

        annotation as_annotation(
            std::string const& name, std::string const& codename) const
        {
            annotation ann("tile");
            for (std::size_t i = spans_.size(); i != 0; --i)
            {
                ann.add_annotation(get_span_name(i - 1),
                    ir::range(spans_[i - 1].start_, spans_[i - 1].stop_), name,
                    codename);
            }
            return ann;
        }

        // the number of annotations corresponds to the dimensionality of the
        // described data
        std::size_t dimension() const
        {
            std::size_t dim = 0;
            for (auto const& span : spans_)
            {
                if (span.is_valid())
                {
                    ++dim;
                }
            }
            return dim;
        }

        std::vector<tiling_span> spans_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_information_1d
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

        tiling_information_1d(tile1d_type type, tiling_span const& span)
          : type_(type)
          , span_(span)
        {}

        PHYLANX_EXPORT tiling_information_1d(annotation const& ann,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT tiling_information_1d(tiling_information const& info,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT annotation as_annotation(
            std::string const& name, std::string const& codename) const;

        PHYLANX_EXPORT void transpose();

        tile1d_type type_;    // column or row span
        tiling_span span_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_information_2d
    {
        tiling_information_2d(
            tiling_span const& row_span, tiling_span const& column_span)
          : spans_{row_span, column_span}
        {}

        PHYLANX_EXPORT tiling_information_2d(annotation const& ann,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT tiling_information_2d(tiling_information const& tile,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT annotation as_annotation(
            std::string const& name, std::string const& codename) const;

        PHYLANX_EXPORT void transpose();

        tiling_span spans_[2];    // row and column spans
    };

    ////////////////////////////////////////////////////////////////////////////
    struct tiling_information_3d
    {
        PHYLANX_EXPORT tiling_information_3d(annotation const& ann,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT tiling_information_3d(tiling_information const& tile,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT annotation as_annotation(
            std::string const& name, std::string const& codename) const;

        PHYLANX_EXPORT void transpose(
            std::int64_t const* data, std::size_t count);

        tiling_span spans_[3];    // page, row and column spans
    };
}}

#endif
