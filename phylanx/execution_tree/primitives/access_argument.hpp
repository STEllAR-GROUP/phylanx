//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ACCESS_ARGUMENT_OCT_20_2017_0804PM)
#define PHYLANX_PRIMITIVES_ACCESS_ARGUMENT_OCT_20_2017_0804PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class access_argument
      : public primitive_component_base
      , public std::enable_shared_from_this<access_argument>
    {
    public:
        static match_pattern_type const match_data;

        access_argument() = default;

        access_argument(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context) const override;

        void store(primitive_arguments_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;

        void store(primitive_argument_type&& data,
            primitive_arguments_type&& params, eval_context ctx) override;

    private:
        std::size_t argnum_;
    };
}}}

#endif


