// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM)
#define PHYLANX_PRIMITIVES_EXTRACT_SHAPE_OCT_25_2017_1237PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class extract_shape
        : public primitive_component_base
        , public std::enable_shared_from_this<extract_shape>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;

    public:
        static match_pattern_type const match_data;

        extract_shape() = default;

        extract_shape(std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type shape0d(args_type&& args) const;
        primitive_argument_type shape1d(args_type&& args) const;
        primitive_argument_type shape2d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_extract_shape(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif


