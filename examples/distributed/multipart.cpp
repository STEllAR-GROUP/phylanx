#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
std::string code1 = "define(a, 1)";

std::string code2 = R"(block(
    define(fx, arg0, block(
        cout(arg0 + 4),
        cout(42)
    )),
    fx
))";

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code, std::uint32_t locality_id)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            hpx::naming::get_id_from_locality_id(locality_id));

    return phylanx::execution_tree::compile(code, snippets, env);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    auto et1 = compile(code1, 1);
    auto r1 = et1();
    std::cout << r1 << '\n';

    //auto et2 = compile(code2, 1);
    //auto r2 = et2(r1);
    //std::cout << r2 << '\n';

    return 0;
}
