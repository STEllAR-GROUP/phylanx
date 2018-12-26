// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_SHUFFLE_OPERATION_HPP)
#define PHYLANX_SHUFFLE_OPERATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Randomly shuffles the elements of a PhySL list or the rows of a
    /// PhySL matrix in place. Unlike [NumPy shuffle](
    /// https://docs.scipy.org/doc/numpy-1.14.0/reference/generated/numpy.random
    /// .shuffle.html)
    /// it returns the shuffled matrix or list
    ///
    /// \returns The PhySL matrix or PhySL list that was shuffled in place
    ///
    /// \param args Is either a matrix or list of any values to be
    /// randomly shuffled
    class shuffle_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<shuffle_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        shuffle_operation() = default;

        shuffle_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type shuffle_1d(primitive_argument_type&& arg) const;
        primitive_argument_type shuffle_2d(primitive_argument_type&& arg) const;

        template <typename T>
        primitive_argument_type shuffle_1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type shuffle_2d(ir::node_data<T>&& arg) const;
    };

    inline primitive create_shuffle_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "shuffle", std::move(operands), name, codename);
    }
}}}

#endif
