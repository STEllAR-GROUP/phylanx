// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

// condition is false, no iteration is performed
void test_for_operation_false()
{
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    phylanx::execution_tree::primitive cond =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), false);

    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

// condition is set to false in first iteration
void test_for_operation_true()
{
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), true);
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                cond, false
            });

    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_for_operation_42()
{
    //initial condition init is set to 1.0
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{1.0});

    phylanx::execution_tree::primitive forty_two =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive one =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{1.0});

    //check if init is less than 42 (this is true)
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::less>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, forty_two
            });

    //in the body add 1 to init to make it 42 which makes the condition false
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, hpx::new_<phylanx::execution_tree::primitives::add_operation>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    init, one
                })
            });

    //evaluate the for loop
    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(body)
            });

    //when the loop ends the value should be 42.0 as the condition will fail
    //init is equal to 42.0
    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST_EQ(phylanx::execution_tree::extract_numeric_value(f.get())[0],42.0);
}

void test_for_operation_42_with_store()
{
    //set init to zero
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    //store 3 to init
    phylanx::execution_tree::primitive store =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, std::move(rhs)
            });

    phylanx::execution_tree::primitive forty_two =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive five =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{5.0});

    //check if init is less than 42 (this is true)
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::less>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, forty_two
            });

    //in the body add five to init to make it greater than
    // 42 which makes the condition false
    
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, hpx::new_<phylanx::execution_tree::primitives::add_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                        init, five
                    })
            });

    //evaluate the for loop
    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(store), std::move(cond), std::move(body)
            });

    //when the loop ends the value should be 43.0 as the condition will fail
    //init is greater than 42.0
    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST_EQ(phylanx::execution_tree::extract_numeric_value(f.get())[0],43.0);
}

int main(int argc, char* argv[])
{
    test_for_operation_false();
    test_for_operation_true();
    test_for_operation_42();
    test_for_operation_42_with_store();

    return hpx::util::report_errors();
}


