// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMAX)
#define PHYLANX_PRIMITIVES_ARGMAX

#include <phylanx/config.hpp>
#include <phylanx/plugins/matrixops/argminmax.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        struct argmax_op;
    }

    /// \brief Implementation of argmax as a Phylanx primitive.
    /// Returns the index of the largest element of argument a.
    /// This implementation is intended to behave like [NumPy implementation of argmax]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.argmax.html).
    /// \param a It may be a scalar value, vector, or matrix
    /// \param axis The dimension along which the indices of the max value(s) should be found
    class argmax
      : public argminmax<detail::argmax_op, argmax>
    {
        using base_type = argminmax<detail::argmax_op, argmax>;

    public:
        static match_pattern_type const match_data;

        argmax() = default;

        argmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_argmax(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "argmax", std::move(operands), name, codename);
    }
}}}

#endif
