//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LITERAL_SEP_05_2017_1105AM)
#define PHYLANX_PRIMITIVES_LITERAL_SEP_05_2017_1105AM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>

namespace phylanx { namespace primitives
{
    class HPX_COMPONENT_EXPORT literal_value
        : public base_primitive
        , public hpx::components::component_base<literal_value>
    {
    public:
        literal_value() = default;

        literal_value(ir::node_data<double> const& data)
            : data_(data)
        {}
        literal_value(ir::node_data<double> && data)
            : data_(std::move(data))
        {}

        hpx::future<ir::node_data<double>> eval() override
        {
            return hpx::make_ready_future(data_);
        }

    private:
        ir::node_data<double> data_;
    };
}}

#endif


