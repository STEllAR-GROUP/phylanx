//  Copyright (c) 2018 Hartmut Kaiser
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
    class make_list
      : public primitive_component_base
      , public std::enable_shared_from_this<make_list>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        make_list() = default;

        ///
        /// \brief Creates a PhySL list by concatenating its arguments
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        make_list(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_make_list(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "make_list", std::move(operands), name, codename);
    }
}}}

#endif
