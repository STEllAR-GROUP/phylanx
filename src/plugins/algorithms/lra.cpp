//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/algorithms/lra.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const lra::match_data =
    {
        hpx::util::make_tuple("lra",
            std::vector<std::string>{
                "lra(_1, _2, _3, _4, _5)",
                "lra(_1, _2, _3, _4)"
            },
            &create_lra, &create_primitive<lra>)
    };

    ///////////////////////////////////////////////////////////////////////////
    lra::lra(std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct add_simd
        {
        public:
            explicit add_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
                ->  decltype(a + std::declval<double>())
            {
                return a + scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDAdd<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a + blaze::set(scalar_);
            }

        private:
            double scalar_;
        };

        struct div0dnd_simd
        {
        public:
            explicit div0dnd_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(std::declval<double>() / a)
            {
                return scalar_ / a;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDDiv<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return blaze::set(scalar_) / a;
            }

        private:
            double scalar_;
        };
    }

    primitive_argument_type lra::calculate_lra(
        std::vector<primitive_argument_type> && args) const
    {
        // extract arguments
        auto arg1 = extract_numeric_value(args[0], name_, codename_);
        if (arg1.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the first "
                    "argument ('x') to represent a matrix"));
        }
        auto x = arg1.matrix();

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the second "
                    "argument ('y') to represent a vector"));
        }
        auto y = arg2.vector();

        // verify correctness of argument dimensions
        if (x.rows() != y.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the number "
                    "of rows in 'x' to be equal to the size of 'y'"));
        }

        auto arg3 = extract_numeric_value(args[2], name_, codename_);
        if (arg3.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the second "
                    "argument ('y') to represent a vector"));
        }
        auto alpha = arg3.scalar();

        auto iterations = extract_integer_value(args[3], name_, codename_);

        bool enable_output = false;
        if (args.size() == 5 && valid(args[4]))
        {
            enable_output = extract_integer_value(args[4], name_, codename_) != 0;
        }

        using vector_type = ir::node_data<double>::storage1d_type;
        using matrix_type = ir::node_data<double>::storage2d_type;

        // perform calculations
        vector_type weights(x.columns(), 0.0);
        matrix_type transx = blaze::trans(x);

        for (std::int64_t step = 0; step < iterations; ++step)
        {
            if (enable_output)
            {
                hpx::cout << "step: " << step << ", " << weights << std::endl;
            }

            // pred = 1.0 / (1.0 + exp(-dot(x, weights)))
            vector_type pred = blaze::map(
                blaze::map(blaze::exp(-(x * weights)), detail::add_simd(1.0)),
                detail::div0dnd_simd(1.0));

            weights = weights - (alpha * (transx * (pred - y)));
        }

        return primitive_argument_type{std::move(weights)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> lra::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 4 && operands.size() != 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires exactly either "
                        "four or five operands"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type> && args)
            ->  primitive_argument_type
            {
                return this_->calculate_lra(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> lra::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
