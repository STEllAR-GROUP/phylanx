// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_DETAIL_RANGE_DIMENSION)
#define PHYLANX_UTIL_DETAIL_RANGE_DIMENSION

#include <array>
#include <cstddef>
#include <string>

namespace phylanx { namespace util { namespace detail {

    inline std::size_t extract_range_num_dimensions(
        phylanx::ir::range const& shape)
    {
        return shape.size();
    }

    inline std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_positive_range_dimensions(phylanx::ir::range const& shape,
        std::string const& name, std::string const& codename)
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result = {0};
        if (!shape.empty())
        {
            if (shape.size() == 1)
            {
                result[0] = extract_scalar_positive_integer_value_strict(
                    *shape.begin(), name, codename);
            }
            else if (shape.size() == 2)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_positive_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
            }
            else if (shape.size() == 3)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_positive_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
                result[2] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
            }
            else if (shape.size() == 4)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_positive_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
                result[2] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
                result[3] = extract_scalar_positive_integer_value_strict(
                    *++elem_1, name, codename);
            }
        }
        return result;
    }

    inline std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_nonneg_range_dimensions(phylanx::ir::range const& shape,
        std::string const& name, std::string const& codename)
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result = {0};
        if (!shape.empty())
        {
            if (shape.size() == 1)
            {
                result[0] = extract_scalar_nonneg_integer_value_strict(
                    *shape.begin(), name, codename);
            }
            else if (shape.size() == 2)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_nonneg_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
            }
            else if (shape.size() == 3)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_nonneg_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
                result[2] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
            }
            else if (shape.size() == 4)
            {
                auto elem_1 = shape.begin();
                result[0] = extract_scalar_nonneg_integer_value_strict(
                    *elem_1, name, codename);
                result[1] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
                result[2] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
                result[3] = extract_scalar_nonneg_integer_value_strict(
                    *++elem_1, name, codename);
            }
        }
        return result;
    }
}}}
#endif
