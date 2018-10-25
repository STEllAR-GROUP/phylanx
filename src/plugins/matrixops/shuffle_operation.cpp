// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/matrixops/random_utils.hpp>
#include <phylanx/plugins/matrixops/shuffle_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

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

//////////////////////////////////////////////////////////////////////////
namespace blaze
{
    namespace _row_swap
    {
        using std::swap;

        template <typename T>
        void check_swap() noexcept(
            noexcept(swap(std::declval<typename T::BaseType&>(),
                std::declval<typename T::BaseType&>())))
        {}
    }

    // BADBAD: This overload of swap is necessary to work around the problems
    //         caused by matrix_row_iterator not being a real random access
    //         iterator. Dereferencing matrix_row_iterator does not yield a
    //         true reference but only a temporary blaze::Row holding true
    //         references.
    //
    // A real fix for this problem is proposed in PR0022R0
    // (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r0.html)
    //
    using std::iter_swap;

    template <typename T>
    void swap(Row<T>&& x, Row<T>&& y) noexcept(
        noexcept(_row_swap::check_swap<T>()) &&
        noexcept(*std::declval<typename T::Iterator>()))
    {
        for (auto a = x.begin(), b = y.begin(); a != x.end() && b != y.end();
            ++a, ++b)
        {
            iter_swap(a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const shuffle_operation::match_data =
    {
        hpx::util::make_tuple("shuffle",
            std::vector<std::string>{"shuffle(_1)"},
            &create_shuffle_operation, &create_primitive<shuffle_operation>,
            "args\n"
            "Args:\n"
            "\n"
            "    args (list) : a list of values\n"
            "\n"
            "Returns:\n"
            "\n"
            "A shuffled version of the list. Note that `args` itself will be "
            "shuffled by this call."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    shuffle_operation::shuffle_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type shuffle_operation::shuffle_1d(arg_type && arg) const
    {
        auto x = arg.vector();
        std::shuffle(x.begin(), x.end(), util::rng_);

        return primitive_argument_type{ir::node_data<double>{std::move(x)}};
    }

    primitive_argument_type shuffle_operation::shuffle_2d(arg_type&& arg) const
    {
        auto x = arg.matrix();
        auto x_begin = util::matrix_row_iterator<decltype(x)>(x);
        auto x_end = util::matrix_row_iterator<decltype(x)>(x, x.rows());
        std::shuffle(x_begin, x_end, util::rng_);

        return primitive_argument_type{ir::node_data<double>{std::move(x)}};
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> shuffle_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "shuffle_operation::eval",
                util::generate_error_message(
                    "the shuffle_operation primitive requires "
                        "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "shuffle_operation::eval",
                util::generate_error_message(
                    "the shuffle_operation primitive requires that "
                        "the argument is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](arg_type&& arg)
                -> primitive_argument_type
                {
                    std::size_t lhs_dims = arg.num_dimensions();
                    switch (lhs_dims)
                    {
                    case 1:
                        return this_->shuffle_1d(std::move(arg));

                    case 2:
                        return this_->shuffle_2d(std::move(arg));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "shuffle_operation::eval",
                            util::generate_error_message(
                                "operand has an unsupported number of "
                                    "dimensions. Only possible values are: "
                                    "1 or 2.",
                                this_->name_, this_->codename_));
                    }
                }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    hpx::future<primitive_argument_type> shuffle_operation::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
