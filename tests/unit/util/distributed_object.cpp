// Copyright (c) 2019 Weile Wei
// Copyright (c) 2019 Maxwell Reeser
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////
/// This is a distributed_object example using HPX component.
///
/// A distributed object is a single logical object partitioned across
/// a set of localities. (A locality is a single node in a cluster or a
/// NUMA domian in a SMP machine.) Each locality constructs an instance of
/// distributed_object<T>, where a value of type T represents the value of this
/// this locality's instance value. Once distributed_object<T> is conctructed, it
/// has a universal name which can be used on any locality in the given
/// localities to locate the resident instance.

#include <phylanx/config.hpp>
#include <phylanx/include/util.hpp>

#include <hpx/barrier.hpp>
#include <hpx/future.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/parallel/algorithms/for_each.hpp>

#include <boost/range/irange.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

REGISTER_DISTRIBUTED_OBJECT(int);

using vector_int = std::vector<int>;
REGISTER_DISTRIBUTED_OBJECT(vector_int);
using matrix_int = std::vector<std::vector<int>>;
REGISTER_DISTRIBUTED_OBJECT(matrix_int);

REGISTER_DISTRIBUTED_OBJECT(double);
using vector_double = std::vector<double>;
REGISTER_DISTRIBUTED_OBJECT(vector_double);
using matrix_double = std::vector<std::vector<double>>;
REGISTER_DISTRIBUTED_OBJECT(matrix_double);
using vector_double_const = std::vector<double> const;
REGISTER_DISTRIBUTED_OBJECT(vector_double_const);

using int_ref = int&;
REGISTER_DISTRIBUTED_OBJECT(int_ref);
using vector_int_ref = std::vector<int>&;
REGISTER_DISTRIBUTED_OBJECT(vector_int_ref);
using vector_double_const_ref = std::vector<double> const&;
REGISTER_DISTRIBUTED_OBJECT(vector_double_const_ref);

