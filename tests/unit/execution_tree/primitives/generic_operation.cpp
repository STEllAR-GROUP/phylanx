//
// Created by tianyi on 3/16/18.
//

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>
#include <utility>

void test_generic_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive generic =
            phylanx::execution_tree::primitives::create_generic_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(lhs)}, "log");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            generic.eval();
    
    HPX_TEST_EQ(std::log(5.0),
                phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}
int main(int argc, char* argv[]){
    test_generic_operation_0d();
    return hpx::util::report_errors();
}