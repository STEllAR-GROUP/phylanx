//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMIN)
#define PHYLANX_PRIMITIVES_ARGMIN

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Implementation of argmax as a Phylanx primitive
    /// This implementation is intended to behave like [NumPy implementation
    /// of argmin]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.argmin.html).
    class argmin : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        argmin() = default;

        /// \brief Calculates argmin of a node_data
        /// \param args A scalar value, vector, or matrix
        argmin(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };

    PHYLANX_EXPORT primitive create_argmin(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