// addition/sum reduced to locality  for distributed_object
void test_distributed_object_int_reduce_to_locality_0()
{
    using phylanx::util::distributed_object;
    size_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    size_t cur_locality = hpx::get_locality_id();

    // Construct a distributed object of type int in all provided localities
    // User needs to provide the distributed object with a unique basename
    // and data for construction. The unique basename string enables HPX
    // register and retrieve the distributed object.
    distributed_object<int> dist_int(
        "test_distributed_object_int_reduce_to_locality_0", cur_locality,
        num_localities, cur_locality);

    // If there exists more than 2 (and include) localities, we are able to
    // asynchronously fetch a future of a copy of the instance of this
    // distributed object associated with the given locality
    if (hpx::get_locality_id() >= 2)
    {
        HPX_TEST_EQ(dist_int.fetch(1).get(), 1);
    }

    if (cur_locality == 0 && num_localities >= 2)
    {
        using hpx::parallel::for_each;
        using hpx::parallel::execution::par;
        auto range = boost::irange(std::size_t(1), num_localities);

        // compute expect result in parallel locality 0 fetches all values
        int expect_res = 0;
        for_each(par, std::begin(range), std::end(range),
            [&](std::uint64_t b) { expect_res += dist_int.fetch(b).get(); });
        hpx::wait_all();

        // compute target result
        // to verify the accumulation results
        int target_res = 0;
        for (int i = 0; i < num_localities; i++)
        {
            target_res += i;
        }

        HPX_TEST_EQ(expect_res, target_res);
    }

    hpx::lcos::barrier wait_for_operation(
        "barrier_test_distributed_object_int_reduce_to_locality_0",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    wait_for_operation.wait();
}

// element-wise addition for vector<int> for distributed_object
void test_distributed_object_vector_elem_wise_add()
{
    using phylanx::util::distributed_object;
    size_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    size_t cur_locality = hpx::get_locality_id();

    // define vector based on the locality that it is running
    int here_ = 42 + static_cast<int>(hpx::get_locality_id());
    int len = 10;

    // data for distributed object
    std::vector<int> local(len, here_);

    // construct a distributed_object with vector<int> type
    distributed_object<std::vector<int>> local_do(
        "test_distributed_object_vector_elem_wise_add", local);

    // testing -> operator
    HPX_TEST_EQ(local_do->size(), static_cast<size_t>(len));

    // testing dist_object and its vector underneath
    // testing * operator
    HPX_TEST((*local_do) == local);

    // perform element-wise addition between distributed_objects
    for (int i = 0; i < len; i++)
    {
        (*local_do)[i] += 1;
    }

    if (cur_locality == 0 && num_localities >= 2)
    {
        using hpx::parallel::for_each;
        using hpx::parallel::execution::par;
        auto range = boost::irange(std::size_t(1), num_localities);

        std::vector<std::vector<int>> res(num_localities);

        // compute expect result in parallel, locality 0 fetches all values
        for_each(par, std::begin(range), std::end(range), [&](std::uint64_t b) {
            res[b] = local_do.fetch(b).get();
            for (int i = 0; i < len; i++)
            {
                HPX_TEST_EQ(res[b][i], static_cast<int>((*local_do)[i] + b));
            }
        });
    }

    hpx::lcos::barrier wait_for_operation(
        "barrier_test_distributed_object_vector_elem_wise_add",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    wait_for_operation.wait();
}

// element-wise addition for vector<vector<double>> for distributed_object
void test_distributed_object_matrix()
{
    using phylanx::util::distributed_object;
    double val = 42.0 + static_cast<double>(hpx::get_locality_id());
    int rows = 5, cols = 5;

    matrix_double lhs_data(rows, std::vector<double>(cols, val));
    matrix_double rhs_data(rows, std::vector<double>(cols, val));
    matrix_double res_data(rows, std::vector<double>(cols, 0));

    distributed_object<matrix_double> lhs(
        "test_distributed_object_matrix_m1", lhs_data);
    distributed_object<matrix_double> rhs(
        "test_distributed_object_matrix_m2", rhs_data);
    distributed_object<matrix_double> res(
        "test_distributed_object_matrix_m3", res_data);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            (*res)[i][j] = (*lhs)[i][j] + (*rhs)[i][j];
            res_data[i][j] = lhs_data[i][j] + rhs_data[i][j];
        }
    }

    HPX_TEST((*res) == res_data );
    HPX_TEST_EQ(res->size(), static_cast<size_t>(rows));

    // test fetch function when 2 or more localities provided
    if (hpx::get_num_localities(hpx::launch::sync) > 1)
    {
        if (hpx::get_locality_id() == 0)
        {
            hpx::future<matrix_double> res_first = res.fetch(1);
            HPX_TEST_EQ(res_first.get()[0][0], 86);
        }
        else
        {
            hpx::future<matrix_double> res_first = res.fetch(0);
            HPX_TEST_EQ(res_first.get()[0][0], 84);
        }
    }

    hpx::lcos::barrier b_dist_matrix("barrier_test_distributed_object_matrix",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    b_dist_matrix.wait();
}

// test constructor in all_to_all option
void test_distributed_object_matrix_all_to_all()
{
    using phylanx::util::distributed_object;
    double val = 42.0 + static_cast<double>(hpx::get_locality_id());
    int rows = 5, cols = 5;

    matrix_double lhs_data(rows, std::vector<double>(cols, val));
    matrix_double rhs_data(rows, std::vector<double>(cols, val));
    matrix_double res_data(rows, std::vector<double>(cols, 0));

    using c_t = phylanx::util::construction_type;

    distributed_object<matrix_double, c_t::all_to_all> lhs(
        "test_distributed_object_matrix_all_to_all_m1", lhs_data);
    distributed_object<matrix_double, c_t::all_to_all> rhs(
        "test_distributed_object_matrix_all_to_all_m2", rhs_data);
    distributed_object<matrix_double, c_t::all_to_all> res(
        "test_distributed_object_matrix_all_to_all_m3", res_data);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            (*res)[i][j] = (*lhs)[i][j] + (*rhs)[i][j];
            res_data[i][j] = lhs_data[i][j] + rhs_data[i][j];
        }
    }
    HPX_TEST((*res) == res_data);
    HPX_TEST_EQ(res->size(), static_cast<size_t>(rows));

    // test fetch function when 2 or more localities provided
    if (hpx::get_num_localities(hpx::launch::sync) > 1)
    {
        if (hpx::get_locality_id() == 0)
        {
            hpx::future<matrix_double> res_first = res.fetch(1);
            HPX_TEST_EQ(res_first.get()[0][0], 86);
        }
        else
        {
            hpx::future<matrix_double> res_first = res.fetch(0);
            HPX_TEST_EQ(res_first.get()[0][0], 84);
        }
    }

    hpx::lcos::barrier b_dist_matrix_2(
        "barrier_test_distributed_object_matrix_all_to_all",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    b_dist_matrix_2.wait();
}

