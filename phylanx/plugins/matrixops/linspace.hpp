//   Copyright (c) 2018 R. Tohid
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM)
#define PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///
    /// \brief Creates a linear space of evenly spaced numbers over the given interval.
    ///
    /// \param args Is a vector with exactly three elements (in order):
    ///    start: the first value of the sequence.
    ///    stop: the last value of the sequence. It will be ignored if the
    ///    number of samples is less than 2.
    ///    num_samples: number of samples in the sequence.
    ///
    class linspace
      : public primitive_component_base
      , public std::enable_shared_from_this<linspace>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        ///
        /// \brief Default constructor.
        ///
        linspace() = default;

        ///
        /// \brief Creates a linear space of evenly spaced numbers over the given interval.
        ///
        /// \param args Is a vector with exactly three elements (in order):
        ///
        /// start: the first value of the sequence.\n
        /// stop: the last value of the sequence. It will be ignored if the number of
        /// samples is less than 2.\n
        /// num_samples: number of samples in the sequence.
        ///
        /// \param name The name of the primitive
        /// \param codename The codename of the primitive

        linspace(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type linspace1d(primitive_argument_type&& start,
            primitive_argument_type&& end, std::int64_t nelements) const;

        template <typename T>
        primitive_argument_type linspace1d(
            T start, T end, std::int64_t nelements) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_linspace(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "linspace", std::move(operands), name, codename);
    }
}}}

#endif
