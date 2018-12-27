// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PROD)
#define PHYLANX_PRIMITIVES_PROD

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_prod_op;
    }

    /// \brief products the values of the elements of a vector or a matrix or
    ///        returns the value of the scalar that was given to it.
    /// \param a         The scalar, vector, or matrix to perform prod over
    /// \param axis      Optional. If provided, prod is calculated along the
    ///                  provided axis and a vector of results is returned.
    ///                  \p keep_dims is ignored if \p axis present. Must be
    ///                  nil if \p keep_dims is set
    /// \param keep_dims Optional. Whether the prod value has to have the same
    ///                  number of dimensions as \p a. Ignored if \p axis is
    ///                  anything except nil.
    /// This implementation is intended to behave like [NumPy implementation of prod]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.prod.html).
    class prod_operation
      : public statistics<detail::statistics_prod_op, prod_operation>
    {
        using base_type =
            statistics<detail::statistics_prod_op, prod_operation>;

    public:
        static match_pattern_type const match_data;

        prod_operation() = default;

        prod_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_prod_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "prod", std::move(operands), name, codename);
    }
}}}

#endif
