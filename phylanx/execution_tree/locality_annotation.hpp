//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LOCALITY_ANNOTATION_JUN_28_2019_1123AM)
#define PHYLANX_PRIMITIVES_LOCALITY_ANNOTATION_JUN_28_2019_1123AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <string>

namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    struct locality_information
    {
        PHYLANX_EXPORT locality_information();

        locality_information(
                std::uint32_t locality_id, std::uint32_t num_localities)
          : locality_id_(locality_id)
          , num_localities_(num_localities)
        {}

        PHYLANX_EXPORT locality_information(ir::range const& data,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT execution_tree::annotation as_annotation() const;

        std::uint32_t locality_id_;
        std::uint32_t num_localities_;
    };

    PHYLANX_EXPORT locality_information extract_locality_information(
        execution_tree::annotation const& ann, std::string const& name,
        std::string const& codename);
}}

#endif

