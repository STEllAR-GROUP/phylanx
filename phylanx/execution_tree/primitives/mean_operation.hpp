// Copyright (c) 2017 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PHYLANX_MEAN_OPERATION_HPP
#define PHYLANX_MEAN_OPERATION_HPP


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
            /// \brief Mean Operation Primitive
            ///
            /// This primitive computes the arithmatic mean along the specified axis.
            /// \param operands Vector of phylanx node data objects
            ///
            /// If used inside PhySL:
            ///
            ///      mean ( input, axis (optional) )
            ///
            ///          input : Scalar, Vector or a Matrix
            ///          axis     : The axis along which mean will be calculated
            ///
            class mean_operation
                    : public primitive_component_base
                            , public std::enable_shared_from_this<mean_operation>
            {
            protected:
                using arg_type = ir::node_data<double>;
                using args_type = std::vector<arg_type>;
                using storage0d_type = typename arg_type::storage0d_type;
                //using storage1d_type = typename arg_type::storage1d_type;
                //using storage2d_type = typename arg_type::storage2d_type;

                hpx::future<primitive_argument_type> eval(
                        std::vector<primitive_argument_type> const& operands,
                        std::vector<primitive_argument_type> const& args) const;

            public:
                static match_pattern_type const match_data;

                mean_operation() = default;

                mean_operation(std::vector<primitive_argument_type>&& operands,
                                      std::string const& name, std::string const& codename);

                hpx::future<primitive_argument_type> eval(
                        std::vector<primitive_argument_type> const& params) const override;

            private:
                //std::vector<int> create_list_row(
                  //      int start, int stop, int step, int array_length) const;
                primitive_argument_type mean0d(args_type&& args) const;
                //primitive_argument_type row_slicing1d(args_type&& args) const;
                //primitive_argument_type row_slicing2d(args_type&& args) const;
            };

            PHYLANX_EXPORT primitive create_mean_operation(
                    hpx::id_type const& locality,
            std::vector<primitive_argument_type>&& operands,
                    std::string const& name = "", std::string const& codename = "");
        }}}


#endif //PHYLANX_MEAN_OPERATION_HPP
