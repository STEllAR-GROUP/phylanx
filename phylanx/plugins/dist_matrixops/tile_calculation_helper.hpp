// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_MATRIXOPS_TILE_CALCULATION_HELPER)
#define PHYLANX_DIST_MATRIXOPS_TILE_CALCULATION_HELPER

#include <phylanx/util/generate_error_message.hpp>

#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

#include <blaze/Math.h>

namespace tile_calculation
{
    ///////////////////////////////////////////////////////////////////////////
    inline std::tuple<std::int64_t, std::size_t> tile_calculation_1d(
        std::uint32_t tile_idx, std::size_t dim, std::uint32_t numtiles)
    {
        if (dim < numtiles)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_calculation::tile_calculation_1d",
                phylanx::util::generate_error_message(
                    "the length of array in each dimension should not be less "
                    "than number of tiles on that dimension"));
        }

        // calculates start and size or the tile_index-th tile on the given
        // dimension with the given dim size
        std::int64_t start;
        std::size_t size = static_cast<std::size_t>(dim / numtiles);
        std::size_t remainder = dim % numtiles;
        if (tile_idx < remainder)
        {
            size++;
        }

        if (remainder != 0 && tile_idx >= remainder)
        {
            start = (size + 1) * remainder + size * (tile_idx - remainder);
        }
        else
        {
            start = size * tile_idx;
        }
        return std::make_tuple(start, size);
    }

    inline std::tuple<std::int64_t, std::size_t> tile_calculation_overlap_1d(
        std::int64_t start, std::size_t size, std::size_t dim,
        std::size_t intersection)
    {
        if (start + size == dim)    // bottom part
        {
            start -= intersection;
            size += intersection;
        }
        else if (start == 0)    // top part
        {
            size += intersection;
        }
        else    // middle part
        {
            start -= static_cast<std::size_t>(intersection);
            size += 2 * intersection;
        }

        if (start < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_calculation::tile_calculation_overlap_1d",
                phylanx::util::generate_error_message(
                    "the given intersection produces negative start for the "
                    "array"));
        }
        if (start + size > dim)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_calculation::tile_calculation_overlap_1d",
                phylanx::util::generate_error_message(
                    "the given intersection need an end point larger than the "
                    "array size"));
        }
        return std::make_tuple(start, size);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline std::tuple<std::size_t, std::size_t> middle_divisors(std::size_t num)
    {
        std::size_t first = blaze::sqrt(num);
        std::size_t second;
        while (true)
        {
            if (num % first == 0)
            {
                // the worst case scenario is a prime number where `first`
                // becomes 1
                second = num / first;
                break;
            }
            first -= 1;
        }
        return std::make_tuple(first, second);
    }

    inline std::tuple<std::int64_t, std::int64_t, std::size_t, std::size_t>
    tile_calculation_2d(std::uint32_t tile_idx, std::size_t row_dim,
        std::size_t column_dim, std::uint32_t numtiles,
        std::string const& tiling_type)
    {
        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;
        if (tiling_type == "row" ||
            (row_dim >= column_dim && numtiles == 2 && tiling_type != "column"))
        {
            // row_tiling (horizontal tiling)
            column_start = 0;
            column_size = column_dim;
            std::tie(row_start, row_size) =
                tile_calculation_1d(tile_idx, row_dim, numtiles);
        }
        else if (tiling_type == "column" ||
            (row_dim < column_dim && numtiles == 2))
        {
            // column_tiling (vertical tiling)
            row_start = 0;
            row_size = row_dim;
            std::tie(column_start, column_size) =
                tile_calculation_1d(tile_idx, column_dim, numtiles);
        }
        else if (tiling_type == "sym" && numtiles == 4)
        {
            // distributing the matrix over 4 blocks
            std::tie(row_start, row_size) =
                tile_calculation_1d(blaze::floor(tile_idx / 2), row_dim, 2);
            std::tie(column_start, column_size) =
                tile_calculation_1d(tile_idx % 2, column_dim, 2);
        }
        else if (tiling_type == "sym")
        {
            // the assumption is tiles are numbered in a row-major fashion
            std::uint32_t first_div,
                second_div;    // first_div is the smaller one
            std::tie(first_div, second_div) = middle_divisors(numtiles);
            if (row_dim > column_dim)
            {
                // larger number of rows (larger row_dim)
                std::tie(row_start, row_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / first_div), row_dim, second_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % first_div, column_dim, first_div);
            }
            else
            {
                // larger column_dim
                std::tie(row_start, row_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / second_div), row_dim, first_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % second_div, column_dim, second_div);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_calculation::tile_calculation_2d",
                phylanx::util::generate_error_message(
                    "the given tiling_type is invalid. tiling_type can be "
                    "`sym`, `row` or `column` for a matrix"));
        }
        return std::make_tuple(row_start, column_start, row_size, column_size);
    }

    ///////////////////////////////////////////////////////////////////////////
    inline std::tuple<std::size_t, std::size_t, std::size_t>
    three_middle_divisors(std::size_t num)
    {
        if (num == 2 || num == 3 || num == 5)
        {
            return std::make_tuple(
                static_cast<std::size_t>(1), static_cast<std::size_t>(1), num);
        }
        if (num == 4)
        {
            return std::make_tuple(static_cast<std::size_t>(1),
                static_cast<std::size_t>(2), static_cast<std::size_t>(2));
        }

        std::size_t second, third;
        std::size_t first = blaze::cbrt(num);    //cubic root
        while (true)
        {
            if (num % first == 0)
            {
                // the worst case scenario is a prime number where `first`
                // becomes 1
                std::tie(second, third) = middle_divisors(num / first);
                break;
            }
            first -= 1;
        }
        return std::make_tuple(first, second, third);
    }

    inline std::tuple<std::int64_t, std::int64_t, std::int64_t, std::size_t,
        std::size_t, std::size_t>
    tile_calculation_3d(std::uint32_t tile_idx, std::size_t page_dim,
        std::size_t row_dim, std::size_t column_dim, std::uint32_t numtiles,
        std::string const& tiling_type)
    {
        std::int64_t page_start, row_start, column_start;
        std::size_t page_size, row_size, column_size;
        if (tiling_type == "page")
        {
            // page_tiling
            row_start = column_start = 0;
            row_size = row_dim;
            column_size = column_dim;
            std::tie(page_start, page_size) =
                tile_calculation_1d(tile_idx, page_dim, numtiles);
        }
        else if (tiling_type == "row")
        {
            // row_tiling (horizontal tiling)
            page_start = column_start = 0;
            page_size = page_dim;
            column_size = column_dim;
            std::tie(row_start, row_size) =
                tile_calculation_1d(tile_idx, row_dim, numtiles);
        }
        else if (tiling_type == "column")
        {
            // column_tiling (vertical tiling)
            page_start = row_start = 0;
            page_size = page_dim;
            row_size = row_dim;
            std::tie(column_start, column_size) =
                tile_calculation_1d(tile_idx, column_dim, numtiles);
        }
        else if (tiling_type == "sym")
        {
            // the assumption is tiles are numbered in a row-major fashion in
            // each page (the default order of numpy.flatten())
            std::uint32_t first_div, second_div, third_div;
            std::tie(first_div, second_div, third_div) =
                three_middle_divisors(numtiles);

            if (column_dim <= row_dim && row_dim <= page_dim)
            {
                // column_dim(first_div) < row_dim(second_div) < page_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (first_div * second_div)), page_dim,
                    third_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / first_div)) %
                        second_div,
                    row_dim, second_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % first_div, column_dim, first_div);
            }
            else if (column_dim <= page_dim && page_dim <= row_dim)
            {
                // column_dim(first_div) < page_dim(second_div) < row_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (first_div * third_div)), page_dim,
                    second_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / first_div)) %
                        third_div,
                    row_dim, third_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % first_div, column_dim, first_div);
            }
            else if (row_dim <= page_dim && page_dim <= column_dim)
            {
                // row_dim(first_div) < page_dim(second_div) < column_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (first_div * third_div)), page_dim,
                    second_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / third_div)) %
                        first_div,
                    row_dim, first_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % third_div, column_dim, third_div);
            }
            else if (row_dim <= column_dim && column_dim <= page_dim)
            {
                // row_dim(first_div) < column_dim(second_div) < page_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (first_div * second_div)), page_dim,
                    third_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / second_div)) %
                        first_div,
                    row_dim, first_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % second_div, column_dim, second_div);
            }
            else if (page_dim <= column_dim && column_dim <= row_dim)
            {
                // page_dim(first_div) < column_dim(second_div) < row_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (second_div * third_div)), page_dim,
                    first_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / second_div)) %
                        third_div,
                    row_dim, third_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % second_div, column_dim, second_div);
            }
            else
            {
                // page_dim(first_div) < row_dim(second_div) < column_dim(third_div)
                std::tie(page_start, page_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / (second_div * third_div)), page_dim,
                    first_div);
                std::tie(row_start, row_size) = tile_calculation_1d(
                    static_cast<std::uint32_t>(
                        blaze::floor(tile_idx / third_div)) %
                        second_div,
                    row_dim, second_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % third_div, column_dim, third_div);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_calculation::tile_calculation_3d",
                phylanx::util::generate_error_message(
                    "the given tiling_type is invalid. tiling_type can be "
                    "`sym`, `page`, `row` or `column` for a tensor"));
        }
        return std::make_tuple(page_start, row_start, column_start, page_size,
            row_size, column_size);
    }
}
#endif
