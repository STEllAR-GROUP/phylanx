// Copyright (c) 2017-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/generic_function.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/random.hpp>
#include <phylanx/util/random.hpp>
#include <phylanx/util/truncated_normal_distribution.hpp>

#include <hpx/assert.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <type_traits>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const random::match_data = {
        match_pattern_type("random",
            std::vector<std::string>{
                "random(_1, __arg(_2_dist, nil), __arg(_3_dtype, nil))"},
            &create_random, &create_primitive<random>, R"(
            size, dist, dtype
            Args:

                size (int or tuple of ints) : the size of the array of random
                    numbers to generate
                dist (optional, string or list) : the name of the
                    distribution, or a list that begins with the name and is
                    followed by up to two numeric parameters.
                dtype (optional, string) : the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            An array of random numbers.)"),
        match_pattern_type("random_sample",
            std::vector<std::string>{"random_sample(__arg(_1_size, nil))"},
            &create_random, &create_primitive<random>, R"(
            size
            Args:

                size (int or tuple of ints) : the size of the array of random
                    numbers to generate

            Returns:

            An array of random numbers.)"),
        match_pattern_type("random_integers",
            std::vector<std::string>{
                "random_integers(low, __arg(_2_high, nil), "
                    "__arg(_3_size, nil))"},
            &create_random, &create_primitive<random>, R"(
            low, high, size
            Args:

                low (int) : Lowest (signed) integer to be drawn from the
                    distribution (unless high=nil, in which case this parameter
                    is the highest such integer, the lowest in this case is 1).
                high (int, optional) : If provided, the largest (signed) integer
                    to be drawn from the distribution
                size (int or tuple of ints, optional) : the size of the array of
                    random numbers to generate

            Returns:

            An array of random numbers.)"),
        match_pattern_type("randn",
            std::vector<std::string>{"randn(__1)"},
            &create_random, &create_primitive<random>, R"(
            dN*
            Args:

                dN (integer) : The dimensions of the returned array, must be
                    non-negative. If no argument is given a single Python float
                    is returned.

            Returns:

            Return an array filled with random numbers from the
            "standard normal" distribution.)"),
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> adjust_dimensions(
            ir::node_data<T> const& data, std::string const& name,
            std::string const& codename)
        {
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result{
                data.dimensions()};

            switch (data.num_dimensions())
            {
            case 0:
                result[0] = extract_scalar_integer_value(data, name, codename);
                break;

            case 4:  [[fallthrough]];
            case 3:  [[fallthrough]];
            case 1:  [[fallthrough]];
            case 2:
                break;

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::adjust_dimensions",
                    util::generate_error_message(
                        "primitive_argument_type does not represent a "
                        "supported dimensionality",
                        name, codename));
            }

            return result;
        }
    }

    // extract the required dimensionality from argument 1
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> extract_dimensions(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case primitive_argument_type::bool_index:
            return detail::adjust_dimensions(
                util::get<1>(val), name, codename);

        case primitive_argument_type::int64_index:
            return detail::adjust_dimensions(
                util::get<2>(val), name, codename);

        case primitive_argument_type::float64_index:
            return detail::adjust_dimensions(
                util::get<4>(val), name, codename);

        case primitive_argument_type::list_index:
            {
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result{};
                auto const& args = util::get<7>(val);
                switch (args.size())
                {
                case 4:
                {
                    auto elem_0 = args.begin();
                    result[0] = extract_scalar_integer_value(
                        *elem_0, name, codename);
                    result[1] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                    result[2] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                    result[3] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                }
                    return result;

                case 3:
                {
                    auto elem_0 = args.begin();
                    result[0] = extract_scalar_integer_value(
                        *elem_0, name, codename);
                    result[1] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                    result[2] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                }
                    return result;

                case 2:
                {
                    auto elem_0 = args.begin();
                    result[0] = extract_scalar_integer_value(
                        *elem_0, name, codename);
                    result[1] = extract_scalar_integer_value(
                        *(++elem_0), name, codename);
                }
                    return result;

                case 1:
                    result[0] = extract_scalar_integer_value(
                        *args.begin(), name, codename);
                    return result;

                case 0:
                    result[0] = extract_scalar_integer_value(
                        *args.begin(), name, codename);
                    return result;

                default:
                    break;
                }
            }
            break;

        case primitive_argument_type::nil_index:
            return std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{};

        case primitive_argument_type::string_index: [[fallthrough]];
        case primitive_argument_type::primitive_index: [[fallthrough]];
        case primitive_argument_type::future_index: [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::extract_dimensions",
            util::generate_error_message(
                "primitive_argument_type does not hold a dimensionality "
                    "description",
                name, codename));
    }

    // This function extracts the required dimensions of the returned data,
    // note, that in case of a zero-dimensional argument we return a
    // one-dimensional result as the value specifies the number of values to
    // return. In case of 'nil' we return z zero-dimensional argument as
    // exactly one random number should be generated.
    hpx::future<std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>>
    dimensions_operand(primitive_argument_type const& val,
        primitive_arguments_type const& args, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args, std::move(ctx)).then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_dimensions(f.get(), name, codename);
                });
        }
        return hpx::make_ready_future(extract_dimensions(val, name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // extract the required distribution parameters from argument 2
    //
    // get<0>: name of the distribution to use
    // get<1>: number of additional arguments supplied
    // get<2>: 1st optional additional parameter
    // get<3>: 2nd optional additional parameter
    //
    distribution_parameters_type extract_distribution_parameters(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 7:    // phylanx::ir::range
            {
                distribution_parameters_type result{"normal", 0, 0.0, 1.0};
                auto const& list = util::get<7>(val);
                auto const& args = list.args();
                switch (args.size())
                {
                case 3:
                    std::get<1>(result)++;
                    std::get<3>(result) = double(extract_numeric_value(args[2])[0]);
                    [[fallthrough]];

                case 2:
                    std::get<1>(result)++;
                    std::get<2>(result) = double(extract_numeric_value(args[1])[0]);
                    [[fallthrough]];

                case 1:
                    std::get<0>(result) = extract_string_value(args[0]);
                    return result;

                default:
                    break;
                }
            }
            break;

        case 3: // string
            return distribution_parameters_type{util::get<3>(val), 0, 0.0, 1.0};

        case 0: [[fallthrough]];    // nil
        case 1: [[fallthrough]];    // phylanx::ir::node_data<std::uint8_t>
        case 2: [[fallthrough]];    // std::uint64_t
        case 4: [[fallthrough]];    // phylanx::ir::node_data<double>
        case 5: [[fallthrough]];    // primitive
        case 6: [[fallthrough]];    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "extract_distribution_parameters",
            util::generate_error_message(
                "primitive_argument_type does not hold a distribution "
                    "parameters description",
                name, codename));
    }

    hpx::future<distribution_parameters_type> distribution_parameters_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename, eval_context ctx)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args, std::move(ctx)).then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_distribution_parameters(
                        f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_distribution_parameters(val, name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        random::operation extract_random_operation_helper(
            std::string const& name)
        {
            if (name.find("random_sample") == 0)
                return random::operation::random_sample;

            if (name.find("random_integers") == 0)
                return random::operation::random_integers;

            if (name.find("randn") == 0)
                return random::operation::randn;

            return random::operation::random;
        }

        random::operation extract_random_operation(std::string const& name)
        {
            compiler::primitive_name_parts name_parts;
            if (compiler::parse_primitive_name(name, name_parts))
            {
                return extract_random_operation_helper(name_parts.primitive);
            }
            return extract_random_operation_helper(name);
        }
    }

    random::random(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , random_operation_(detail::extract_random_operation(name))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Dist, typename T>
        ir::node_data<T> randomize(Dist& dist, T& d)
        {
            d = dist(util::rng_);
            return ir::node_data<T>{d};
        }

        template <typename Dist, typename T>
        ir::node_data<T> randomize(
            Dist& dist, blaze::DynamicVector<T>& v)
        {
            std::size_t const size = v.size();

            for (std::size_t i = 0; i != size; ++i)
            {
                v[i] = dist(util::rng_);
            }

            return ir::node_data<T>{std::move(v)};
        }

        template <typename Dist, typename T>
        ir::node_data<T> randomize(
            Dist& dist, blaze::DynamicMatrix<T>& m)
        {
            std::size_t const rows = m.rows();
            std::size_t const columns = m.columns();

            for (std::size_t i = 0; i != rows; ++i)
            {
                for (std::size_t j = 0; j != columns; ++j)
                {
                    m(i, j) = dist(util::rng_);
                }
            }

            return ir::node_data<T>{std::move(m)};
        }

        template <typename Dist, typename T>
        ir::node_data<T> randomize(
            Dist& dist, blaze::DynamicTensor<T>& t)
        {
            std::size_t const pages = t.pages();
            std::size_t const rows = t.rows();
            std::size_t const columns = t.columns();

            for (std::size_t k = 0; k != pages; ++k)
            {
                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        t(k, i, j) = dist(util::rng_);
                    }
                }
            }

            return ir::node_data<T>{std::move(t)};
        }

        template <typename Dist, typename T>
        ir::node_data<T> randomize(
            Dist& dist, blaze::DynamicArray<4UL, T>& q)
        {
            std::size_t const quats = q.quats();
            std::size_t const pages = q.pages();
            std::size_t const rows  = q.rows();
            std::size_t const columns = q.columns();

            for (std::size_t l = 0; l != quats; ++l)
            {
                for (std::size_t k = 0; k != pages; ++k)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            q(l, k, i, j) = dist(util::rng_);
                        }
                    }
                }
            }

            return ir::node_data<T>{std::move(q)};
        }

        ///////////////////////////////////////////////////////////////////////
        struct distribution
        {
            virtual ~distribution() = default;

            virtual primitive_argument_type call0d(node_data_type dtype) = 0;
            virtual primitive_argument_type call1d(
                std::size_t dim, node_data_type dtype) = 0;
            virtual primitive_argument_type call2d(
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
                node_data_type dtype) = 0;
            virtual primitive_argument_type call3d(
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
                node_data_type dtype) = 0;
            virtual primitive_argument_type call4d(
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
                node_data_type dtype) = 0;
        };

        using create_distribution_type = std::unique_ptr<distribution> (*)(
            distribution_parameters_type const&, std::string const&,
            std::string const&, eval_context);

        template <typename Result, typename T>
        typename std::enable_if<!std::is_same<Result, T>::value,
            primitive_argument_type>::type
        convert_to(ir::node_data<T>&& val)
        {
            return primitive_argument_type(
                ir::node_data<Result>(std::move(val)));
        }

        template <typename Result, typename T>
        typename std::enable_if<std::is_same<Result, T>::value,
            primitive_argument_type>::type
        convert_to(ir::node_data<T>&& val)
        {
            return primitive_argument_type(std::move(val));
        }

        template <typename T, typename Dist, typename Array>
        primitive_argument_type randomize(
            Dist& dist, Array& data, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            ir::node_data<T> result = randomize(dist, data);
            switch (dtype)
            {
            case node_data_type_int64:
                return convert_to<std::int64_t>(std::move(result));

            case node_data_type_unknown: [[fallthrough]];
            case node_data_type_double:
                return convert_to<double>(std::move(result));

            case node_data_type_bool:
                return convert_to<std::uint8_t>(std::move(result));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::random::randomize",
                util::generate_error_message(
                    "unsupported requested numeric data type", name, codename,
                    ctx.back_trace()));
        }

