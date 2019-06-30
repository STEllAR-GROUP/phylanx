//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <cstdint>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    locality_information::locality_information()
      : locality_id_(hpx::get_locality_id())
      , num_localities_(hpx::get_num_localities(hpx::launch::sync))
    {
    }

    locality_information::locality_information(ir::range const& data,
        std::string const& name, std::string const& codename)
    {
        if (data.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "locality_information::locality_information",
                util::generate_error_message(
                    "unexpected number of 'locality' annotation data elements",
                    name, codename));
        }

        using execution_tree::extract_scalar_integer_value;

        auto it = data.begin();
        locality_id_ = static_cast<std::uint32_t>(
            extract_scalar_integer_value(*it, name, codename));
        num_localities_ = static_cast<std::uint32_t>(
            extract_scalar_integer_value(*++it, name, codename));
    }

    locality_information extract_locality_information(
        execution_tree::annotation const& ann, std::string const& name,
        std::string const& codename)
    {
        if (ann.get_type() != "locality")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "detail::extract_locality_information",
                util::generate_error_message(
                    "'locality' annotation type not given",
                    name, codename));
        }
        return locality_information(ann.get_data(), name, codename);
    }
}}


