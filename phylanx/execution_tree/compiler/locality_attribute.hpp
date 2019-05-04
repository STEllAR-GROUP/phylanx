//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_LOCALITY_ATTRIBUTE_HPP)
#define PHYLANX_EXECUTION_TREE_LOCALITY_ATTRIBUTE_HPP

#include <phylanx/config.hpp>

#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <string>

namespace phylanx { namespace execution_tree { namespace compiler
{
    // Attempt to parse the given attribute as a locality specification
    PHYLANX_EXPORT hpx::id_type parse_locality_attribute(
        std::string const& attr);

    PHYLANX_EXPORT bool parse_locality_attribute(
        std::string const& name, hpx::id_type& parts);

    // Compose an attribute referencing a locality
    PHYLANX_EXPORT std::string compose_locality_attribute(
        hpx::id_type const& locality);
}}}

#endif