// test constructor option with reference to an existing object
void test_distributed_object_ref()
{
    using phylanx::util::distributed_object;
    size_t n = 10;
    int val = 2;

    int val_update = 42;
    vector_int vec1(n, val);
    distributed_object<vector_int&> dist_vec(
        "test_distributed_object_ref", vec1);

    // The update/change to the existing/referring object
    // will reflect the change to the distributed object
    vec1[2] = val_update;

    HPX_TEST_EQ((*dist_vec)[2], val_update);
    HPX_TEST_EQ(dist_vec->size(), static_cast<size_t>(n));

    hpx::lcos::barrier b(
        "barrier_test_distributed_object_ref",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    b.wait();
}

// test constructor option with reference to a const existing object
void test_distributed_object_const_ref()
{
    using phylanx::util::distributed_object;
    int n = 10;
    double val = 42.0;

    vector_double_const vec1(n, val);
    distributed_object<vector_double_const_ref> dist_vec(
        "test_distributed_object_const_ref", vec1);

    hpx::lcos::barrier b(
        "barrier_test_distributed_object_const_ref",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    b.wait();
}

// simple matrix multiplication example
void test_distributed_object_matrix_mul()
{
    using phylanx::util::distributed_object;
    size_t cols = 5;    // Decide how big the matrix should be

    size_t num_locs = hpx::get_num_localities(hpx::launch::sync);
    std::vector<std::pair<size_t, size_t>> ranges(num_locs);

    // Create a list of row ranges for each partition
    size_t start = 0;
    size_t diff = (int) std::ceil((double) cols / ((double) num_locs));
    for (size_t i = 0; i < num_locs; i++)
    {
        size_t second = (std::min)(cols, start + diff);
        ranges[i] = std::make_pair(start, second);
        start += diff;
    }

    // Create our data, stored in all_data's. This way we can check for validity
    // without using anything distributed. The seed being a constant is needed
    // in order for all nodes to generate the same data
    size_t here = hpx::get_locality_id();
    size_t local_rows = ranges[here].second - ranges[here].first;
    std::vector<std::vector<std::vector<int>>> all_data_m1(
        hpx::get_num_localities(hpx::launch::sync));

    std::srand(123456);

    for (size_t i = 0; i < all_data_m1.size(); i++)
    {
        size_t tmp_num_rows = ranges[i].second - ranges[i].first;
        all_data_m1[i] = std::vector<std::vector<int>>(
            tmp_num_rows, std::vector<int>(cols, 0));
        for (size_t j = 0; j < tmp_num_rows; j++)
        {
            for (size_t k = 0; k < cols; k++)
            {
                all_data_m1[i][j][k] = std::rand();
            }
        }
    }

    std::vector<std::vector<std::vector<int>>> all_data_m2(
        hpx::get_num_localities(hpx::launch::sync));

    std::srand(7891011);

    for (size_t i = 0; i < all_data_m2.size(); i++)
    {
        size_t tmp_num_rows = ranges[i].second - ranges[i].first;
        all_data_m2[i] = std::vector<std::vector<int>>(
            tmp_num_rows, std::vector<int>(cols, 0));
        for (size_t j = 0; j < tmp_num_rows; j++)
        {
            for (size_t k = 0; k < cols; k++)
            {
                all_data_m2[i][j][k] = std::rand();
            }
        }
    }

    std::vector<std::vector<int>> here_data_m3(
        local_rows, std::vector<int>(cols, 0));

    using c_t = phylanx::util::construction_type;

    distributed_object<matrix_int, c_t::all_to_all> M1(
        "M1_meta_mat_mul", all_data_m1[here]);
    distributed_object<matrix_int, c_t::all_to_all> M2(
        "M2_meta_mat_mul", all_data_m2[here]);
    distributed_object<matrix_int, c_t::all_to_all> M3(
        "M3_meta_mat_mul", here_data_m3);

    // Actual matrix multiplication. For non-local values, get the data
    // and then use it, for local, just use the local data without doing
    // a fetch to get it
    size_t num_before_me = here;
    for (size_t p = 0; p < num_before_me; p++)
    {
        std::vector<std::vector<int>> non_local = M2.fetch(p).get();
        for (size_t i = 0; i < local_rows; i++)
        {
            for (size_t j = ranges[p].first; j < ranges[p].second; j++)
            {
                for (size_t k = 0; k < cols; k++)
                {
                    (*M3)[i][j] +=
                        (*M1)[i][k] * non_local[j - ranges[p].first][k];
                    here_data_m3[i][j] += all_data_m1[here][i][k] *
                        all_data_m2[p][j - ranges[p].first][k];
                }
            }
        }
    }
    for (size_t i = 0; i < local_rows; i++)
    {
        for (size_t j = ranges[here].first; j < ranges[here].second; j++)
        {
            for (size_t k = 0; k < cols; k++)
            {
                (*M3)[i][j] += (*M1)[i][k] * (*M2)[j - ranges[here].first][k];
                here_data_m3[i][j] += all_data_m1[here][i][k] *
                    all_data_m2[here][j - ranges[here].first][k];
            }
        }
    }
    for (size_t p = here + 1; p < num_locs; p++)
    {
        std::vector<std::vector<int>> non_local = M2.fetch(p).get();
        for (size_t i = 0; i < local_rows; i++)
        {
            for (size_t j = ranges[p].first; j < ranges[p].second; j++)
            {
                for (size_t k = 0; k < cols; k++)
                {
                    (*M3)[i][j] +=
                        (*M1)[i][k] * non_local[j - ranges[p].first][k];
                    here_data_m3[i][j] += all_data_m1[here][i][k] *
                        all_data_m2[p][j - ranges[p].first][k];
                }
            }
        }
    }
    HPX_TEST((*M3) == here_data_m3);

    hpx::lcos::barrier b(
        "barrier_test_distributed_object_const_ref",
        hpx::get_num_localities(hpx::launch::sync),
        hpx::get_locality_id());
    b.wait();
}

void test_dist_object_vector_mo_sub_localities_constructor()
{
    using c_t = phylanx::util::construction_type;
    using phylanx::util::distributed_object;
    size_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    size_t cur_locality = static_cast<size_t>(hpx::get_locality_id());

    // define vector based on the locality that it is running
    int here_ = 42 + static_cast<int>(hpx::get_locality_id());
    int len = 10;

    // prepare vector data
    std::vector<int> local(len, here_);
    std::vector<size_t> sub_localities{0, 1};

    if (num_localities >= 2 &&
        std::find(sub_localities.begin(), sub_localities.end(),
            static_cast<size_t>(cur_locality)) != sub_localities.end())
    {
        // construct a distributed_object with vector<int> type
        distributed_object<std::vector<int>, c_t::all_to_all> local_do(
            "test_dist_object_vector_mo_sub_localities_constructor", local, 2);

        // testing -> operator
        HPX_TEST_EQ(local_do->size(), static_cast<size_t>(len));

        // testing dist_object and its vector underneath
        // testing * operator
        HPX_TEST((*local_do) == local);

        // perform element-wise addition between distributed_objects
        for (int i = 0; i < len; i++)
        {
            (*local_do)[i] += 1;
        }

        hpx::lcos::barrier b(
            "barrier_test_dist_object_vector_mo_sub_localities_constructor",
            sub_localities.size(), cur_locality);
        b.wait();

        std::sort(sub_localities.begin(), sub_localities.end());
        if (cur_locality == sub_localities[0])
        {
            using hpx::for_each;
            using hpx::execution::par;

            std::vector<std::vector<int>> res(num_localities);

            // compute expect result in parallel, locality 0 fetches all values
            for_each(par, std::begin(sub_localities) + 1, std::end(sub_localities),
                [&](std::uint64_t b) {
                    res[b] = local_do.fetch(b).get();
                    for (int i = 0; i < len; i++)
                    {
                        HPX_TEST_EQ(
                            res[b][i], static_cast<int>((*local_do)[i] + b));
                    }
                });
        }

        b.wait();
    }
}

void test_distributed_object_sub_localities_constructor()
{
    using phylanx::util::distributed_object;
    std::vector<int> input(10, 1);
    if (hpx::get_locality_id() == 0)
    {
        distributed_object<std::vector<int>> vec1(
            "test_distributed_object_sub_localities_constructor", input, 1, 0);
    }
}

int hpx_main()
{
    {
        test_distributed_object_int_reduce_to_locality_0();
        test_distributed_object_vector_elem_wise_add();
        test_distributed_object_matrix();
        test_distributed_object_matrix_all_to_all();
        test_distributed_object_matrix_mul();
        test_distributed_object_ref();
        test_distributed_object_const_ref();
        test_dist_object_vector_mo_sub_localities_constructor();
        test_distributed_object_sub_localities_constructor();
    }
    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    hpx::init_params params;
    params.cfg = std::move(cfg);
    return hpx::init(argc, argv, params);
}
