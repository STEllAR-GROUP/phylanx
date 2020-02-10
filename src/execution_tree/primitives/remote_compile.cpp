//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/remote_compile.hpp>
#include <phylanx/execution_tree/compiler_component.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/runtime/naming/name.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_remote_compile(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("remote_compile");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const remote_compile::match_data =
    {
        hpx::util::make_tuple("remote_compile",
            std::vector<std::string>{"remote_compile(_1,_2)"},
            &create_remote_compile, &create_primitive<remote_compile>,
            R"(loc, func
            Args:

                loc (int) - locality
                func (string) - the function to run at the remote locality

            Returns:
              Compiles and runs the function on the remote locality
            )")
    };

    ///////////////////////////////////////////////////////////////////////////
    remote_compile::remote_compile(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> remote_compile::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "remote_compile::eval",
                generate_error_message(
                    "phylanx.name requires exactly two arguments"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                // yyy
                using namespace phylanx::execution_tree::detail;

                std::int64_t loc_id = extract_scalar_integer_value(args[0], this_->name_, this_->codename_);
                auto code = extract_string_value(args[1], this_->name_, this_->codename_);
                std::cout << "loc=" << loc_id << " code=" << code << std::endl;
                auto id = hpx::naming::get_id_from_locality_id(loc_id);
                physl_compiler pc(hpx::components::new_<phylanx::execution_tree::compiler_component>(id));
                pc.compile(code);

                std::string name = "answer";
                return primitive_argument_type{std::move(name)};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
