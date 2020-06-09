//  Copyright (c) 2020 Chris Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_LDA_April_27_2020_09013PM)
#define PHYLANX_LDA_April_27_2020_09013PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lda_trainer.hpp"

namespace phylanx { namespace execution_tree { namespace primitives
{
    class lda_trainer
      : public primitive_component_base
      , public std::enable_shared_from_this<lda_trainer>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

    public:
        static match_pattern_type const match_data;

        lda_trainer() = default;

        ///
        /// Creates a primitive executing the ALS algorithm on the given
        /// input data
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        lda_trainer(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

//        hpx::future<primitive_argument_type> eval(
//            primitive_arguments_type const& params) const override;

    protected:
        primitive_argument_type calculate_lda_trainer(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_lda_trainer(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "lda_trainer", std::move(operands), name, codename);
    }
}}}

#endif
