// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_RANDOM_OCT_10_2017_0258PM)
#define PHYLANX_PRIMITIVES_RANDOM_OCT_10_2017_0258PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/util/random.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace primitives
{
    using distribution_parameters_type =
        std::tuple<std::string, int, double, double>;

    class random
      : public primitive_component_base
      , public std::enable_shared_from_this<random>
    {
    public:
        static match_pattern_type const match_data;

        random() = default;

        ///
        /// Create a new random data element of the requested dimensions that
        /// is filled using random numbers distributed based on the given
        /// arguments
        ///
        /// \param operands Is a vector with at least one element (in order):
        ///
        /// dimensions: Specifies the number of dimensions of the newly created
        ///             data element\n
        ///             This value can either be another data element, in which
        ///             case the newly created element will have the same
        ///             dimensions. This can also be a list of values to use as
        ///             the dimensions for the newly created data element.\n
        /// parameters: (optional) Specifies what distribution to use to generate
        ///             the random numbers. This has to be a list of parameters
        ///             where the first list element is the name of the
        ///             distribution to use (see below). The remaining list
        ///             elements are specific to the distribution selected.
        ///             (default: <code>list("normal", 0.0, 1.0)</code>).\n
        ///
        /// \note       Possible values for \a parameters\n
        ///     list("uniform_int", min, max): Produces integer values evenly
        ///             distributed across a range ([min, max): range of
        ///             generated values)\n
        ///     list("uniform", min, max): Produces floating point values evenly
        ///             distributed across a range ([min, max): range of
        ///             generated values)\n
        ///     list("bernoulli", p):      Produces random boolean values,
        ///             according to the discrete probability function
        ///             (p: probability of a trial generating true)\n
        ///     list("binomial", t, p):    Produces random non-negative
        ///             integer values, distributed according to the discrete
        ///             binomial probability function (t: number of trials,
        ///             p: probability of a trial generating true)\n
        ///     list("negative_binomial", k, p): Produces random non-
        ///             negative integer values, distributed according to the
        ///             discrete negative binomial probability (k: number of
        ///             trial failures, p: probability of a trial generating
        ///             true)\n
        ///     list("geometric", p): Produces random non-negative integer
        ///             values, distributed according to the discrete geometric
        ///             probability function (p: probability of a trial
        ///             generating true)\n
        ///     list("poisson", mean): Produces random non-negative integer
        ///             values, distributed according to discrete Poisson
        ///             probability function (mean: the mean of the distribution)\n
        ///     list("exponential", lambda): Produces random non-negative
        ///             floating-point values, distributed according to the
        ///             exponential probability density function
        ///             (lambda: the rate parameter)\n
        ///     list("gamma", alpha, beta): Produces random positive
        ///             floating-point values, distributed according to the
        ///             gamma probability density function
        ///             (alpha: shape, beta: scale)\n
        ///     list("weibull", a, b): Produces random positive
        ///             floating-point values, distributed according to the
        ///             Weibull probability density function (a: shape, b: scale)\n
        ///     list("extreme_value", a, b): Produces random positive
        ///             floating-point values, distributed according to the
        ///             extreme value probability density function, also known
        ///             as Gumbel Type I, log-Weibull, or Fisher-Tippett Type I
        ///             distribution (a: location, b: scale)\n
        ///     list("normal", mean, stddev): Generates random numbers
        ///             according to the Normal (or Gaussian) random number
        ///             distribution (mean: mean, stddev: standard deviation)\n
        ///     list("lognormal", m, s): Produces random numbers according
        ///             to a log-normal distribution (m: log-scale, s: shape)\n
        ///     list("chi_squared", n): Produces random positive
        ///             floating-point values, distributed according to the
        ///             Chi-squared probability density function
        ///             (n: degrees of freedom)\n
        ///     list("cauchy", a, b): Produces random positive
        ///             floating-point values, distributed according to the
        ///             Cauchy (Lorentz) probability density function
        ///             (a: location, s: scale)\n
        ///     list("fisher_f", m, n): Produces random positive
        ///             floating-point values, distributed according to the
        ///             f-probability density function (m: degrees of freedom,
        ///             n: degrees of freedom)\n
        ///     list("student_t", n): Produces random positive floating-point
        ///             values, distributed according to the student probability
        ///             density function (n: degrees of freedom)\n
        ///
        random(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        primitive_argument_type random0d(
            distribution_parameters_type&& params) const;
        primitive_argument_type random1d(
            std::size_t dim, distribution_parameters_type&& params) const;
        primitive_argument_type random2d(
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
            distribution_parameters_type&& params) const;
    };

    inline primitive create_random(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "random", std::move(operands), name, codename);
    }

    extern match_pattern_type const get_seed_match_data;
    extern match_pattern_type const set_seed_match_data;
}}}

#endif
