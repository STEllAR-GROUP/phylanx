//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/phyname.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_phyname(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("phyname");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const phyname::match_data =
    {
        hpx::util::make_tuple("phyname",
            std::vector<std::string>{"phyname(_1)"},
            &create_phyname, &create_primitive<phyname>,
            R"(arg
            Args:

                arg (primitive type) : return the name of the arg

            Returns:

                The name of the primitive type)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    phyname::phyname(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> phyname::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                std::size_t tid = 0;
                if(args.size() > 0)
                    tid = args[0].index();
                using namespace phylanx::execution_tree::detail;
                std::string name = get_primitive_argument_type_name(tid);
                return primitive_argument_type{name};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
