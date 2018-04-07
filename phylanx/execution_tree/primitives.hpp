//  Copyright (c) 2017-2018 Hartmut Kaiser
//                2018 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PRIMITIVES_HPP)
#define PHYLANX_PRIMITIVES_PRIMITIVES_HPP

#include <phylanx/execution_tree/primitives/access_argument.hpp>
#include <phylanx/execution_tree/primitives/add_dimension.hpp>
#include <phylanx/execution_tree/primitives/all_operation.hpp>
#include <phylanx/execution_tree/primitives/any_operation.hpp>
#include <phylanx/execution_tree/primitives/argmax.hpp>
#include <phylanx/execution_tree/primitives/argmin.hpp>
#include <phylanx/execution_tree/primitives/car_cdr_operation.hpp>
#include <phylanx/execution_tree/primitives/column_set.hpp>
#include <phylanx/execution_tree/primitives/column_slicing.hpp>
#include <phylanx/execution_tree/primitives/console_output.hpp>
#include <phylanx/execution_tree/primitives/constant.hpp>
#include <phylanx/execution_tree/primitives/cross_operation.hpp>
#include <phylanx/execution_tree/primitives/debug_output.hpp>
#include <phylanx/execution_tree/primitives/define_function.hpp>
#include <phylanx/execution_tree/primitives/define_variable.hpp>
#include <phylanx/execution_tree/primitives/determinant.hpp>
#include <phylanx/execution_tree/primitives/diag_operation.hpp>
#include <phylanx/execution_tree/primitives/dot_operation.hpp>
#include <phylanx/execution_tree/primitives/enable_tracing.hpp>
#include <phylanx/execution_tree/primitives/exponential_operation.hpp>
#include <phylanx/execution_tree/primitives/extract_shape.hpp>
#include <phylanx/execution_tree/primitives/file_read.hpp>
#include <phylanx/execution_tree/primitives/file_read_csv.hpp>
#include <phylanx/execution_tree/primitives/file_read_hdf5.hpp>
#include <phylanx/execution_tree/primitives/file_write.hpp>
#include <phylanx/execution_tree/primitives/file_write_csv.hpp>
#include <phylanx/execution_tree/primitives/file_write_hdf5.hpp>
#include <phylanx/execution_tree/primitives/function_reference.hpp>
#include <phylanx/execution_tree/primitives/gradient_operation.hpp>
#include <phylanx/execution_tree/primitives/hstack_operation.hpp>
#include <phylanx/execution_tree/primitives/identity.hpp>
#include <phylanx/execution_tree/primitives/inverse_operation.hpp>
#include <phylanx/execution_tree/primitives/linearmatrix.hpp>
#include <phylanx/execution_tree/primitives/linspace.hpp>
#include <phylanx/execution_tree/primitives/make_list.hpp>
#include <phylanx/execution_tree/primitives/mean_operation.hpp>
#include <phylanx/execution_tree/primitives/power_operation.hpp>
#include <phylanx/execution_tree/primitives/random.hpp>
#include <phylanx/execution_tree/primitives/range_operation.hpp>
#include <phylanx/execution_tree/primitives/row_set.hpp>
#include <phylanx/execution_tree/primitives/row_slicing.hpp>
#include <phylanx/execution_tree/primitives/set_operation.hpp>
#include <phylanx/execution_tree/primitives/shuffle_operation.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>
#include <phylanx/execution_tree/primitives/square_root_operation.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/execution_tree/primitives/string_output.hpp>
#include <phylanx/execution_tree/primitives/sum_operation.hpp>
#include <phylanx/execution_tree/primitives/transpose_operation.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/execution_tree/primitives/vstack_operation.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>
#include <phylanx/execution_tree/primitives/wrapped_variable.hpp>

#endif