////////////////////////////////////////////////////////////////////////////////
#define PHYLANX_RANDOM_IMPLEMENT_TENSOR(T)                                     \
    primitive_argument_type call3d(                                            \
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,           \
        node_data_type dtype) override                                         \
    {                                                                          \
        blaze::DynamicTensor<T> data(dims[0], dims[1], dims[2]);               \
        return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);       \
    }                                                                          \
    /**/
#define PHYLANX_RANDOM_IMPLEMENT_QUATERN(T)                                    \
    primitive_argument_type call4d(                                            \
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,           \
        node_data_type dtype) override                                         \
    {                                                                          \
        blaze::DynamicArray<4UL, T> data(dims[0], dims[1], dims[2], dims[3]);  \
        return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);       \
    }                                                                          \
    /**/

#define PHYLANX_RANDOM_DISTRIBUTION_1(type, stdtype, param, T)                 \
    struct type##_distribution : distribution                                  \
    {                                                                          \
        type##_distribution(distribution_parameters_type const& params,        \
            std::string const& name, std::string const& codename,              \
            eval_context ctx)                                                  \
          : name_(name)                                                        \
          , codename_(codename)                                                \
          , ctx_(std::move(ctx))                                               \
        {                                                                      \
            switch (std::get<1>(params))                                       \
            {                                                                  \
            case 0:                                                            \
                dist_ = stdtype{};                                             \
                break;                                                         \
                                                                               \
            case 1:                                                            \
                dist_ = stdtype{static_cast<param>(std::get<2>(params))};      \
                break;                                                         \
                                                                               \
            case 2:                                                            \
            default:                                                           \
                HPX_ASSERT(                                                    \
                    std::get<1>(params) >= 0 && std::get<1>(params) <= 1);     \
                break;                                                         \
            }                                                                  \
        }                                                                      \
                                                                               \
        primitive_argument_type call0d(node_data_type dtype) override          \
        {                                                                      \
            T data;                                                            \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        primitive_argument_type call1d(                                        \
            std::size_t dim, node_data_type dtype) override                    \
        {                                                                      \
            blaze::DynamicVector<T> data(dim);                                 \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        primitive_argument_type call2d(                                        \
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,       \
            node_data_type dtype) override                                     \
        {                                                                      \
            blaze::DynamicMatrix<T> data(dims[0], dims[1]);                    \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        PHYLANX_RANDOM_IMPLEMENT_TENSOR(T)                                     \
        PHYLANX_RANDOM_IMPLEMENT_QUATERN(T)                                    \
        stdtype dist_;                                                         \
        std::string const& name_;                                              \
        std::string const& codename_;                                          \
        eval_context ctx_;                                                     \
    };                                                                         \
    /**/

#define PHYLANX_RANDOM_DISTRIBUTION_2(type, stdtype, param, T)                 \
    struct type##_distribution : distribution                                  \
    {                                                                          \
        type##_distribution(distribution_parameters_type const& params,        \
            std::string const& name, std::string const& codename,              \
            eval_context ctx)                                                  \
          : name_(name)                                                        \
          , codename_(codename)                                                \
          , ctx_(std::move(ctx))                                               \
        {                                                                      \
            switch (std::get<1>(params))                                       \
            {                                                                  \
            case 0:                                                            \
                dist_ = stdtype{};                                             \
                break;                                                         \
                                                                               \
            case 1:                                                            \
                dist_ = stdtype(static_cast<param>(std::get<2>(params)));      \
                break;                                                         \
                                                                               \
            case 2:                                                            \
                dist_ = stdtype(static_cast<param>(std::get<2>(params)),       \
                    std::get<3>(params));                                      \
                break;                                                         \
                                                                               \
            default:                                                           \
                HPX_ASSERT(                                                    \
                    std::get<1>(params) >= 0 && std::get<1>(params) <= 2);     \
                break;                                                         \
            }                                                                  \
        }                                                                      \
                                                                               \
        primitive_argument_type call0d(node_data_type dtype) override          \
        {                                                                      \
            T data;                                                            \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        primitive_argument_type call1d(                                        \
            std::size_t dim, node_data_type dtype) override                    \
        {                                                                      \
            blaze::DynamicVector<T> data(dim);                                 \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        primitive_argument_type call2d(                                        \
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,       \
            node_data_type dtype) override                                     \
        {                                                                      \
            blaze::DynamicMatrix<T> data(dims[0], dims[1]);                    \
            return randomize<T>(dist_, data, dtype, name_, codename_, ctx_);   \
        }                                                                      \
        PHYLANX_RANDOM_IMPLEMENT_TENSOR(T)                                     \
        PHYLANX_RANDOM_IMPLEMENT_QUATERN(T)                                    \
        stdtype dist_;                                                         \
        std::string const& name_;                                              \
        std::string const& codename_;                                          \
        eval_context ctx_;                                                     \
    };                                                                         \
    /**/

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(uniform_int,
        std::uniform_int_distribution<std::int64_t>, std::int64_t,
        std::int64_t);

    std::unique_ptr<distribution> create_uniform_int(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) > std::get<3>(params))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_uniform_int",
                util::generate_error_message(
                    hpx::util::format(
                        "the uniform_int distributions requires for the given "
                        "min value to be less than the given max value "
                        "(actual values: min: {}, max: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new uniform_int_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(uniform,
        std::uniform_real_distribution<double>, double, double);

    std::unique_ptr<distribution> create_uniform(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) > std::get<3>(params))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_uniform",
                util::generate_error_message(
                    hpx::util::format(
                        "the uniform distributions requires for the given min "
                        "value to be less than the given max value "
                        "(actual values: min: {}, max: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new uniform_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(bernoulli,
        std::bernoulli_distribution, double, std::uint8_t);

    std::unique_ptr<distribution> create_bernoulli(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) < 0 || std::get<2>(params) > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_bernoulli",
                util::generate_error_message(
                    hpx::util::format(
                        "the bernoulli distribution requires for the given "
                        "probability argument to be in the range "
                        "of [0, 1] (actual value: p: {})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new bernoulli_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(
        binomial, std::binomial_distribution<int>, int, double);

    std::unique_ptr<distribution> create_binomial(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) < 0 || std::get<3>(params) < 0 ||
            std::get<3>(params) > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_binomial",
                util::generate_error_message(
                    hpx::util::format(
                        "the binomial distribution requires for the number of "
                        "trials to be non-negative and the "
                        "probability to be in "
                        "the range of [0, 1] (actual values: t: {}, p: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new binomial_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(negative_binomial,
        std::negative_binomial_distribution<int>, int, double);

    std::unique_ptr<distribution> create_negative_binomial(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0 || std::get<3>(params) <= 0 ||
            std::get<3>(params) >= 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_negative_binomial",
                util::generate_error_message(
                    hpx::util::format(
                        "the negative binomial distribution requires for the "
                        "number of trials to be strictly positive "
                        "and the probability to be in the range of (0, 1) "
                        "(actual values: k: {}, p: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new negative_binomial_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(geometric,
        std::geometric_distribution<int>, double, double);

    std::unique_ptr<distribution> create_geometric(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0 || std::get<2>(params) >= 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_geometric",
                util::generate_error_message(
                    hpx::util::format(
                        "the geometric distribution requires for the given "
                        "probability argument to be in the range of (0, 1) "
                        "(actual value: p: {})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new geometric_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(
        poisson, std::poisson_distribution<int>, double, double);

    std::unique_ptr<distribution> create_poisson(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_poisson",
                util::generate_error_message(
                    hpx::util::format(
                        "the poisson distribution requires for its argument to be "
                        "strictly positive (non-zero) (actual value: m: {})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new poisson_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(exponential,
        std::exponential_distribution<double>, double, double);

    std::unique_ptr<distribution> create_exponential(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_exponential",
                util::generate_error_message(
                    hpx::util::format(
                        "the exponential distribution requires for its argument "
                        "to be strictly positive (non-zero) (actual value: "
                        "lambda: {})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new exponential_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(
        gamma, std::gamma_distribution<double>, double, double);

    std::unique_ptr<distribution> create_gamma(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0 || std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_gamma",
                util::generate_error_message(
                    hpx::util::format(
                        "the gamma distribution requires for its arguments "
                        "to be strictly positive (non-zero) (actual values: "
                        "alpha: {}, beta: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new gamma_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(
        weibull, std::weibull_distribution<double>, double, double);

    std::unique_ptr<distribution> create_weibull(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0 || std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_weibull",
                util::generate_error_message(
                    hpx::util::format(
                        "the weibull distribution requires for its arguments "
                        "to be strictly positive (non-zero) (actual values: "
                        "a: {}, b: {})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new weibull_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(extreme_value,
        std::extreme_value_distribution<double>, double, double);

    std::unique_ptr<distribution> create_extreme_value(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_extreme_value",
                util::generate_error_message(
                    hpx::util::format(
                        "the extreme_value distribution requires for its third "
                        "argument to be strictly positive (non-zero)  actual "
                        "value: b: {})",
                        std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new extreme_value_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(
        normal, std::normal_distribution<double>, double, double);

    std::unique_ptr<distribution> create_normal(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_normal",
                util::generate_error_message(
                    hpx::util::format(
                        "the normal distribution requires for its third "
                        "argument to be strictly positive (non-zero) (actual "
                        "value: s: {})",
                        std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new normal_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(truncated_normal,
        util::truncated_normal_distribution<double>, double,
        double);

    std::unique_ptr<distribution> create_truncated_normal(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_truncated_normal",
                util::generate_error_message(
                    hpx::util::format(
                        "the truncated_normal distribution requires for its "
                        "third argument to be strictly positive "
                        "(non-zero) (actual value: s: {})",
                        std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new truncated_normal_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(lognormal,
        std::lognormal_distribution<double>, double, double);

    std::unique_ptr<distribution> create_lognormal(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_lognormal",
                util::generate_error_message(
                    hpx::util::format(
                        "the lognormal distribution requires for its third "
                        "argument to be strictly positive (non-zero) (actual "
                        "value: s: {})",
                        std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new lognormal_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(chi_squared,
        std::chi_squared_distribution<double>, double, double);

    std::unique_ptr<distribution> create_chi_squared(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_chi_squared",
                util::generate_error_message(
                    hpx::util::format(
                        "the chi_squared distribution requires for its "
                        "argument to be strictly positive (non-zero) (actual "
                        "value: n: {})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{new chi_squared_distribution(
            params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(
        cauchy, std::cauchy_distribution<double>, double, double);

    std::unique_ptr<distribution> create_cauchy(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_cauchy",
                util::generate_error_message(
                    hpx::util::format(
                        "the cauchy distribution requires for its third "
                        "argument to be strictly positive (non-zero) (actual "
                        "value: b: {})",
                        std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new cauchy_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_2(fisher_f,
        std::fisher_f_distribution<double>, double, double);

    std::unique_ptr<distribution> create_fisher_f(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0 || std::get<3>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_fisher_f",
                util::generate_error_message(
                    hpx::util::format(
                        "the fisher_f distribution requires for its arguments "
                        "to be positive (non-zero) (actual values: m: {}, n: "
                        "{})",
                        std::get<2>(params), std::get<3>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new fisher_f_distribution(params, name, codename, std::move(ctx))};
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_RANDOM_DISTRIBUTION_1(student_t,
        std::student_t_distribution<double>, double, double);

    std::unique_ptr<distribution> create_student_t(
        distribution_parameters_type const& params, std::string const& name,
        std::string const& codename, eval_context ctx)
    {
        if (std::get<2>(params) <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::create_student_t",
                util::generate_error_message(
                    hpx::util::format(
                        "the student_t distribution requires for its argument "
                        "to be strictly positive (non-zero) (actual value: n: "
                        "{})",
                        std::get<2>(params)),
                    name, codename, ctx.back_trace()));
        }
        return std::unique_ptr<distribution>{
            new student_t_distribution(params, name, codename, std::move(ctx))};
    }

#undef PHYLANX_RANDOM_DISTRIBUTION_1
#undef PHYLANX_RANDOM_DISTRIBUTION_2
#undef PHYLANX_RANDOM_IMPLEMENT_TENSOR
#undef PHYLANX_RANDOM_IMPLEMENT_QUATERN

        ///////////////////////////////////////////////////////////////////////
        std::map<std::string, create_distribution_type> distributions =
        {
            { "uniform_int", create_uniform_int },
            { "uniform", create_uniform },
            { "bernoulli", create_bernoulli },
            { "binomial", create_binomial },
            { "negative_binomial", create_negative_binomial },
            { "geometric", create_geometric },
            { "poisson", create_poisson },
            { "exponential", create_exponential },
            { "gamma", create_gamma },
            { "weibull", create_weibull },
            { "extreme_value", create_extreme_value },
            { "normal", create_normal },
            { "truncated_normal", create_truncated_normal },
            { "lognormal", create_lognormal },
            { "chi_squared", create_chi_squared },
            { "cauchy", create_cauchy },
            { "fisher_f", create_fisher_f },
            { "student_t", create_student_t }
        };

        ///////////////////////////////////////////////////////////////////////
        primitive_argument_type randomize0d(
            distribution_parameters_type&& params, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                std::ostringstream msg;
                msg << "attempting to use an unknown random number "
                       "distribution: "
                    << std::get<0>(params) << ". Known distributions are: ";
                bool first = true;
                for (auto&& dist : distributions)
                {
                    if (!first)
                    {
                        msg << ", ";
                        first = false;
                    }
                    msg << dist.first;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize0d",
                    util::generate_error_message(
                            msg.str(), name, codename, ctx.back_trace()));
            }
            return (it->second)(params, name, codename, std::move(ctx))
                ->call0d(dtype);
        }

        primitive_argument_type randomize1d(std::size_t dim,
            distribution_parameters_type&& params, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                std::ostringstream msg;
                msg << "attempting to use an unknown random number "
                       "distribution: "
                    << std::get<0>(params) << ". Known distributions are: ";
                bool first = true;
                for (auto&& dist : distributions)
                {
                    if (!first)
                    {
                        msg << ", ";
                        first = false;
                    }
                    msg << dist.first;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize1d",
                    util::generate_error_message(
                        msg.str(), name, codename, ctx.back_trace()));
            }
            return (it->second)(params, name, codename, std::move(ctx))
                ->call1d(dim, dtype);
        }

        primitive_argument_type randomize2d(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
            distribution_parameters_type&& params, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                std::ostringstream msg;
                msg << "attempting to use an unknown random number "
                       "distribution: "
                    << std::get<0>(params) << ". Known distributions are: ";
                bool first = true;
                for (auto&& dist : distributions)
                {
                    if (!first)
                    {
                        msg << ", ";
                        first = false;
                    }
                    msg << dist.first;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize2d",
                    util::generate_error_message(
                        msg.str(), name, codename, ctx.back_trace()));
            }
            return (it->second)(params, name, codename, std::move(ctx))
                ->call2d(dims, dtype);
        }

        primitive_argument_type randomize3d(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
            distribution_parameters_type&& params, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                std::ostringstream msg;
                msg << "attempting to use an unknown random number "
                       "distribution: "
                    << std::get<0>(params) << ". Known distributions are: ";
                bool first = true;
                for (auto&& dist : distributions)
                {
                    if (!first)
                    {
                        msg << ", ";
                        first = false;
                    }
                    msg << dist.first;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize3d",
                    util::generate_error_message(
                        msg.str(), name, codename, ctx.back_trace()));
            }
            return (it->second)(params, name, codename, std::move(ctx))
                ->call3d(dims, dtype);
        }

        primitive_argument_type randomize4d(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
            distribution_parameters_type&& params, node_data_type dtype,
            std::string const& name, std::string const& codename,
            eval_context ctx)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                std::ostringstream msg;
                msg << "attempting to use an unknown random number "
                       "distribution: "
                    << std::get<0>(params) << ". Known distributions are: ";
                bool first = true;
                for (auto&& dist : distributions)
                {
                    if (!first)
                    {
                        msg << ", ";
                        first = false;
                    }
                    msg << dist.first;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize4d",
                    util::generate_error_message(
                        msg.str(), name, codename, ctx.back_trace()));
            }
            return (it->second)(params, name, codename, std::move(ctx))
                ->call4d(dims, dtype);
        }

        ///////////////////////////////////////////////////////////////////////
        inline int num_dimensions(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
        {
            if (dims[3] != 0)
            {
                return 4;
            }
            if (dims[2] != 0)
            {
                return 3;
            }
            if (dims[1] != 0)
            {
                return 2;
            }
            if (dims[0] != 0)
            {
                return 1;
            }
            return 0;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> random::eval_random_integers(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        using array_type = std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>;

        auto&& low = value_operand(operands[0], args, name_, codename_, ctx);
        auto&& high = value_operand(operands.size() > 1 && valid(operands[1]) ?
                operands[1] :
                primitive_argument_type{},
            args, name_, codename_, ctx);

        auto&& dims = dimensions_operand(
            operands.size() < 3 ? primitive_argument_type{} : operands[2],
            args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<primitive_argument_type>&& low_f,
                hpx::future<primitive_argument_type>&& high_f,
                hpx::future<array_type>&& dims_f) mutable
            -> primitive_argument_type
        {
            primitive_argument_type&& high = high_f.get();
            array_type&& dims = dims_f.get();

            std::int64_t low_value = extract_scalar_integer_value(
                low_f.get(), this_->name_, this_->codename_);
            std::int64_t high_value;

            if (is_integer_operand(high))
            {
                high_value = extract_scalar_integer_value(
                    std::move(high), this_->name_, this_->codename_);
            }
            else
            {
                high_value = low_value;
                low_value = 1;
            }

            distribution_parameters_type params{
                "uniform_int", 2, double(low_value), double(high_value)};

            switch (detail::num_dimensions(dims))
            {
            case 0:
                return this_->random0d(
                    std::move(params), node_data_type_int64, std::move(ctx));

            case 1:
                return this_->random1d(dims[0], std::move(params),
                    node_data_type_int64, std::move(ctx));

            case 2:
                return this_->random2d(dims, std::move(params),
                    node_data_type_int64, std::move(ctx));

            case 3:
                return this_->random3d(dims, std::move(params),
                    node_data_type_int64, std::move(ctx));

            case 4:
                return this_->random4d(dims, std::move(params),
                    node_data_type_int64, std::move(ctx));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random_integers::eval_random_integers",
                    this_->generate_error_message(
                        "size operand has unsupported number of dimensions",
                        std::move(ctx)));
            }
        },
        std::move(low), std::move(high), std::move(dims));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> random::eval_randn(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        using array_type = std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::unwrapping(
            [this_ = std::move(this_), ctx = std::move(ctx)](
                std::vector<ir::node_data<std::int64_t>>&& args) mutable
            -> primitive_argument_type
        {
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims;
            for (std::size_t i = 0; i != args.size(); ++i)
            {
                dims[i] = args[i].scalar();
                if (dims[i] < 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random_integers::eval_randn",
                        this_->generate_error_message(
                            "all operands must be non-negative",
                            std::move(ctx)));
                }
            }

            distribution_parameters_type params{"normal", 2, 0.0, 1.0};
            switch (args.size())
            {
            case 0:
                return this_->random0d(
                    std::move(params), node_data_type_double, std::move(ctx));

            case 1:
                return this_->random1d(dims[0], std::move(params),
                    node_data_type_double, std::move(ctx));

            case 2:
                return this_->random2d(dims, std::move(params),
                    node_data_type_double, std::move(ctx));

            case 3:
                return this_->random3d(dims, std::move(params),
                    node_data_type_double, std::move(ctx));

            case 4:
                return this_->random4d(dims, std::move(params),
                    node_data_type_double, std::move(ctx));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random_integers::eval_randn",
                    this_->generate_error_message(
                        "number of operands implies unsupported number of "
                        "dimensions",
                        std::move(ctx)));
            }
        }),
        detail::map_operands(
            operands, functional::integer_operand_strict{}, args, name_,
            codename_, std::move(ctx)));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> random::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (random_operation_ == operation::random)
        {
            if (operands.size() > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random::eval",
                    generate_error_message(
                        "the random primitive requires at most three operand",
                        std::move(ctx)));
            }

            if ((operands.size() > 1 && !valid(operands[1]) &&
                    !is_explicit_nil(operands[1])) ||
                (operands.size() > 2 && !valid(operands[2]) &&
                    !is_explicit_nil(operands[2])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random::eval",
                    generate_error_message(
                        "the random primitive requires that the arguments "
                        "given by the operands array are valid",
                        std::move(ctx)));
            }
        }
        else if (random_operation_ == operation::random_sample)
        {
            if (operands.size() > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_sample::eval",
                    generate_error_message("the random_sample primitive "
                                           "requires at most three operand",
                        std::move(ctx)));
            }

            if ((operands.size() > 1 && !valid(operands[0]) &&
                !is_explicit_nil(operands[0])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_sample::eval",
                    generate_error_message(
                        "the random_sample primitive requires that the "
                        "arguments given by the operands array are valid",
                        std::move(ctx)));
            }
        }
        else if (random_operation_ == operation::random_integers)
        {
            if (operands.size() > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_integers::eval",
                    generate_error_message("the random_integers primitive "
                                           "requires at most three operand",
                        std::move(ctx)));
            }

            if (!valid(operands[0]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_integers::eval",
                    generate_error_message(
                        "the random_integers primitive requires that the "
                        "arguments given by the operands array are valid",
                        std::move(ctx)));
            }
            if (operands.size() > 1 &&
                (!valid(operands[1]) && !is_explicit_nil(operands[1])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_integers::eval",
                    generate_error_message(
                        "the random_integers primitive requires that the "
                        "arguments given by the operands array are valid",
                        std::move(ctx)));
            }
            if (operands.size() > 2 &&
                (!valid(operands[2]) && !is_explicit_nil(operands[2])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random_integers::eval",
                    generate_error_message(
                        "the random_integers primitive requires that the "
                        "arguments given by the operands array are valid",
                        std::move(ctx)));
            }

            return eval_random_integers(operands, args, std::move(ctx));
        }
        else if (random_operation_ == operation::randn)
        {
            if (operands.size() > 4)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "random::eval",
                    generate_error_message(
                        "the randn primitive supports at most four operand",
                        std::move(ctx)));
            }

            return eval_randn(operands, args, std::move(ctx));
        }

        // the first argument encodes the requested dimensionality, this
        // can either be an instance of node_data or a list of values
        using array_type = std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>;
        hpx::future<array_type> dims;

        // the second (optional) argument encodes the distribution to use
        hpx::future<distribution_parameters_type> params;

        if (random_operation_ == operation::random)
        {
            dims = dimensions_operand(
                operands.empty() ? primitive_argument_type{} : operands[0],
                args, name_, codename_, ctx);
            if (operands.size() < 2 || !valid(operands[1]))
            {
                params = hpx::make_ready_future(
                    distribution_parameters_type{"normal", 2, 0.0, 1.0});
            }
            else
            {
                params = distribution_parameters_operand(
                    operands[1], args, name_, codename_, std::move(ctx));
            }
        }
        else if(random_operation_ == operation::random_sample)
        {
            dims = dimensions_operand(
                operands.empty() ? primitive_argument_type{} : operands[0],
                args, name_, codename_, ctx);
            params = hpx::make_ready_future(
                distribution_parameters_type{"uniform", 2, 0.0, 1.0});
        }

        if (operands.size() < 3 || !valid(operands[2]))
        {
            auto this_ = this->shared_from_this();
            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_), ctx = std::move(ctx)](
                        hpx::future<array_type>&& dims_f,
                        hpx::future<distribution_parameters_type>&& params_f)
                mutable ->  primitive_argument_type
                {
                    array_type&& dims = dims_f.get();
                    distribution_parameters_type&& params = params_f.get();

                    switch (detail::num_dimensions(dims))
                    {
                    case 0:
                        return this_->random0d(
                            std::move(params), node_data_type_double, ctx);

                    case 1:
                        return this_->random1d(dims[0], std::move(params),
                            node_data_type_double, std::move(ctx));

                    case 2:
                        return this_->random2d(dims, std::move(params),
                            node_data_type_double, std::move(ctx));

                    case 3:
                        return this_->random3d(dims, std::move(params),
                            node_data_type_double, std::move(ctx));

                    case 4:
                        return this_->random4d(dims, std::move(params),
                            node_data_type_double, std::move(ctx));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "random::eval",
                            this_->generate_error_message(
                                "size operand has unsupported number of "
                                "dimensions",
                                std::move(ctx)));
                    }
                }, std::move(dims), std::move(params));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                    hpx::future<array_type>&& dims_f,
                    hpx::future<distribution_parameters_type>&& params_f,
                    primitive_argument_type const& dtype_arg) mutable
            ->  primitive_argument_type
            {
                array_type&& dims = dims_f.get();
                distribution_parameters_type&& params = params_f.get();
                std::string dtype_str = extract_string_value_strict(
                    dtype_arg, this_->name_, this_->codename_);

                node_data_type dtype = map_dtype(dtype_str);
                switch (detail::num_dimensions(dims))
                {
                case 0:
                    return this_->random0d(
                        std::move(params), dtype, std::move(ctx));

                case 1:
                    return this_->random1d(
                        dims[0], std::move(params), dtype, std::move(ctx));

                case 2:
                    return this_->random2d(
                        dims, std::move(params), dtype, std::move(ctx));

                case 3:
                    return this_->random3d(
                        dims, std::move(params), dtype, std::move(ctx));

                case 4:
                    return this_->random4d(
                        dims, std::move(params), dtype, std::move(ctx));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "random::eval",
                        this_->generate_error_message(
                            "size operand has unsupported number of dimensions",
                            std::move(ctx)));
                }
            }, std::move(dims), std::move(params), operands[2]);
    }

    primitive_argument_type random::random0d(
        distribution_parameters_type&& params, node_data_type dtype,
        eval_context ctx) const
    {
        return detail::randomize0d(
            std::move(params), dtype, name_, codename_, std::move(ctx));
    }

    primitive_argument_type random::random1d(std::size_t dim,
        distribution_parameters_type&& params, node_data_type dtype,
        eval_context ctx) const
    {
        auto result = detail::randomize1d(
            dim, std::move(params), dtype, name_, codename_, std::move(ctx));
        return result;
    }

    primitive_argument_type random::random2d(
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
        distribution_parameters_type&& params, node_data_type dtype,
        eval_context ctx) const
    {
        return detail::randomize2d(
            dims, std::move(params), dtype, name_, codename_, std::move(ctx));
    }

    primitive_argument_type random::random3d(
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
        distribution_parameters_type&& params, node_data_type dtype,
        eval_context ctx) const
    {
        return detail::randomize3d(
            dims, std::move(params), dtype, name_, codename_, std::move(ctx));
    }

    primitive_argument_type random::random4d(
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
        distribution_parameters_type&& params, node_data_type dtype,
        eval_context ctx) const
    {
        return detail::randomize4d(
            dims, std::move(params), dtype, name_, codename_, std::move(ctx));
    }
}}}    // namespace phylanx::execution_tree::primitives

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    hpx::future<primitive_argument_type> set_seed(
        primitive_arguments_type const&,
        primitive_arguments_type const&,
        std::string const&, std::string const&, eval_context);

    primitive_argument_type get_seed(
        primitive_arguments_type const&,
        primitive_arguments_type const&,
        std::string const&, std::string const&, eval_context);
}}}

HPX_PLAIN_ACTION(
    phylanx::execution_tree::primitives::set_seed, set_seed_action);
HPX_PLAIN_DIRECT_ACTION(
    phylanx::execution_tree::primitives::get_seed, get_seed_action);

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const set_seed_match_data =
    {
        hpx::make_tuple(
            "set_seed", std::vector<std::string>{"set_seed(_1)"},
            &create_generic_function<set_seed_action>,
            &create_primitive<generic_function<set_seed_action>>,
            R"(seed
            Args:

                seed (int) : the seed of a random number generator

            Returns:)"
            )
    };

    match_pattern_type const get_seed_match_data =
    {
        hpx::make_tuple(
            "get_seed", std::vector<std::string>{"get_seed()"},
            &create_generic_function<get_seed_action>,
            &create_primitive<generic_function<get_seed_action>>,
            R"(
            Args:

            Returns:

            The seed used to generate random numbers.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> set_seed(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename, eval_context ctx)
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::set_seed",
                util::generate_error_message(
                    "the set_seed function requires exactly one operand",
                    name, codename, ctx.back_trace()));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::set_seed",
                util::generate_error_message(
                    "the set_seed function requires that the arguments "
                        "given by the operands array are valid",
                    name, codename, ctx.back_trace()));
        }

        return integer_operand(operands[0], args, name, codename, std::move(ctx))
            .then(hpx::launch::sync, hpx::unwrapping(
                [](ir::node_data<std::int64_t>&& data) -> primitive_argument_type
                {
                    util::set_seed(static_cast<std::uint32_t>(data[0]));
                    return primitive_argument_type{};
                }));
    }

    primitive_argument_type get_seed(
        primitive_arguments_type const&, primitive_arguments_type const&,
        std::string const&, std::string const&, eval_context)
    {
        return primitive_argument_type{
            static_cast<std::int64_t>(util::get_seed())};
    }
}}}
