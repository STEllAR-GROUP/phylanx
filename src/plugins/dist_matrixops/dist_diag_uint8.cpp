//  Copyright (c) 2020 Hartmut Kaiser
//  Copyright (c) 2020 Nanmiao Wu
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/dist_diag.hpp>
#include <phylanx/plugins/dist_matrixops/dist_diag_impl.hpp>

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    // explicitly instantiate the required functions

    ///////////////////////////////////////////////////////////////////////////

    template execution_tree::primitive_argument_type dist_diag::dist_diag1d(
        ir::node_data<std::uint8_t>&& arr, std::int64_t k,
        std::string const& tiling_type, std::uint32_t tile_idx,
        std::uint32_t numtiles,
        execution_tree::localities_information&& arr_localities) const;

}}}