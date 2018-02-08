//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_PERFORMANCE_DATA)
#define PHYLANX_UTIL_PERFORMANCE_DATA

#include <phylanx/config.hpp>

#include <hpx/runtime/find_here.hpp>

#include <map>
#include <vector>


namespace phylanx { namespace util
{
    /// Retrieve performance counter data for selected primitives
    /// \param primitive_instances The primitives for which performance counter 
    /// data is required
    /// \param locality_id The locality the performance counter data is going 
    /// to be queried from
    /// \param counter_name_last_parts A vector containing the last part of the 
    /// performance counter names. 
    /// e.g. std::vector{ "count/eval", "count/direct_eval" }
    /// \return a std::map containing a key/value pair of primitive 
    /// instances/performance counter values
    /// \note primitive_instances are not verified
    /// \exception hpx::exception
    PHYLANX_EXPORT std::map<std::string, std::vector<std::int64_t>>
    retrieve_counter_data(std::vector<std::string> const& primitive_instances,
        std::vector<std::string> const& counter_name_last_parts,
        hpx::naming::id_type const& locality_id = hpx::find_here());
}}
#endif
