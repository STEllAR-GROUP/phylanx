//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MAKE_LIST_FEB_25_2018_1030AM)
#define PHYLANX_MAKE_LIST_FEB_25_2018_1030AM

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
    class lra
      : public primitive_component_base
      , public std::enable_shared_from_this<lra>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        lra() = default;

        ///
        /// Creates a primitive executing the LRA algorithm on the given
        /// input data
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        lra(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        primitive_argument_type calculate_lra(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_lra(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "lra", std::move(operands), name, codename);
    }
}}}

#endif
