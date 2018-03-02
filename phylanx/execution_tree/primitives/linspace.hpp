// Copyright (c) 2018 R. Tohid
// 
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM)
#define PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
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
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using vector_type = blaze::DynamicVector<double>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;
        linspace(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type linspace1d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_linspace(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
