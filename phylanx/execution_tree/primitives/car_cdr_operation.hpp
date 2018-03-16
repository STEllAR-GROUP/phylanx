// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_CAR_CDR_OPERATION_MAR_16_2018_1233PM)
#define PHYLANX_CAR_CDR_OPERATION_MAR_16_2018_1233PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class car_cdr_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<car_cdr_operation>
    {
    public:
        static std::vector<match_pattern_type> const match_data;

        car_cdr_operation() = default;

        car_cdr_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        primitive_argument_type car(primitive_argument_type&& arg) const;
        primitive_argument_type cdr(primitive_argument_type&& arg) const;

    private:
        std::string operation_;
    };

    PHYLANX_EXPORT primitive create_car_cdr_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
