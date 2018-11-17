//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_ALS_May_07_2018_0500PM)
#define PHYLANX_ALS_May_07_2018_0500PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class als
      : public primitive_component_base
      , public std::enable_shared_from_this<als>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        als() = default;

        ///
        /// Creates a primitive executing the ALS algorithm on the given
        /// input data
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        als(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        primitive_argument_type calculate_als(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_als(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "als", std::move(operands), name, codename);
    }
}}}

#endif
