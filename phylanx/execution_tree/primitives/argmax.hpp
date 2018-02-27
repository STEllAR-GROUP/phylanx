//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMAX)
#define PHYLANX_PRIMITIVES_ARGMAX

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
    /// of argmax]
    /// (https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.argmax.html).
    class argmax
        : public primitive_component_base
        , public std::enable_shared_from_this<argmax>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using val_type = double;
        using arg_type = ir::node_data<val_type>;
        using args_type = std::vector<arg_type>;

    public:
        static match_pattern_type const match_data;

        argmax() = default;

        /// \brief Calculates argmax of a node_data
        /// \param args A scalar value, vector, or matrix
        argmax(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type argmax0d(args_type && args) const;
        primitive_argument_type argmax1d(args_type && args) const;
        primitive_argument_type argmax2d_flatten(arg_type && arg_a) const;
        primitive_argument_type argmax2d_x_axis(arg_type && arg_a) const;
        primitive_argument_type argmax2d_y_axis(arg_type && arg_a) const;
        primitive_argument_type argmax2d(args_type && args) const;
    };

    PHYLANX_EXPORT primitive create_argmax(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
