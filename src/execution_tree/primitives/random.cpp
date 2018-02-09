//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/random.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cmath>
#include <cstddef>
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

    // extract the required dimensionality from argument 1
    std::array<std::size_t, 2> extract_dimensions(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<bool>
            return util::get<1>(val).dimensions();

        case 2:    // std::uint64_t
            return std::array<std::size_t, 2>{
                std::size_t(util::get<2>(val)), 1ull};

        case 4:    // phylanx::ir::node_data<double>
            return util::get<4>(val).dimensions();

        case 7:    // std::vector<primitive_argument_type>
            {
                std::array<std::size_t, 2> result{1ull, 1ull};
                auto const& list = util::get<7>(val).get();
                switch (list.size())
                {
                case 2:
                    result[1] = std::size_t(extract_numeric_value(list[1])[0]);
                    HPX_FALLTHROUGH;

                case 1:
                    result[0] = std::size_t(extract_numeric_value(list[0])[0]);
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
            "primitive_argument_type does not hold a dimensionality "
                "description");
    }

    hpx::future<std::array<std::size_t, 2>> dimensions_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_dimensions(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_dimensions(val));
    }

    ///////////////////////////////////////////////////////////////////////////
    // extract the required distribution parameters from argument 2
    using distribution_parameters_type =
        std::tuple<std::string, double, double>;

    distribution_parameters_type extract_distribution_parameters(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 7:    // std::vector<primitive_argument_type>
            {
                distribution_parameters_type result{"uniform", 0.0, 1.0};
                auto const& list = util::get<7>(val).get();
                switch (list.size())
                {
                case 3:
                    std::get<2>(result) = double(extract_numeric_value(list[2])[0]);
                    HPX_FALLTHROUGH;

                case 2:
                    std::get<1>(result) = double(extract_numeric_value(list[1])[0]);
                    HPX_FALLTHROUGH;

                case 1:
                    std::get<0>(result) = extract_string_value(list[0]);
                    return result;

                default:
                    break;
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<bool>
        case 2: HPX_FALLTHROUGH;    // std::uint64_t
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "extract_distribution_parameters",
            "primitive_argument_type does not hold a distribution parameters "
                "description");
    }

    hpx::future<distribution_parameters_type> distribution_parameters_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_distribution_parameters(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_distribution_parameters(val));
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const random::match_data =
    {
        hpx::util::make_tuple("random",
            std::vector<std::string>{"random(_1)"},
            &create_random, &create_primitive<random>)
    };

    ///////////////////////////////////////////////////////////////////////////
    random::random(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Dist>
        void randomize(Dist const& dist, double& d)
        {
            d = dist(blaze::Random<blaze::RNG>::rng_);
        }

        template <typename Dist>
        void randomize(Dist const& dist, blaze::DynamicVector<double>& v)
        {
            std::size_t const size = v.size();

            for (std::size_t i = 0; i != size; ++i)
            {
                v[i] = dist(blaze::Random<blaze::RNG>::rng_);
            }
        }

        template <typename Dist>
        void randomize(Dist const& dist, blaze::DynamicMatrix<double>& m)
        {
            std::size_t const rows = m.rows();
            std::size_t const columns = m.columns();

            for (std::size_t i = 0; i != rows; ++i)
            {
                for (std::size_t j = 0; j != columns; ++j)
                {
                    m(i, j) = dist(blaze::Random<blaze::RNG>::rng_);
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////
        struct distribution
        {
            virtual ~distribution() = default;

            virtual void call(double& data) const = 0;
            virtual void call(blaze::DynamicVector<double>& data) const = 0;
            virtual void call(blaze::DynamicMatrix<double>& data) const = 0;
        };

        using create_distribution_type = std::unique_ptr<distribution>(*)(
            distribution_parameters_type const&);

        //////////////////////////////////////////////////////////////////////
        struct uniform_distribution : distribution
        {
            uniform_distribution(distribution_parameters_type const& params)
              : dist_(std::get<1>(params), std::get<2>(params))
            {}

            void call(double& data) const override
            {
                return randomize(dist_, data);
            }

            void call(blaze::DynamicVector<double>& data) const override
            {
                return randomize(dist_, data);
            }

            void call(blaze::DynamicMatrix<double>& data) const override
            {
                return randomize(dist_, data);
            }

            std::uniform_real_distribution<double> dist_;
        };

        std::unique_ptr<distribution> create_uniform(
            distribution_parameters_type const& params)
        {
            return std::unique_ptr<distribution>{
                new uniform_distribution(params)};
        }

        ///////////////////////////////////////////////////////////////////////
        std::map<std::string, create_distribution_type> distributions =
        {
            { "uniform", create_uniform }
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename Data>
        void randomize(distribution_parameters_type && params, Data& data)
        {
            return distributions[std::get<0>(params)](params)->call(data);
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

        struct random : std::enable_shared_from_this<random>
        {
            random() = default;

            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args,
                std::string const& name, std::string const& codename)
            {
                if (operands.empty() || operands.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random::eval",
                        generate_error_message(
                        "the random primitive requires"
                                "at most one operand",
                            name, codename));
                }

                if (!valid(operands[0]) ||
                    operands.size() == 2 && !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random::eval",
                        generate_error_message(
                        "the random primitive requires that the "
                                "arguments given by the operands array "
                                "are valid",
                            name, codename));
                }

                // the first argument encodes the requested dimensionality, this
                // can either be an instance of node_data or a list of values
                hpx::future<std::array<std::size_t, 2>> dims =
                    dimensions_operand(operands[0], args);

                // the second (optional) argument encodes the distribution to use
                hpx::future<distribution_parameters_type> params;
                if (operands.size() < 2)
                {
                    params = hpx::make_ready_future(
                        distribution_parameters_type{"uniform", 0.0, 1.0});
                }
                else
                {
                    params = distribution_parameters_operand(operands[1], args);
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](std::array<std::size_t, 2> && dims,
                            distribution_parameters_type && params)
                    ->  primitive_argument_type
                    {
                        switch (num_dimensions(dims))
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
                                generate_error_message(
                                "left hand side operand has unsupported "
                                        "number of dimensions",
                                    name, codename));
                        }
                    }), std::move(dims), std::move(params));
            }

        protected:
            primitive_argument_type random0d(
                distribution_parameters_type&& params) const
            {
                double d = 0.0;
                randomize(params, d);
                return primitive_argument_type{d};
            }

            primitive_argument_type random1d(
                std::size_t dim, distribution_parameters_type&& params) const
            {
                blaze::DynamicVector<double> v(dim);
                randomize(params, v);
                return primitive_argument_type{std::move(v)};
            }

            primitive_argument_type random2d(
                std::array<std::size_t, 2> const& dims,
                distribution_parameters_type&& params) const
            {
                blaze::DynamicMatrix<double> m(dims[0], dims[1]);
                randomize(params, m);
                return primitive_argument_type{std::move(m)};
            }
        };
    }

    hpx::future<primitive_argument_type> random::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::random>()->eval(
                args, noargs, name_, codename_);
        }
        return std::make_shared<detail::random>()->eval(
            operands_, args, name_, codename_);
    }
}}}
