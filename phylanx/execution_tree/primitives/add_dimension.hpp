// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_DIM)
#define PHYLANX_PRIMITIVES_ADD_DIM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Creates a vector from scalar values and a column matrix from
    /// vectors
    /// \param a may either be a scalar or a vector
    class add_dimension
        : public primitive_component_base
        , public std::enable_shared_from_this<add_dimension>
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

        add_dimension() = default;

        add_dimension(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type add_dim_0d(args_type && args) const;
        primitive_argument_type add_dim_1d(args_type && args) const;
    };

    PHYLANX_EXPORT primitive create_add_dimension(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
