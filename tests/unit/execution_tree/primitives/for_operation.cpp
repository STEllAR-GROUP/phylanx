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
// init =0.0; reinit=0.0; //some values we will not use
// for(init, false, reinit, body)
void test_for_operation_false()
{
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

  phylanx::execution_tree::primitive reinit =
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
                std::move(init), std::move(cond), std::move(reinit), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

// condition is set to false in first iteration
// for(0.0, true, 0.0 , body )
// body sets condition to false
void test_for_operation_true()
{
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

  phylanx::execution_tree::primitive reinit =
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
                std::move(init), std::move(cond), std::move(reinit), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

//init is stated at zero
//for(init, init< 42,init=init+1; body)
//body: track the value of init just so that we can verify
//what its value was when the loop ended
//body(
//     set temp = init
//    )
void test_for_operation_42()
{
    //initial condition init is set to 0.0
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    phylanx::execution_tree::primitive forty_two =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive one =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{1.0});

    //check if init is less than to 42 (this is true)
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::less>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, forty_two
            });

    //set temp=0.0 in the beginning
     phylanx::execution_tree::primitive temp =
      hpx::new_<phylanx::execution_tree::primitives::variable>(
          hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    //do something in the body
    //here: set temp = init
    phylanx::execution_tree::primitive body =
      hpx::new_<phylanx::execution_tree::primitives::store_operation>(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              temp, init
          });

    //in the reinit statement, add 1 to init
    phylanx::execution_tree::primitive reinit =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                init, hpx::new_<phylanx::execution_tree::primitives::add_operation>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    init, one
                })
            });

    //evaluate the for loop until condition is false
    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(reinit) , std::move(body)
            });

    // when the loop ends the value obtained as the result
    // should be 42.0 as the condition will fail when
    // init is equal to 42.0
    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST_EQ(phylanx::execution_tree::extract_numeric_value(f.get())[0],41.0);
}


//for(init=3.0; init<42; init=init+5; body
//body: track the value of init just so that we can verify
//what its value was when the loop ended
//body(
//     set temp = init
//    )
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

    //in the reinit statement add five to init
    phylanx::execution_tree::primitive reinit =
      hpx::new_<phylanx::execution_tree::primitives::store_operation>(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              init, hpx::new_<phylanx::execution_tree::primitives::add_operation>(
                  hpx::find_here(),
                  std::vector<phylanx::execution_tree::primitive_argument_type>{
                      init, five
                  })
          });

    //set temp=0.0 in the beginning
    phylanx::execution_tree::primitive temp =
      hpx::new_<phylanx::execution_tree::primitives::variable>(
          hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    //do something in the body
    //here: set temp = init
    phylanx::execution_tree::primitive body =
      hpx::new_<phylanx::execution_tree::primitives::store_operation>(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              temp, init
          });

    //evaluate the for loop
    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(store), std::move(cond), std::move(reinit), std::move(body)
            });

    //when the loop ends the value should be 38.0 as the condition will fail
    //init is greater than 42.0
    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST_EQ(phylanx::execution_tree::extract_numeric_value(f.get())[0],38.0);
}

int main(int argc, char* argv[])
{
    test_for_operation_false();
    test_for_operation_true();
    test_for_operation_42();
    test_for_operation_42_with_store();

    return hpx::util::report_errors();
}


