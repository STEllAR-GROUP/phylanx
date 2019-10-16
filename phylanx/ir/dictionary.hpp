// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DICTIONARY)
#define PHYLANX_DICTIONARY

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>
#include <hpx/serialization/serialization_fwd.hpp>
#include <hpx/serialization/unordered_map.hpp>

#include <cstddef>
#include <functional>
#include <unordered_map>

namespace phylanx { namespace execution_tree {
    struct primitive_argument_type;
}}

namespace phylanx { namespace ir
{
    using dictionary = std::unordered_map<
        phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>,
        phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>>;
}}

namespace std {
    template <>
    struct hash<phylanx::util::recursive_wrapper<
        phylanx::execution_tree::primitive_argument_type>>
    {
        using argument_type = phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>;
        using result_type = std::size_t;
        PHYLANX_EXPORT result_type operator()(argument_type const& s) const noexcept;
    };
}

#endif
