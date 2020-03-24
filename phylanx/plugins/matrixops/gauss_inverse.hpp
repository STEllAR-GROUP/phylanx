// Copyright (c) 2020 Rory Hector
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#if !defined(PHYLANX_PRIMITIVES_GAUSS_INVERSE_OPERATION_FEB_18_2020)
#define PHYLANX_PRIMITIVES_GAUSS_INVERSE_OPERATION_FEB_18_2020

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // Create a class for the node
    class matrix_GJE_Inverse
        : public primitive_component_base
        , public std::enable_shared_from_this<matrix_GJE_Inverse>
        {

        protected:

            // eval is a function called by phylanx when the value
            // of a node has been evaluated. This returns a future
            // to whatever the primitive calculates
            hpx::future<primitive_argument_type> eval(
                primitive_arguments_type const& operands,
                primitive_arguments_type const& args,
                eval_context ctx) const override;

        public:
            
            // Declare match_data type that appears in cpp file
            static match_pattern_type const match_data;
 
            // Needed for serialization and distributed computing,
            // can leave this alone just change name
            matrix_GJE_Inverse() = default;

            // Constructor
            // first string is the unique name of the primitive
            // instance, second arguement is the name of the
            // file used to generate that thing. It uses this
            // or the next thing based on whether value for the
            // primitive is provided at construction time or
            // evaluation time in more dynamic scenarios
            matrix_GJE_Inverse(primitive_arguments_type && operands,
                               std::string const& name,
                               std::string const& codename);

        private:
            // bool correctRows(int currentRow,
            // vector<vector<T>> &matrix, int n);
            primitive_argument_type gaussInverse2d(
                primitive_argument_type&& ops) const;
            template <typename T>
            primitive_argument_type gaussInverse2d(
                ir::node_data<T>&& ops) const;
        };

        // Creation function for the node
        // Takes arguements from before and also an HPX locality for
        // the component. What it does is calls a predefined function
        // create_primitive_component where you give it the name and
        // you're done.
        // Pretty much boilerplate, just change the name
        inline primitive create_matrix_GJE_Inverse(hpx::id_type const& locality,
                primitive_arguments_type&& operands,
                std::string const& name = "",
                std::string const& codename = "")
        {
            return create_primitive_component(
                 locality, "invGJE", std::move(operands), name, codename);
        }
}}}

#endif
