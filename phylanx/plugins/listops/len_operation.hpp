// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_len_operation_FEB_25_2018_1030AM)
#define PHYLANX_len_operation_FEB_25_2018_1030AM

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
    /// \brief Length of lists and strings
    /// Returns the number of elements in argument a or the length of the
    /// string argument a
    /// \param a It may be either a list or a vector
    class len_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<len_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        len_operation() = default;

        ///
        /// \brief Creates a PhySL list by concatenating its arguments
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        len_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_len_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "len", std::move(operands), name, codename);
    }
}}}

#endif
