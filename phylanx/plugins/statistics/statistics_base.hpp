// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STATISTICS_DEC_24_2018_0227PM)
#define PHYLANX_PRIMITIVES_STATISTICS_DEC_24_2018_0227PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/futures/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {

    template <template <class T> class Op, typename Derived>
    class statistics_base
      : public primitive_component_base
      , public std::enable_shared_from_this<Derived>
    {
    private:
        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }
        Derived const& derived() const
        {
            return static_cast<Derived const&>(*this);
        }

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        statistics_base() = default;

        statistics_base(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        node_data_type dtype_;
    };
}}}    // namespace phylanx::execution_tree::primitives

#endif
