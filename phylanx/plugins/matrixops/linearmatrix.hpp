// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM)
#define PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class linearmatrix
     : public primitive_component_base
     , public std::enable_shared_from_this<linearmatrix>
    {
    protected:
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using matrix_type = blaze::DynamicMatrix<double>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        linearmatrix() = default;

        linearmatrix(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type linmatrix(args_type&& args) const;
    };

    inline primitive create_linearmatrix(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        static std::string type("linearmatrix");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }
}}}

#endif

