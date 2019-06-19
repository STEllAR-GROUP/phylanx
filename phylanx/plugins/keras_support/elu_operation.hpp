// Copyright (c) 2018-2019 Hartmut Kaiser
// Copyright (c) 2019 Jules Penuchot
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_ELU_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_ELU_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx {  namespace execution_tree {  namespace primitives
{
    //  TODO : Update doc
/// \brief Returns an array of the same shape which is the the differential
///        surrogate of the input and is defined as f(x) = ln(1 + e^x).
///        Both the ReLU and Elu are largely similar, except near 0 where
///        the Elu is enticingly smooth and differentiable.
///
/// \param a      The scalar, vector, matrix, or tensor to perform Elu over
///
    class elu_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<elu_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using val_type = double;
        using mat_type = ir::node_data<val_type>;
        using alpha_type = ir::node_data<val_type>;

    public:
        static match_pattern_type const match_data;

        elu_operation() = default;

        elu_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type elu0d(mat_type&& arg, double alpha) const;
        primitive_argument_type elu1d(mat_type&& arg, double alpha) const;
        primitive_argument_type elu2d(mat_type&& arg, double alpha) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type elu3d(mat_type&& arg, double alpha) const;
#endif
    };

    inline primitive create_elu_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "elu", std::move(operands), name, codename);
    }
}}}

#endif
