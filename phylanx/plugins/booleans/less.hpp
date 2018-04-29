//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LESS_OCT_07_2017_0225PM)
#define PHYLANX_PRIMITIVES_LESS_OCT_07_2017_0225PM

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
    class less
      : public primitive_component_base
      , public std::enable_shared_from_this<less>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<primitive_argument_type>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        less() = default;

        less(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        template <typename T>
        primitive_argument_type less0d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less0d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less0d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less1d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less1d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less2d0d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less2d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;
        template <typename T>
        primitive_argument_type less_all(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, bool type_double) const;

    private:
        struct visit_less;
    };

    inline primitive create_less(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__lt", std::move(operands), name, codename);
    }
}}}

#endif


