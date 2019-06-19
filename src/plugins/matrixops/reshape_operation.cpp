// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/reshape_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const reshape_operation::match_data = {
        match_pattern_type{"reshape",
            std::vector<std::string>{"reshape(_1, _2)"},
            &create_reshape_operation, &create_primitive<reshape_operation>,
            R"(
            a, newshape
            Args:

                a (array_like) : input array
                newshape (integer or tuple of integers): The new shape should be
                     compatible with the original shape (number of elements in
                     both arrays are the same). If an integer, then the result
                     will be a 1-D array of that length. The last parameter of
                     the newshape can be -1. In this case, the value is inferred
                     from the length of the array and remaining dimensions.

            Returns:

            Returns a new shape to an array without changing its data."
            )"},

        match_pattern_type{"flatten",
            std::vector<std::string>{"flatten(_1, _2)", "flatten(_1)"},
            &create_reshape_operation, &create_primitive<reshape_operation>,
            R"(
            a, order
            Args:

                a (array) : a scalar, vector, matrix or a tensor
                order (optional, char): 'C' means row-major(C-style),
                                        'F' means column-major(Fortran-style),
                                        The default is 'C'.

            Returns:

            A copy of the array collapsed into one dimension."
            )"}};

    ///////////////////////////////////////////////////////////////////////////
    reshape_operation::reshape_mode extract_reshape_mode(
        std::string const& name)
    {
        reshape_operation::reshape_mode result =
            reshape_operation::general_reshape;

        if (name.find("flatten") != std::string::npos)
        {
            result = reshape_operation::flatten_mode;
        }
        return result;
    }

    reshape_operation::reshape_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
        , mode_(extract_reshape_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool reshape_operation::validate_shape(
        std::size_t const& n, ir::range const& arg) const
    {
        if (arg.size() == 1)
        {
            std::int64_t first = extract_scalar_integer_value_strict(*arg.begin());
            if (first == -1)
                return true;

            return first == n;
        }
        else if (arg.size() == 2)
        {
            auto it = arg.begin();
            std::int64_t first = extract_scalar_integer_value_strict(*it);
            std::int64_t second = extract_scalar_integer_value_strict(*++it);
            if (first == -1 && second > 0)
            {
                if (n % second == 0)
                    return true;
            }
            else if (second == -1 && first > 0)
            {
                if (n % first == 0)
                    return true;
            }
            else if (first > 0 && second > 0)
            {
                return first * second == n;
            }
            return false;
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        else if (arg.size() == 3)
        {
            auto it = arg.begin();
            std::int64_t first = extract_scalar_integer_value_strict(*it);
            std::int64_t second = extract_scalar_integer_value_strict(*++it);
            std::int64_t third = extract_scalar_integer_value_strict(*++it);

            if (first == -1 && second > 0 && third > 0)
            {
                if (n % (second * third) == 0)
                    return true;
            }
            else if (second == -1 && first > 0 && third > 0)
            {
                if (n % (first * third) == 0)
                    return true;
            }
            else if (third == -1 && first > 0 && second > 0)
            {
                if (n % (first * second) == 0)
                    return true;
            }
            else if (first > 0 && second > 0 && third > 0)
            {
                return first * second * third == n;
            }
            return false;
        }
#endif
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::validate_shape",
                util::generate_error_message("The given shape has an invalid "
                    "number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape0d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch(arg.size())
        {
        case 1:
            return primitive_argument_type{
                blaze::DynamicVector<T>{arr.scalar()}};

        case 2:
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, 1, arr.scalar()}};

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return primitive_argument_type{
                blaze::DynamicTensor<T>{1, 1, 1, arr.scalar()}};
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "reshaping to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape0d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape0d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape0d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape0d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape0d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape0d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape1d_2d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto a = arr.vector();

        auto it = arg.begin();
        std::int64_t first = extract_scalar_integer_value(*it);
        std::int64_t second = extract_scalar_integer_value(*++it);

        if (first == -1)
        {
            first = a.size() / second;
        }
        else if (second == -1)
        {
            second = a.size() / first;
        }

        blaze::DynamicMatrix<T> result(first, second);

        phylanx::util::matrix_iterator<blaze::DynamicMatrix<T>> dest(result);
        std::copy(a.begin(), a.end(), dest);

        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type reshape_operation::reshape1d_3d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto a = arr.vector();

        auto it = arg.begin();
        std::int64_t pages = extract_scalar_integer_value(*it);
        std::int64_t rows = extract_scalar_integer_value(*++it);
        std::int64_t columns = extract_scalar_integer_value(*++it);

        if (rows == -1)
        {
            rows = (a.size()) / (columns * pages);
        }
        else if (columns == -1)
        {
            columns = (a.size()) / (rows * pages);
        }
        else if (pages == -1)
        {
            pages = (a.size()) / (rows * columns);
        }

        blaze::DynamicTensor<T> result(pages, rows, columns);

        phylanx::util::tensor_iterator<blaze::DynamicTensor<T>> dest(result);

        std::copy(a.begin(), a.end(), dest);

        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type reshape_operation::reshape1d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return primitive_argument_type{std::move(arr)};

        case 2:
            return reshape1d_2d(std::move(arr),std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return reshape1d_3d(std::move(arr),std::move(arg));
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::reshape1d",
                util::generate_error_message(
                    "reshaping to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape1d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape1d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape1d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape1d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape1d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape2d_1d(
        ir::node_data<T>&& arr) const
    {
        auto a = arr.matrix();

        using phylanx::util::matrix_iterator;
        matrix_iterator<decltype(a)> const a_begin(a);
        matrix_iterator<decltype(a)> const a_end(a, a.rows());

        blaze::DynamicVector<T> result(a.rows() * a.columns());

        std::copy(a_begin, a_end, result.data());

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape2d_2d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto a = arr.matrix();

        auto it = arg.begin();
        std::int64_t rows = extract_scalar_integer_value(*it);
        std::int64_t columns = extract_scalar_integer_value(*++it);

        if (rows == -1)
        {
            rows = (a.rows() * a.columns()) / columns;
        }
        else if (columns == -1)
        {
            columns = (a.rows() * a.columns()) / rows;
        }

        blaze::DynamicMatrix<T> result(rows, columns);

        using phylanx::util::matrix_iterator;

        matrix_iterator<decltype(a)> begin(a, 0);
        matrix_iterator<decltype(a)> end(a, a.rows());
        matrix_iterator<blaze::DynamicMatrix<T>> dest(result);

        std::copy(begin, end, dest);

        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type reshape_operation::reshape2d_3d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto a = arr.matrix();

        auto it = arg.begin();
        std::int64_t pages = extract_scalar_integer_value(*it);
        std::int64_t rows = extract_scalar_integer_value(*++it);
        std::int64_t columns = extract_scalar_integer_value(*++it);

        if (rows == -1)
        {
            rows = (a.rows() * a.columns()) / (columns * pages);
        }
        else if (columns == -1)
        {
            columns = (a.rows() * a.columns()) / (rows * pages);
        }
        else if (pages == -1)
        {
            pages = (a.rows() * a.columns()) / (rows * columns);
        }

        blaze::DynamicTensor<T> result(pages, rows, columns);

        auto src = blaze::ravel(a);
        phylanx::util::tensor_iterator<blaze::DynamicTensor<T>> dest(result);

        std::copy(src.begin(), src.end(), dest);

        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type reshape_operation::reshape2d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return reshape2d_1d(std::move(arr));

        case 2:
            return reshape2d_2d(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return reshape2d_3d(std::move(arr), std::move(arg));
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::reshape2d",
                util::generate_error_message(
                    "reshaping to >3d is not supported", name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape2d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape2d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape3d_1d(
        ir::node_data<T>&& arr) const
    {
        auto t = arr.tensor();

        blaze::DynamicVector<T> result(t.pages() * t.rows() * t.columns());

        auto src = blaze::ravel(t);
        std::copy(src.begin(), src.end(), result.begin());

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape3d_2d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto t = arr.tensor();

        auto it = arg.begin();
        std::int64_t rows = extract_scalar_integer_value(*it);
        std::int64_t columns = extract_scalar_integer_value(*++it);

        if (rows == -1)
        {
            rows = (t.pages() * t.rows() * t.columns()) / columns;
        }
        else if (columns == -1)
        {
            columns = (t.pages() * t.rows() * t.columns()) / rows;
        }

        blaze::DynamicMatrix<T> result(rows, columns);

        auto src = blaze::ravel(t);
        phylanx::util::matrix_iterator<blaze::DynamicMatrix<T>> dest(result);

        std::copy(src.begin(), src.end(), dest);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape3d_3d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto t = arr.tensor();

        auto it = arg.begin();
        std::int64_t pages = extract_scalar_integer_value(*it);
        std::int64_t rows = extract_scalar_integer_value(*++it);
        std::int64_t columns = extract_scalar_integer_value(*++it);

        if (rows == -1)
        {
            rows = (t.pages() * t.rows() * t.columns()) / (columns * pages);
        }
        else if (columns == -1)
        {
            columns = (t.pages() * t.rows() * t.columns()) / (rows * pages);
        }
        else if (pages == -1)
        {
            pages = (t.pages() * t.rows() * t.columns()) / (rows * columns);
        }

        blaze::DynamicTensor<T> result(pages, rows, columns);

        auto src = blaze::ravel(t);
        phylanx::util::tensor_iterator<blaze::DynamicTensor<T>> dest(result);

        std::copy(src.begin(), src.end(), dest);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape3d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return reshape3d_1d(std::move(arr));

        case 2:
            return reshape3d_2d(std::move(arr), std::move(arg));

        case 3:
            return reshape3d_3d(std::move(arr), std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::reshape3d",
                util::generate_error_message(
                    "reshaping to >3d is not supported", name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape3d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape3d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape3d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape3d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape3d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape3d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::flatten2d(
        ir::node_data<T>&& arr, std::string order) const
    {
        if (order == "F")
        {
            using phylanx::util::matrix_column_iterator;

            auto a = arr.matrix();
            blaze::DynamicVector<T> result(a.rows() * a.columns());
            auto d = result.data();

            const matrix_column_iterator<decltype(a)> a_begin(a);
            const matrix_column_iterator<decltype(a)> a_end(a, a.columns());

            for (auto it = a_begin; it != a_end; ++it)
            {
                d = std::copy(it->begin(), it->end(), d);
            }
            return primitive_argument_type{std::move(result)};
        }
        // order is 'C'
        return reshape2d_1d(std::move(arr));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type reshape_operation::flatten3d(
        ir::node_data<T>&& arr, std::string order) const
    {
        if (order == "F")
        {
            using phylanx::util::matrix_column_iterator;

            auto t = arr.tensor();
            blaze::DynamicVector<T> result(t.pages() * t.rows() * t.columns());
            auto d = result.data();

            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> const a_begin(slice);
                matrix_column_iterator<decltype(slice)> const a_end(
                    slice, slice.columns());

                for (auto it = a_begin; it != a_end; ++it)
                    d = std::copy(it->begin(), it->end(), d);
            }
            return primitive_argument_type{std::move(result)};
        }
        // order is 'C'
        return reshape3d_1d(std::move(arr));
    }
#endif

    template <typename T>
    primitive_argument_type reshape_operation::flatten_nd(
        ir::node_data<T>&& arr) const
    {
        switch (extract_numeric_value_dimension(arr))
        {
        case 0:
            return primitive_argument_type{
                blaze::DynamicVector<T>{arr.scalar()}};

        case 1:
            return primitive_argument_type{std::move(arr)};

        case 2:
            return reshape2d_1d(std::move(arr));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return reshape3d_1d(std::move(arr));
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "reshape_operation::flatten_nd",
            util::generate_error_message(
                "the array has unsupported number of dimensions", name_,
                codename_));
    }

    template <typename T>
    primitive_argument_type reshape_operation::flatten_nd(
        ir::node_data<T>&& arr, std::string order) const
    {
        switch (extract_numeric_value_dimension(arr))
        {
        case 0:
            return primitive_argument_type{
                blaze::DynamicVector<T>{arr.scalar()}};

        case 1:
            return primitive_argument_type{std::move(arr)};

        case 2:
            return flatten2d(std::move(arr), std::move(order));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return flatten3d(std::move(arr), std::move(order));
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "reshape_operation::flatten_nd",
            util::generate_error_message(
                "the array has unsupported number of dimensions", name_,
                codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> reshape_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "the reshape/flatten operation primitive requires one or "
                    "two operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    util::generate_error_message(
                        "the reshape_operation primitive requires that the "
                        "arguments given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 1)
        {
            if (mode_ == general_reshape)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    this_->generate_error_message(
                        "the reshape requires exactly two operands"));
            }

            if (mode_ == flatten_mode)
            {
                return hpx::dataflow(hpx::launch::sync,
                    [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f)
                    -> primitive_argument_type
                    {
                        auto&& arr = f.get();

                        switch (extract_common_type(arr))
                        {
                        case node_data_type_bool:
                            return this_->flatten_nd(
                                extract_boolean_value_strict(std::move(arr),
                                    this_->name_, this_->codename_));

                        case node_data_type_int64:
                            return this_->flatten_nd(
                                extract_integer_value_strict(std::move(arr),
                                    this_->name_, this_->codename_));

                        case node_data_type_double:
                            return this_->flatten_nd(
                                extract_numeric_value_strict(std::move(arr),
                                    this_->name_, this_->codename_));

                        case node_data_type_unknown:
                            return this_->flatten_nd(
                                extract_numeric_value(std::move(arr),
                                    this_->name_, this_->codename_));

                        default:
                            break;
                        }

                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            this_->generate_error_message(
                                "the reshape/flatten primitive requires for "
                                "all arguments to be numeric data types"));
                    },
                    value_operand(operands[0], args, name_, codename_, std::move(ctx)));
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                generate_error_message(
                    "unsupported reshape mode requested"));
        }

        if (mode_ == general_reshape)
        {
            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f1,
                        hpx::future<ir::range>&& f2)
                ->primitive_argument_type
                {
                    auto&& arr = f1.get();
                    auto&& arg = f2.get();

                    std::size_t arr_dims_num = extract_numeric_value_dimension(
                        arr, this_->name_, this_->codename_);
                    std::size_t size = extract_numeric_value_size(
                        arr, this_->name_, this_->codename_);

                    if (arr_dims_num > PHYLANX_MAX_DIMENSIONS)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of dimensions"));
                    }

                    if (!this_->validate_shape(size, arg))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            this_->generate_error_message(
                                "The given shape is not compatible with the shape "
                                "of the original array. Notice that you can only "
                                "specify one unknown dimension"));
                    }

                    switch (arr_dims_num)
                    {
                    case 0:
                        return this_->reshape0d(std::move(arr), std::move(arg));

                    case 1:
                        return this_->reshape1d(std::move(arr), std::move(arg));

                    case 2:
                        return this_->reshape2d(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->reshape3d(std::move(arr), std::move(arg));
#endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of dimensions"));
                    }
                },
                std::move(op0),
                list_operand(operands[1], args, name_, codename_, std::move(ctx)));
        }

        if (mode_ == flatten_mode)
        {
            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f1,
                        hpx::future<std::string>&& f2)
                -> primitive_argument_type
                {
                    auto&& arr = f1.get();
                    auto&& order = f2.get();

                    if (order != "C" && order != "F")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            this_->generate_error_message(
                                "order not understood. the order parameter could "
                                " only be 'C' or 'F'"));
                    }

                    switch (extract_common_type(arr))
                    {
                    case node_data_type_bool:
                        return this_->flatten_nd(
                            extract_boolean_value_strict(
                                std::move(arr), this_->name_, this_->codename_),
                            std::move(order));

                    case node_data_type_int64:
                        return this_->flatten_nd(
                            extract_integer_value_strict(
                                std::move(arr), this_->name_, this_->codename_),
                            std::move(order));

                    case node_data_type_double:
                        return this_->flatten_nd(
                            extract_numeric_value_strict(
                                std::move(arr), this_->name_, this_->codename_),
                            std::move(order));

                    case node_data_type_unknown:
                        return this_->flatten_nd(
                            extract_numeric_value(
                                std::move(arr), this_->name_, this_->codename_),
                            std::move(order));

                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "reshape_operation::eval",
                        this_->generate_error_message(
                            "the reshape/flatten primitive requires for "
                            "all arguments to be numeric data types"));

                },
                std::move(op0),
                string_operand(operands[1], args, name_, codename_, std::move(ctx)));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "reshape_operation::eval",
            generate_error_message("unsupported reshape mode requested"));
    }
}}}
