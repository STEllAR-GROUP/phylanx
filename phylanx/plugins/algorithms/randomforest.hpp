//  Copyright (c) 2018 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MAKE_LIST_NOV_10_2018_1848AM)
#define PHYLANX_MAKE_LIST_NOV_10_2018_1848AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <tuple>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <blaze/Blaze.h>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class randomforest
      : public primitive_component_base
      , public std::enable_shared_from_this<randomforest>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        randomforest() = default;

        ///
        /// Creates a primitive executing the RandomForest algorithm on 
        /// the given input data
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        randomforest(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        primitive_argument_type calculate_randomforest(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_randomforest(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "randomforest", std::move(operands), name, codename);
    }
}}}

#endif
