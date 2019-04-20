// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_KERAS_SUPPORT_CTC_DECODE_OPERATION)
#define PHYLANX_PLUGINS_KERAS_SUPPORT_CTC_DECODE_OPERATION

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
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
    /// \brief Returns the result of Connectionist temporal classification applied to a
    ///  squence.
    /// \param y_pred The scalar, vector, matrix, or tensor to perform ctc_decode over
    /// \param input_length
    /// \param greedy boolean, if True performs best-path search otherwise beam-search
    /// \param beam_width Integer, if greedy is False specifies the width of the beam.
    /// \param top_paths Integer, if greedy is False specifies the number of top paths
    ///  desired.
    class ctc_decode_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<ctc_decode_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        ctc_decode_operation() = default;

        ctc_decode_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_ctc_decode_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "ctc_decode", std::move(operands), name, codename);
    }
}}}

#endif
#endif
