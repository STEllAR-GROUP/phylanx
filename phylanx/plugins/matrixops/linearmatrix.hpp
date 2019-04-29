// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM)
#define PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
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
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        linearmatrix() = default;

        linearmatrix(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type linmatrix(std::int64_t nx, std::int64_t ny,
            primitive_argument_type&& x0, primitive_argument_type&& dx,
            primitive_argument_type&& dy) const;

        template <typename T>
        primitive_argument_type linmatrix(
            std::int64_t nx, std::int64_t ny, T x0, T dx, T dy) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_linearmatrix(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        static std::string type("linearmatrix");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }
}}}

#endif

