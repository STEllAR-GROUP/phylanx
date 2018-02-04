//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

namespace pe = phylanx::execution_tree;

// condition is false, no iteration is performed
// init =0.0; reinit=0.0; //some values we will not use
// for(init, false, reinit, body)
void test_for_operation_false()
{
    pe::primitive init =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

  pe::primitive reinit =
      hpx::new_<pe::primitives::variable>(
          hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    pe::primitive cond =
            hpx::new_<pe::primitives::variable>(
                    hpx::find_here(), false);

    pe::primitive body =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    pe::primitive for_ =
        hpx::new_<pe::primitives::for_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(reinit),
                std::move(body)
            });

    hpx::future<pe::primitive_argument_type> f =
        for_.eval();

    HPX_TEST(!pe::valid(f.get()));
}

// condition is set to false in first iteration
// for(0.0, true, 0.0 , body )
// body sets condition to false
void test_for_operation_true()
{
    pe::primitive init =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

  pe::primitive reinit =
      hpx::new_<pe::primitives::variable>(
          hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    pe::primitive cond =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), true);
    pe::primitive body =
        hpx::new_<pe::primitives::store_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                cond, false
            });

    pe::primitive for_ =
        hpx::new_<pe::primitives::for_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(reinit),
                std::move(body)
            });

    hpx::future<pe::primitive_argument_type> f =
        for_.eval();

    HPX_TEST(!pe::extract_boolean_value(f.get()));
}

// init is stated at zero
// for(init, init< 42,init=init+1; body)
// body: track the value of init just so that we can verify
// what its value was when the loop ended
// body(
//      set temp = init
//     )
void test_for_operation_42()
{
    // initial condition init is set to 0.0
    pe::primitive init =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    pe::primitive forty_two =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    pe::primitive one =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{1.0});

    // check if init is less than to 42 (this is true)
    pe::primitive cond =
        hpx::new_<pe::primitives::less>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                init, forty_two
            });

    // set temp=0.0 in the beginning
    pe::primitive temp =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    // do something in the body
    // here: set temp = init
    pe::primitive body =
      hpx::new_<pe::primitives::store_operation>(
          hpx::find_here(),
          std::vector<pe::primitive_argument_type>{
              temp, init
          });

    // in the reinit statement, add 1 to init
    pe::primitive reinit =
        hpx::new_<pe::primitives::store_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                init,
                pe::primitive_argument_type{
                    hpx::new_<pe::primitives::add_operation>(
                        hpx::find_here(),
                        std::vector<pe::primitive_argument_type>{
                            init, one
                        }
                    )
                }
            });

    // evaluate the for loop until condition is false
    pe::primitive for_ =
        hpx::new_<pe::primitives::for_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(reinit) ,
                std::move(body)
            });

    // when the loop ends the value obtained as the result
    // should be 41.0 as the condition will fail when
    // init is equal to 42.0
    hpx::future<pe::primitive_argument_type> f =
        for_.eval();

    // store() returns nil
    HPX_TEST(!pe::valid(f.get()));
    HPX_TEST_EQ(41.0, pe::numeric_operand_sync(temp, {})[0]);
}

// for(init=3.0; init<42; init=init+5; body
// body: track the value of init just so that we can verify
// what its value was when the loop ended
// body(
//      set temp = init
//     )
void test_for_operation_42_with_store()
{
    //set init to zero
    pe::primitive init =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    pe::primitive rhs =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    // store 3 to init
    pe::primitive store =
        hpx::new_<pe::primitives::store_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                init, std::move(rhs)
            });

    pe::primitive forty_two =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    pe::primitive five =
        hpx::new_<pe::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{5.0});

    // check if init is less than 42 (this is true)
    pe::primitive cond =
        hpx::new_<pe::primitives::less>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                pe::primitive_argument_type{init},
                pe::primitive_argument_type{forty_two}
            });

    // in the reinit statement add five to init
    pe::primitive reinit =
        hpx::new_<pe::primitives::store_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                init,
                pe::primitive_argument_type{
                    hpx::new_<pe::primitives::add_operation>(
                        hpx::find_here(),
                          std::vector<pe::primitive_argument_type>{
                              init, five
                          }
                    )
                }
            });

    // set temp=0.0 in the beginning
    pe::primitive temp =
      hpx::new_<pe::primitives::variable>(
          hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    // do something in the body
    // here: set temp = init
    pe::primitive body =
      hpx::new_<pe::primitives::store_operation>(
          hpx::find_here(),
          std::vector<pe::primitive_argument_type>{
              temp, init
          });

    // evaluate the for loop
    pe::primitive for_ =
        hpx::new_<pe::primitives::for_operation>(
            hpx::find_here(),
            std::vector<pe::primitive_argument_type>{
                std::move(store), std::move(cond), std::move(reinit),
                std::move(body)
            });

    // when the loop ends the value should be 38.0 as the condition will fail
    // init is greater than 42.0
    hpx::future<pe::primitive_argument_type> f =
        for_.eval();

    // store() returns nil
    HPX_TEST(!pe::valid(f.get()));
    HPX_TEST_EQ(38.0, pe::numeric_operand_sync(temp, {})[0]);
}

int main(int argc, char* argv[])
{
    test_for_operation_false();
    test_for_operation_true();
    test_for_operation_42();
    test_for_operation_42_with_store();

    return hpx::util::report_errors();
}


