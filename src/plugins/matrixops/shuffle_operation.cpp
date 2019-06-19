// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/matrixops/shuffle_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/random.hpp>
#include <phylanx/util/detail/bad_swap.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>


///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const shuffle_operation::match_data =
    {
        hpx::util::make_tuple("shuffle",
            std::vector<std::string>{"shuffle(_1)"},
            &create_shuffle_operation,
            &create_primitive<shuffle_operation>, R"(
            args
            Args:

                args (list) : a list of values

            Returns:

            A shuffled version of the list. Note that `args` itself will be "
            shuffled by this call.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    shuffle_operation::shuffle_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type shuffle_operation::shuffle_1d(
        ir::node_data<T>&& arg) const
    {
        auto x = arg.vector();
        std::shuffle(x.begin(), x.end(), util::rng_);

        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type shuffle_operation::shuffle_2d(
        ir::node_data<T>&& arg) const
    {
        auto x = arg.matrix();

        util::matrix_row_iterator<decltype(x)> x_begin(x);
        util::matrix_row_iterator<decltype(x)> x_end(x, x.rows());
        std::shuffle(x_begin, x_end, util::rng_);

        return primitive_argument_type{std::move(arg)};
    }

    primitive_argument_type shuffle_operation::shuffle_1d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return shuffle_1d(extract_boolean_value_strict(std::move(arg)));

        case node_data_type_int64:
            return shuffle_1d(extract_integer_value_strict(std::move(arg)));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return shuffle_1d(extract_numeric_value(std::move(arg)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "shuffle_operation::shuffle_1d",
            generate_error_message(
                "the shuffle primitive requires for its argument to "
                    "be numeric data type"));
    }

    primitive_argument_type shuffle_operation::shuffle_2d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return shuffle_2d(extract_boolean_value_strict(std::move(arg)));

        case node_data_type_int64:
            return shuffle_2d(extract_integer_value_strict(std::move(arg)));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return shuffle_2d(extract_numeric_value(std::move(arg)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "shuffle_operation::shuffle_2d",
            generate_error_message(
                "the shuffle primitive requires for its argument to "
                    "be numeric data type"));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> shuffle_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "shuffle_operation::eval",
                generate_error_message(
                    "the shuffle_operation primitive requires "
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "shuffle_operation::eval",
                generate_error_message(
                    "the shuffle_operation primitive requires that "
                        "the argument is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f)
            -> primitive_argument_type
            {
                auto&& arg = f.get();

                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 1:
                    return this_->shuffle_1d(std::move(arg));

                case 2:
                    return this_->shuffle_2d(std::move(arg));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "shuffle_operation::eval",
                        this_->generate_error_message(
                            "operand has an unsupported number of "
                                "dimensions. Only possible values are: "
                                "1 or 2."));
                }
            },
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
