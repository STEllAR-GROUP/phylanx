// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/clip.hpp>
#include <phylanx/util/detail/numeric_limits_min.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const clip::match_data = {hpx::util::make_tuple("clip",
        std::vector<std::string>{"clip(_1, _2, _3)"}, &create_clip,
        &create_primitive<clip>, R"(
            a, axis
            Args:

                a (array_like) : array containing elements to clip
                a_min : Minimum value, it may be scalar or array_like or None
                a_max : Maximum valus, it may be scalar or array_like or None

            Returns:

            An array with the elements of a, but where values < a_min are replaced with
            a_min, and those > a_max with a_max."
            )")};

    ///////////////////////////////////////////////////////////////////////////
    clip::clip(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    primitive_argument_type clip::clip0d(ir::node_data<T>&& arg,
        ir::node_data<T>&& min, ir::node_data<T>&& max) const
    {
        auto result = (blaze::max)(
            min.scalar(), (blaze::min)(max.scalar(), arg.scalar()));

        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type clip::clip1d(ir::node_data<T>&& arg,
        ir::node_data<T>&& min, ir::node_data<T>&& max) const
    {
        auto result = (blaze::max)(
            min.vector(), (blaze::min)(max.vector(), arg.vector()));

        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type clip::clip2d(ir::node_data<T>&& arg,
        ir::node_data<T>&& min, ir::node_data<T>&& max) const
    {
        auto result = (blaze::max)(
            min.matrix(), (blaze::min)(max.matrix(), arg.matrix()));

        return primitive_argument_type{ir::node_data<T>{result}};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type clip::clip3d(ir::node_data<T>&& arg,
        ir::node_data<T>&& min, ir::node_data<T>&& max) const
    {
        auto result = (blaze::max)(
            min.tensor(), (blaze::min)(max.tensor(), arg.tensor()));

        return primitive_argument_type{ir::node_data<T>{result}};
    }
#endif

    template <typename T>
    primitive_argument_type clip::clip_helper(
        primitive_arguments_type&& args) const
    {
        if (!valid(args[1]))
            args[1] = primitive_argument_type{
                phylanx::util::detail::numeric_limits_min<T>()};

        if (!valid(args[2]))
            args[2] = primitive_argument_type{(std::numeric_limits<T>::max)()};

        std::size_t size = extract_largest_dimension(args);
        auto sizes = extract_largest_dimensions(
            name_, codename_, args[0], args[1], args[2]);

        switch (size)
        {
        case 0:
            return clip0d(
                extract_value_scalar<T>(std::move(args[0]), name_, codename_),
                extract_value_scalar<T>(std::move(args[1]), name_, codename_),
                extract_value_scalar<T>(std::move(args[2]), name_, codename_));
        case 1:
            return clip1d(extract_value_vector<T>(
                              std::move(args[0]), sizes[0], name_, codename_),
                extract_value_vector<T>(
                    std::move(args[1]), sizes[0], name_, codename_),
                extract_value_vector<T>(
                    std::move(args[2]), sizes[0], name_, codename_));
        case 2:
            return clip2d(extract_value_matrix<T>(std::move(args[0]), sizes[0],
                              sizes[1], name_, codename_),
                extract_value_matrix<T>(
                    std::move(args[1]), sizes[0], sizes[1], name_, codename_),
                extract_value_matrix<T>(
                    std::move(args[2]), sizes[0], sizes[1], name_, codename_));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return clip3d(extract_value_tensor<T>(std::move(args[0]), sizes[0],
                              sizes[1], sizes[2], name_, codename_),
                extract_value_tensor<T>(std::move(args[1]), sizes[0], sizes[1],
                    sizes[2], name_, codename_),
                extract_value_tensor<T>(std::move(args[2]), sizes[0], sizes[1],
                    sizes[2], name_, codename_));
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "clip::clip_helper",
            generate_error_message(
                "the clip primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> clip::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "clip::eval",
                generate_error_message(
                    "the clip primitive requires exactly three operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "clip::eval",
                generate_error_message("the clip primitive requires that the "
                    "first argument is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                switch (extract_common_type(args))
                {
                case node_data_type_int64:
                    return this_->clip_helper<std::int64_t>(std::move(args));

                case node_data_type_bool:
                    return this_->clip_helper<std::uint8_t>(std::move(args));

                case node_data_type_unknown: HPX_FALLTHROUGH;
                case node_data_type_double:
                    return this_->clip_helper<double>(std::move(args));

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "clip::eval",
                    this_->generate_error_message(
                        "the clip primitive requires for all arguments to "
                        "be numeric data types"));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
