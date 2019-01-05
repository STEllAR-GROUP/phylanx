// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2017-2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/mul_operation.hpp>
#include <phylanx/plugins/arithmetics/numeric_impl.hpp>
#include <phylanx/util/detail/mul_simd.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        // scalars need 't1 * t2', vectors and matrices use blaze::map()
        struct mul_op
        {
            ///////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_scalar<T1>::value && traits::is_scalar<T2>::value,
                decltype(std::declval<T1>() * std::declval<T2>())
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 * t2;
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_vector<T1>::value && traits::is_vector<T2>::value,
                decltype(blaze::map(std::declval<T1>(), std::declval<T2>(),
                    util::detail::mulndnd_simd{}))
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return blaze::map(t1, t2, util::detail::mulndnd_simd{});
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_matrix<T1>::value && traits::is_matrix<T2>::value,
                decltype(std::declval<T1>() % std::declval<T2>())
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 % t2;
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_tensor<T1>::value && traits::is_tensor<T2>::value,
                decltype(std::declval<T1>() % std::declval<T2>())
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 % t2;
            }
#endif

            ///////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_scalar<T1>::value && traits::is_scalar<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 *= t2;
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_vector<T1>::value && traits::is_vector<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 = blaze::map(t1, t2, util::detail::mulndnd_simd{});
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_matrix<T1>::value && traits::is_matrix<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 %= t2;
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_tensor<T1>::value && traits::is_tensor<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 %= t2;
            }
#endif
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mul_operation::match_data =
    {
        match_pattern_type{"__mul",
            std::vector<std::string>{"_1 * __2", "__mul(_1, __2)"},
            &create_mul_operation, &create_primitive<mul_operation>, R"(
            x0, x1
            Args:

                 x0 (number): The factor\n"
                *x1 (number list): A list of one or more factors.\n"

            Returns:

            The product of all factors.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    mul_operation::mul_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}

    template <typename T>
    primitive_argument_type mul_operation::handle_numeric_operands_helper(
        primitive_arguments_type&& ops) const
    {
        if (extract_largest_dimension(ops, name_, codename_) ==
            extract_smallest_dimension(ops, name_, codename_))
        {
            return this->base_type::handle_numeric_operands_helper<T>(
                std::move(ops));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "mul_operation::handle_numeric_operands_helper",
            generate_error_message(
                "the operands have incompatible number of dimensions"));
    }

    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<std::uint8_t>(
        primitive_arguments_type&& ops) const;
    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<std::int64_t>(
        primitive_arguments_type&& ops) const;
    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<double>(
        primitive_arguments_type&& ops) const;

    template <typename T>
    primitive_argument_type mul_operation::handle_numeric_operands_helper(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const
    {
        return this->base_type::handle_numeric_operands_helper<T>(
            std::move(op1), std::move(op2));
    }

    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<std::uint8_t>(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const;
    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<std::int64_t>(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const;
    template primitive_argument_type
    mul_operation::handle_numeric_operands_helper<double>(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const;
}}}
