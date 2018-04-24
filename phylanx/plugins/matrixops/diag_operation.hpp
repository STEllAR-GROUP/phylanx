// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIAG_PRIMITIVE_HPP)
#define PHYLANX_DIAG_PRIMITIVE_HPP

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
    class diag_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<diag_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

    public:
        static match_pattern_type const match_data;

        diag_operation() = default;

        /**
         * @brief Diag Operation Primitive
         *
         * Constructs a diagonal matrix if the input is a vector,
         * extracts the diagonal if the input is a matrix and returns
         * unchanged input if the input is a scalar.
         *
         * @param operands Vector of phylanx node data objects of
         * size either one or two
         *
         * If used inside PhySL:
         *
         *      diag (input, k=0 )
         *
         *          input : Scalar, Vector or a Matrix
         *          k     : Which diagonal to extract or create, default being 0
         *
         * This primitives replicates the functionality of
         * <a href=
         * "https://docs.scipy.org/doc/numpy/reference/generated/numpy.diag.html"
         * >numpy.diag</a>
         */

        diag_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        primitive_argument_type diag0d(args_type&& args) const;
        primitive_argument_type diag1d(args_type&& args) const;
        primitive_argument_type diag2d(args_type&& args) const;
    };

    inline primitive create_diag_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "diag", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_DIAG_PRIMITIVE_HPP
