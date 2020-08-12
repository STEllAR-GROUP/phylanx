// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_STATISTICS_LOGSUMEXP_OPERATION)
#define PHYLANX_STATISTICS_LOGSUMEXP_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/common/statistics_operations.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Compute the log of the sum of exponentials of input elements.
    /// \param a          The scalar, vector, or matrix to perform calculations
    ///                   over
    /// \param axis       Optional. If provided, logsumexp is calculated along
    ///                   the provided axis.
    /// \param keep_dims  Optional. If true the output has to have the same
    ///                   number of dimensions as a. Otherwise, the axes with
    ///                   size one will be reduced.
    class logsumexp_operation
      : public statistics_base<common::statistics_logsumexp_op,
            logsumexp_operation>
    {
        using base_type = statistics_base<common::statistics_logsumexp_op,
            logsumexp_operation>;

    public:
        static match_pattern_type const match_data;

        logsumexp_operation() = default;

        logsumexp_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_logsumexp_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "logsumexp", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif
