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
#include <tuple>

#include <blaze/Math.h>

namespace tile_calculation
{
    ///////////////////////////////////////////////////////////////////////////
    inline std::tuple<std::int64_t, std::size_t> tile_calculation_1d(
        std::uint32_t const& tile_idx, std::size_t const& dim,
        std::uint32_t const& numtiles)
    {
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
    tile_calculation_2d(std::uint32_t const& tile_idx,
        std::size_t const& row_dim, std::size_t const& column_dim,
        std::uint32_t const& numtiles, std::string const& tiling_type)
    {
        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;
        if (tiling_type == "row" ||
            (row_dim > column_dim && numtiles == 2 && tiling_type != "column"))
        {
            // row_tiling (horizontal tiling)
            if (row_dim < numtiles)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tile_calculation::tile_calculation_2d",
                    phylanx::util::generate_error_message(
                        "the row dimension should not be less than number of "
                        "tiles"));
            }

            column_start = 0;
            column_size = column_dim;
            std::tie(row_start, row_size) =
                tile_calculation_1d(tile_idx, row_dim, numtiles);
        }
        else if (tiling_type == "column" ||
            (row_dim < column_dim && numtiles == 2))
        {
            // column_tiling (vertical tiling)
            if (column_dim < numtiles)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tile_calculation::tile_calculation_2d",
                    phylanx::util::generate_error_message(
                        "the column dimension should not be less than number "
                        "of tiles"));
            }

            row_start = 0;
            row_size = row_dim;
            std::tie(column_start, column_size) =
                tile_calculation_1d(tile_idx, column_dim, numtiles);
        }
        else if (tiling_type == "sym" && numtiles == 4)
        {
            // distributing the matrix over 4 blocks
            if (column_dim < 2 || row_dim < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tile_calculation::tile_calculation_2d",
                    phylanx::util::generate_error_message(
                        "each dimension should have at least the length of 2 "
                        "to make 4 blocks"));
            }

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
                if (column_dim < first_div || row_dim < second_div)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "tile_calculation::tile_calculation_2d",
                        phylanx::util::generate_error_message(
                            "at least one of the marix dimensions is smaller "
                            "than the number of tiles in that direction"));
                }
                std::tie(row_start, row_size) = tile_calculation_1d(
                    blaze::floor(tile_idx / first_div), row_dim, second_div);
                std::tie(column_start, column_size) = tile_calculation_1d(
                    tile_idx % first_div, column_dim, first_div);
            }
            else
            {
                // larger column_dim
                if (column_dim < second_div || row_dim < first_div)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "tile_calculation::tile_calculation_2d",
                        phylanx::util::generate_error_message(
                            "at least one of the marix dimensions is smaller "
                            "than the number of tiles in that direction"));
                }
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
}
#endif
