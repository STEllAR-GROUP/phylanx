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

                arg object : return the phylanx type name for the object

            Returns:)"
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
                if(args.size() == 0) {
                    std::string name = "None";
                    return primitive_argument_type{name};
                }
                std::string name;
                int tid = args[0].index();
                switch(tid) {
                case 0:
                    name = "None";
                    break;
                case 1:
                    name = "node_data<uint8>";
                    break;
                case 2:
                    name = "node_data<int64>";
                    break;
                case 3:
                    name = "string";
                    break;
                case 4:
                    name = "node_data<float>";
                    break;
                case 5:
                    name = "primitive";
                    break;
                case 6:
                    name = "vector<expression>";
                    break;
                case 7:
                    name = "range";
                    break;
                case 8:
                    name = "dict";
                    break;
                default:
                    name  = "??";
                    break;
                }
                return primitive_argument_type{name};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
