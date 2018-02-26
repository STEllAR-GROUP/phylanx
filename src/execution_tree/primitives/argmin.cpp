//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/argmin.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_argmin(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("argmin");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const argmin::match_data =
    {
        hpx::util::make_tuple("__argmin",
            std::vector<std::string>{"argmin(_1, _2)"},
            &create_argmin, &create_primitive<argmin>)
    };

    ///////////////////////////////////////////////////////////////////////////
    argmin::argmin(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct argmin : std::enable_shared_from_this<argmin>
        {
            argmin(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {}

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                HPX_THROW_EXCEPTION(hpx::not_implemented,
                    "argmin::eval",
                    generate_error_message(
                        "the argmin primitive is not implemented yet",
                        name_, codename_));
            }
        };
    }

    hpx::future<primitive_argument_type> argmin::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::argmin>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::argmin>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
