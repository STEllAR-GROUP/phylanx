// Copyright (c) 2021 Karame M.Shokooh
// Copyright (c) 2021 Hartmut kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_TRIU_OPERATION)
#define PHYLANX_PRIMITIVES_TRIU_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief   Return an N x M matrix with ones on the k-th diagonal and
    ///          zeros elsewhere.
    /// \param N Number of rows in the output.
    /// \param M Optional. Number of columns in the output. If None, defaults
    ///          to N.
    /// \param k Optional. Index of the diagonal: 0 (the default) refers to the
    ///          main diagonal, a positive value refers to an upper diagonal,
    ///          and a negative value to a lower diagonal.
    /// \param dtype Optional. The data-type of the returned array (default:
    ///         'float')
    class triu_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<triu_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        triu_operation() = default;

        triu_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:

        template <typename T>
        primitive_argument_type triu2d(
        ir::node_data<T>&& arg, std::int64_t k) const;

        primitive_argument_type triu2d(
        primitive_argument_type&& arg, std::int64_t k) const;

        template <typename T>
        primitive_argument_type triu3d(
        ir::node_data<T>&& arg, std::int64_t k) const;

        primitive_argument_type triu3d(
        primitive_argument_type&& arg, std::int64_t k) const;

       
    };

    inline primitive create_triu_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "triu", std::move(operands), name, codename);
    }
}}}

#endif
