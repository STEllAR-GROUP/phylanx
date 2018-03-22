//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/generic_function.hpp>
#include <phylanx/execution_tree/primitives/random.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/util/assert.hpp>
#include <hpx/throw_exception.hpp>

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

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_random(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("random");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const random::match_data =
    {
        hpx::util::make_tuple("random",
            std::vector<std::string>{"random(_1)", "random(_1, _2)"},
            &create_random, &create_primitive<random>)
    };

    ///////////////////////////////////////////////////////////////////////////
    std::uint32_t random::default_seed()
    {
        static const std::uint32_t seed =
            static_cast<std::uint32_t>(std::random_device{}());
        return seed;
    }

    std::uint32_t random::seed_ = 0;
    std::mt19937 random::rng_{random::default_seed()};

    void random::set_seed(std::uint32_t seed)
    {
        seed_ = seed;
        rng_.seed(seed_);
    }

    std::uint32_t random::get_seed()
    {
        return seed_;
    }

    ///////////////////////////////////////////////////////////////////////////
    // extract the required dimensionality from argument 1
    std::array<std::size_t, 2> extract_dimensions(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val).dimensions();

        case 2:    // std::uint64_t
            return std::array<std::size_t, 2>{
                std::size_t(util::get<2>(val)), 1ull};

        case 4:    // phylanx::ir::node_data<double>
            return util::get<4>(val).dimensions();

        case 7:    // std::vector<primitive_argument_type>
            {
                std::array<std::size_t, 2> result{1ull, 1ull};
                auto const& list = util::get<7>(val);
                auto const& args = list.args();
                switch (args.size())
                {
                case 2:
                    result[1] = std::size_t(extract_numeric_value(args[1])[0]);
                    HPX_FALLTHROUGH;

                case 1:
                    result[0] = std::size_t(extract_numeric_value(args[0])[0]);
                    HPX_FALLTHROUGH;

                case 0:
                    return result;

                default:
                    break;
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::extract_dimensions",
            phylanx::execution_tree::generate_error_message(
                "primitive_argument_type does not hold a dimensionality "
                    "description",
                name, codename));
    }

    hpx::future<std::array<std::size_t, 2>> dimensions_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_dimensions(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
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
        case 7:    // std::vector<primitive_argument_type>
            {
                distribution_parameters_type result{"uniform", 0, 0.0, 1.0};
                auto const& list = util::get<7>(val);
                auto const& args = list.args();
                switch (args.size())
                {
                case 3:
                    std::get<1>(result)++;
                    std::get<3>(result) = double(extract_numeric_value(args[2])[0]);
                    HPX_FALLTHROUGH;

                case 2:
                    std::get<1>(result)++;
                    std::get<2>(result) = double(extract_numeric_value(args[1])[0]);
                    HPX_FALLTHROUGH;

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

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // std::uint64_t
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "extract_distribution_parameters",
            phylanx::execution_tree::generate_error_message(
                "primitive_argument_type does not hold a distribution "
                    "parameters description",
                name, codename));
    }

    hpx::future<distribution_parameters_type> distribution_parameters_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(hpx::launch::sync,
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
    random::random(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Dist, typename T>
        primitive_argument_type randomize(Dist& dist, T& d)
        {
            d = dist(primitives::random::rng_);
            return primitive_argument_type{d};
        }

        template <typename Dist, typename T>
        primitive_argument_type randomize(
            Dist& dist, blaze::DynamicVector<T>& v)
        {
            std::size_t const size = v.size();

            for (std::size_t i = 0; i != size; ++i)
            {
                v[i] = dist(primitives::random::rng_);
            }

            return primitive_argument_type{std::move(v)};
        }

        template <typename Dist, typename T>
        primitive_argument_type randomize(
            Dist& dist, blaze::DynamicMatrix<T>& m)
        {
            std::size_t const rows = m.rows();
            std::size_t const columns = m.columns();

            for (std::size_t i = 0; i != rows; ++i)
            {
                for (std::size_t j = 0; j != columns; ++j)
                {
                    m(i, j) = dist(primitives::random::rng_);
                }
            }

            return primitive_argument_type{std::move(m)};
        }

        ///////////////////////////////////////////////////////////////////////
        struct distribution
        {
            virtual ~distribution() = default;

            virtual primitive_argument_type call0d() = 0;
            virtual primitive_argument_type call1d(std::size_t dim) = 0;
            virtual primitive_argument_type call2d(
                std::array<std::size_t, 2> const& dims) = 0;
        };

        using create_distribution_type = std::unique_ptr<distribution> (*)(
            distribution_parameters_type const&, std::string const&,
            std::string const&);

        //////////////////////////////////////////////////////////////////////
#define PHYLANX_RANDOM_DISTRIBUTION_1(type, stdtype, param, T)                 \
    struct type##_distribution : distribution                                  \
    {                                                                          \
        type##_distribution(distribution_parameters_type const& params)        \
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
        primitive_argument_type call0d() override                              \
        {                                                                      \
            T data;                                                            \
            return randomize(dist_, data);                                     \
        }                                                                      \
        primitive_argument_type call1d(std::size_t dim) override               \
        {                                                                      \
            blaze::DynamicVector<T> data(dim);                                 \
            return randomize(dist_, data);                                     \
        }                                                                      \
        primitive_argument_type call2d(                                        \
            std::array<std::size_t, 2> const& dims) override                   \
        {                                                                      \
            blaze::DynamicMatrix<T> data(dims[0], dims[1]);                    \
            return randomize(dist_, data);                                     \
        }                                                                      \
        stdtype dist_;                                                         \
    };                                                                         \
                                                                               \
    std::unique_ptr<distribution> create_##type(                               \
        distribution_parameters_type const& params, std::string const& name,   \
        std::string const& codename)                                           \
    {                                                                          \
        return std::unique_ptr<distribution>{new type##_distribution(params)}; \
    }                                                                          \
    /**/

#define PHYLANX_RANDOM_DISTRIBUTION_2(type, stdtype, param, T)                 \
    struct type##_distribution : distribution                                  \
    {                                                                          \
        type##_distribution(distribution_parameters_type const& params)        \
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
                dist_ = stdtype{static_cast<param>(std::get<2>(params)),       \
                    std::get<3>(params)};                                      \
                break;                                                         \
                                                                               \
            default:                                                           \
                HPX_ASSERT(                                                    \
                    std::get<1>(params) >= 0 && std::get<1>(params) <= 2);     \
                break;                                                         \
            }                                                                  \
        }                                                                      \
                                                                               \
        primitive_argument_type call0d() override                              \
        {                                                                      \
            T data;                                                            \
            return randomize(dist_, data);                                     \
        }                                                                      \
        primitive_argument_type call1d(std::size_t dim) override               \
        {                                                                      \
            blaze::DynamicVector<T> data(dim);                                 \
            return randomize(dist_, data);                                     \
        }                                                                      \
        primitive_argument_type call2d(                                        \
            std::array<std::size_t, 2> const& dims) override                   \
        {                                                                      \
            blaze::DynamicMatrix<T> data(dims[0], dims[1]);                    \
            return randomize(dist_, data);                                     \
        }                                                                      \
        stdtype dist_;                                                         \
    };                                                                         \
                                                                               \
    std::unique_ptr<distribution> create_##type(                               \
        distribution_parameters_type const& params, std::string const& name,   \
        std::string const& codename)                                           \
    {                                                                          \
        return std::unique_ptr<distribution>{new type##_distribution(params)}; \
    }                                                                          \
    /**/

        PHYLANX_RANDOM_DISTRIBUTION_2(
            uniform, std::uniform_real_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            bernoulli, std::bernoulli_distribution, double, std::uint8_t);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            binomial, std::binomial_distribution<int>, int, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(negative_binomial,
            std::negative_binomial_distribution<int>, int, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            geometric, std::geometric_distribution<int>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            poisson, std::poisson_distribution<int>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            exponential, std::exponential_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            gamma, std::gamma_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            weibull, std::weibull_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(extreme_value,
            std::extreme_value_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            normal, std::normal_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            lognormal, std::lognormal_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            chi_squared, std::chi_squared_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            cauchy, std::cauchy_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_2(
            fisher_f, std::fisher_f_distribution<double>, double, double);
        PHYLANX_RANDOM_DISTRIBUTION_1(
            student_t, std::student_t_distribution<double>, double, double);

#undef PHYLANX_RANDOM_DISTRIBUTION_1
#undef PHYLANX_RANDOM_DISTRIBUTION_2

        ///////////////////////////////////////////////////////////////////////
        std::map<std::string, create_distribution_type> distributions =
        {
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
            { "lognormal", create_lognormal },
            { "chi_squared", create_chi_squared },
            { "cauchy", create_cauchy },
            { "fisher_f", create_fisher_f },
            { "student_t", create_student_t }
        };

        ///////////////////////////////////////////////////////////////////////
        primitive_argument_type randomize0d(
            distribution_parameters_type&& params, std::string const& name,
            std::string const& codename)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize0d",
                    execution_tree::generate_error_message(
                        "attempting to use an unknown random number "
                            "distribution: " + std::get<0>(params),
                        name, codename));
            }
            return (it->second)(params, name, codename)->call0d();
        }

        primitive_argument_type randomize1d(std::size_t dim,
            distribution_parameters_type&& params, std::string const& name,
            std::string const& codename)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize1d",
                    execution_tree::generate_error_message(
                        "attempting to use an unknown random number "
                            "distribution: " + std::get<0>(params),
                        name, codename));
            }
            return (it->second)(params, name, codename)->call1d(dim);
        }

        primitive_argument_type randomize2d(
            std::array<std::size_t, 2> const& dims,
            distribution_parameters_type&& params, std::string const& name,
            std::string const& codename)
        {
            auto it = distributions.find(std::get<0>(params));
            if (it == distributions.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "random::randomize2d",
                    execution_tree::generate_error_message(
                        "attempting to use an unknown random number "
                            "distribution: " + std::get<0>(params),
                        name, codename));
            }
            return (it->second)(params, name, codename)->call2d(dims);
        }

        ///////////////////////////////////////////////////////////////////////
        inline int num_dimensions(std::array<std::size_t, 2> const& dims)
        {
            if (dims[1] != 1)
            {
                return 2;
            }
            if (dims[0] != 1)
            {
                return 1;
            }
            return 0;
        }
    }

    hpx::future<primitive_argument_type> random::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::eval",
                execution_tree::generate_error_message(
                    "the random primitive requires at most one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::eval",
                execution_tree::generate_error_message(
                    "the random primitive requires that the arguments "
                        "given by the operands array are valid",
                    name_, codename_));
        }

        // the first argument encodes the requested dimensionality, this
        // can either be an instance of node_data or a list of values
        hpx::future<std::array<std::size_t, 2>> dims =
            dimensions_operand(operands[0], args, name_, codename_);

        // the second (optional) argument encodes the distribution to use
        hpx::future<distribution_parameters_type> params;
        if (operands.size() < 2)
        {
            params = hpx::make_ready_future(
                distribution_parameters_type{"uniform", 2, 0.0, 1.0});
        }
        else
        {
            params = distribution_parameters_operand(
                operands[1], args, name_, codename_);
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::array<std::size_t, 2> && dims,
                    distribution_parameters_type && params)
            ->  primitive_argument_type
            {
                switch (detail::num_dimensions(dims))
                {
                case 0:
                    return this_->random0d(std::move(params));

                case 1:
                    return this_->random1d(dims[0], std::move(params));

                case 2:
                    return this_->random2d(dims, std::move(params));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random::eval",
                        execution_tree::generate_error_message(
                        "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }), std::move(dims), std::move(params));
    }

    primitive_argument_type random::random0d(
        distribution_parameters_type&& params) const
    {
        return detail::randomize0d(std::move(params), name_, codename_);
    }

    primitive_argument_type random::random1d(
        std::size_t dim, distribution_parameters_type&& params) const
    {
        auto result = detail::randomize1d(dim, std::move(params), name_, codename_);
        return result;
    }

    primitive_argument_type random::random2d(
        std::array<std::size_t, 2> const& dims,
        distribution_parameters_type&& params) const
    {
        return detail::randomize2d(dims, std::move(params), name_, codename_);
    }

    hpx::future<primitive_argument_type> random::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    hpx::future<primitive_argument_type> set_seed(
        std::vector<primitive_argument_type> const&,
        std::vector<primitive_argument_type> const&,
        std::string const&, std::string const&);

    primitive_argument_type get_seed(
        std::vector<primitive_argument_type> const&,
        std::vector<primitive_argument_type> const&,
        std::string const&, std::string const&);
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
        hpx::util::make_tuple(
            "set_seed", std::vector<std::string>{"set_seed(_1)"},
            &create_generic_function<set_seed_action>,
            &create_primitive<generic_function<set_seed_action>>)
    };

    match_pattern_type const get_seed_match_data =
    {
        hpx::util::make_tuple(
            "get_seed", std::vector<std::string>{"get_seed()"},
            &create_generic_function<get_seed_action>,
            &create_primitive<generic_function<get_seed_action>>)
    };

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> set_seed(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::set_seed",
                execution_tree::generate_error_message(
                    "the set_seed function requires exactly one operand",
                    name, codename));
        }

        if (!valid(operands[0]) || (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "random::set_seed",
                execution_tree::generate_error_message(
                    "the set_seed function requires that the arguments "
                        "given by the operands array are valid",
                    name, codename));
        }

        return numeric_operand(operands[0], args, name, codename)
            .then(hpx::launch::sync, hpx::util::unwrapping(
                [](ir::node_data<double>&& data) -> primitive_argument_type
                {
                    random::set_seed(static_cast<std::uint32_t>(data[0]));
                    return primitive_argument_type{};
                }));
    }

    primitive_argument_type get_seed(
        std::vector<primitive_argument_type> const&,
        std::vector<primitive_argument_type> const&,
        std::string const&, std::string const&)
    {
        return primitive_argument_type{
            static_cast<std::int64_t>(random::get_seed())};
    }
}}}
