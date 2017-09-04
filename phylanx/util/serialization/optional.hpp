//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_OPTIONAL_SERIALIZATION_HPP)
#define PHYLANX_UTIL_OPTIONAL_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/util/optional.hpp>

#include <hpx/include/serialization.hpp>

namespace hpx { namespace serialization
{
    template <typename T>
    void save(
        output_archive& ar, phylanx::util::optional<T> const& o, unsigned)
    {
        bool const valid = bool(o);
        ar << valid;
        if (valid)
        {
            ar << *o;
        }
    }

    template <typename T>
    void load(input_archive& ar, phylanx::util::optional<T>& o, unsigned)
    {
        bool valid = false;
        ar >> valid;
        if (!valid)
        {
            o.reset();
            return;
        }

        T value;
        ar >> value;
        o.emplace(std::move(value));
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (phylanx::util::optional<T>));
}}

#endif //HPX_SERIALIZATION_VARIANT_HPP
