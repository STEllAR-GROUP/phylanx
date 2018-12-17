// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FLATTEN)
#define PHYLANX_PRIMITIVES_FLATTEN

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief Implementation of flatten as a Phylanx primitive.
    /// Returns a copy of the array collapsed into one dimension.
    /// This implementation is intended to behave like [NumPy implementation of flatten]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.chararray.
    /// flatten.html).
    /// \param a It may be a scalar value, vector, or matrix
    /// \param order The order from which the flattened array should be created from the
    /// input (optional)
    class flatten
      : public primitive_component_base
      , public std::enable_shared_from_this<flatten>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        flatten() = default;

        flatten(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        primitive_argument_type flatten0d(primitive_argument_type&& arg) const;
        primitive_argument_type flatten1d(primitive_argument_type&& arg) const;
        primitive_argument_type flatten2d(
            primitive_argument_type&& arg, std::string order) const;

        template <typename T>
        primitive_argument_type flatten0d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flatten1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flatten2d(
            ir::node_data<T>&& arg, std::string order) const;
    };

    inline primitive create_flatten(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "flatten", std::move(operands), name, codename);
    }
}}}

#endif
