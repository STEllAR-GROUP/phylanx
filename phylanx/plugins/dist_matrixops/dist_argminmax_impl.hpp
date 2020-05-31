// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_dist_argminmax_IMPL_2020_MAY_21_0503PM)
#define PHYLANX_PRIMITIVES_DIST_dist_argminmax_IMPL_2020_MAY_21_0503PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/common/argminmax_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/assertion.hpp>
#include <hpx/collectives.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
using std_pair_double_size_t = std::pair<double, std::size_t>;
using std_pair_int64_t_size_t = std::pair<std::int64_t, std::size_t>;
using std_pair_uint8_t_size_t = std::pair<std::uint8_t, std::size_t>;

HPX_REGISTER_ALLREDUCE_DECLARATION(std_pair_double_size_t);
HPX_REGISTER_ALLREDUCE_DECLARATION(std_pair_int64_t_size_t);
HPX_REGISTER_ALLREDUCE_DECLARATION(std_pair_uint8_t_size_t);

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    dist_argminmax<Op, Derived>::dist_argminmax(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax0d(
        execution_tree::primitive_arguments_type&& args) const
    {
        // 0d is always local
        return common::argminmax0d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename Operation>
        struct all_reduce_op_1d
        {
            // the first element in the pairs is the (scalar) value, the second
            // element is the (global) index of that value
            template <typename T>
            std::pair<T, std::size_t> operator()(
                std::pair<T, std::size_t> const& result,
                std::pair<T, std::size_t> const& current) const
            {
                if (Operation::index_compare(result, current))
                {
                    return result;
                }
                return current;
            }
        };

        template <typename Op, typename T>
        execution_tree::primitive_argument_type argminmax1d_reduce(T value,
            std::size_t index,
            execution_tree::localities_information const& locs)
        {
            auto p = hpx::all_reduce(
                ("all_reduce_" + locs.annotation_.name_).c_str(),
                std::make_pair(value, index), all_reduce_op_1d<Op>{},
                locs.locality_.num_localities_, std::size_t(-1),
                locs.locality_.locality_id_)
                         .get();

            return execution_tree::primitive_argument_type{
                static_cast<std::int64_t>(p.second)};
        }
    }    // namespace detail

    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax1d(
        execution_tree::primitive_arguments_type&& args) const
    {
        using namespace execution_tree;
        if (!args[0].has_annotation())
        {
            return common::argminmax1d<Op>(std::move(args), name_, codename_);
        }

        localities_information locs =
            extract_localities_information(args[0], name_, codename_);

        std::size_t ndim = locs.num_dimensions();
        if (ndim > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_argminmax<Op, Derived>::argminmax1d",
                generate_error_message(
                    "the operand has incompatible dimensionalities"));
        }

        std::int64_t index;
        primitive_argument_type local_value;

        if (ndim == 0)
        {
            index = Op::index_initial();

            switch (extract_common_type(args[0]))
            {
            case node_data_type_bool:
                local_value = primitive_argument_type(
                    Op::template initial<std::uint8_t>());
                break;

            case node_data_type_int64:
                local_value = primitive_argument_type(
                    Op::template initial<std::int64_t>());
                break;

            case node_data_type_double:
                HPX_FALLTHROUGH;
            case node_data_type_unknown:
                local_value = primitive_argument_type(
                    Op::template initial<double>());
                break;

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_argminmax<Op, Derived>::argminmax1d",
                    generate_error_message(
                        "the dist_argminmax primitive requires for all "
                        "arguments to be numeric data types"));
            }
        }
        else    // ndim==1
        {
            primitive_argument_type local_result = common::argminmax1d<Op>(
                std::move(args), name_, codename_, &local_value);

            // correct index to be global
            index = extract_scalar_integer_value_strict(
                std::move(local_result), name_, codename_);

            std::size_t span_index = 0;
            if (!locs.has_span(0))
            {
                HPX_ASSERT(locs.has_span(1));
                span_index = 1;
            }

            index += locs.get_span(span_index).start_;
        }

        switch (extract_common_type(local_value))
        {
        case node_data_type_bool:
            return detail::argminmax1d_reduce<Op>(
                extract_scalar_boolean_value_strict(
                    std::move(local_value), name_, codename_),
                index, locs);

        case node_data_type_int64:
            return detail::argminmax1d_reduce<Op>(
                extract_scalar_integer_value_strict(
                    std::move(local_value), name_, codename_),
                index, locs);

        case node_data_type_double:
            return detail::argminmax1d_reduce<Op>(
                extract_scalar_numeric_value_strict(
                    std::move(local_value), name_, codename_),
                index, locs);

        case node_data_type_unknown:
            return detail::argminmax1d_reduce<Op>(
                extract_scalar_numeric_value(
                    std::move(local_value), name_, codename_),
                index, locs);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_argminmax<Op, Derived>::argminmax1d",
            generate_error_message(
                "the dist_argminmax primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax2d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax2d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    execution_tree::primitive_argument_type
    dist_argminmax<Op, Derived>::argminmax3d(
        execution_tree::primitive_arguments_type&& args) const
    {
        return common::argminmax3d<Op>(std::move(args), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<execution_tree::primitive_argument_type>
    dist_argminmax<Op, Derived>::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_argminmax<Op, Derived>::eval",
                generate_error_message(
                    "the dist_argminmax primitive requires exactly one or "
                    "two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_argminmax<Op, Derived>::eval",
                    generate_error_message(
                        "the dist_argminmax primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                    -> execution_tree::primitive_argument_type {
                    std::size_t a_dims =
                        execution_tree::extract_numeric_value_dimension(
                            args[0], this_->name_, this_->codename_);
                    switch (a_dims)
                    {
                    case 0:
                        return this_->argminmax0d(std::move(args));

                    case 1:
                        return this_->argminmax1d(std::move(args));

                    case 2:
                        return this_->argminmax2d(std::move(args));

                    case 3:
                        return this_->argminmax3d(std::move(args));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_argminmax<Op, Derived>::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of "
                                "dimensions"));
                    }
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif
