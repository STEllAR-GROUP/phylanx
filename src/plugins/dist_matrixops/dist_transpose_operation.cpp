// Copyright (c) 2017-2019 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/transpose_operation_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_transpose_operation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_transpose_operation::match_data =
    {
        hpx::util::make_tuple("transpose_d",
            std::vector<std::string>{"transpose_d(_1)", "transpose_d(_1, _2)"},
            &create_dist_transpose_operation,
            &execution_tree::create_primitive<dist_transpose_operation>, R"(
            arg, axes
            Args:

                arg (arr) : an array
                axes (optional, integer or a vector of integers) : By default,
                   reverse the dimensions, otherwise permute the axes according
                   to the values given.

            Returns:

            The transpose of `arg`. If axes are provided, it returns `arg` with
            its axes permuted.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_transpose_operation::dist_transpose_operation(
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool dist_transpose_operation::validate_axes(std::size_t a_dims,
        ir::node_data<std::int64_t>&& axes) const
    {
        if (a_dims == 1)
        {
            if (axes.num_dimensions() == 0)
            {
                if (axes.scalar() == 0 || axes.scalar() == -1)
                    return true;
            }
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                if (v.size() == 1)
                {
                    if (v[0] == 0 || v[0] == -1)
                        return true;
                }
            }
        }
        else if (a_dims == 2)
        {
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                for (auto& val : v)
                {
                    if (val < 0)
                        val += 2;
                    if (val != 0 && val != 1)
                        return false;
                }
                if (blaze::sum(v) == 1)
                    return true;
            }
        }
        else if (a_dims == 3)
        {
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                for (auto& val : v)
                {
                    if (val < 0)
                        val += 3;
                    if (val != 0 && val != 1 && val != 2)
                        return false;
                }
                if (blaze::sum(v) == 3)
                    return true;
            }
        }
        return false;    // axes are not allowed for 0-d arrays
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        execution_tree::annotation transpose_1d_annotation(
            execution_tree::localities_information&& localities,
            std::string const& name, std::string const& codename)
        {
            execution_tree::tiling_information_1d tile_info(
                localities.tiles_[localities.locality_.locality_id_],
                name, codename);
            tile_info.transpose();
            localities.annotation_.name_ += "_transposed";
            ++localities.annotation_.generation_;

            auto locality_ann = localities.locality_.as_annotation();
            return execution_tree::localities_annotation(locality_ann,
                tile_info.as_annotation(name, codename), localities.annotation_,
                name, codename);
        }
    }

    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose1d(
        execution_tree::primitive_argument_type&& arg) const
    {
        execution_tree::annotation loc_ann;
        if (!arg.get_annotation_if("localities", loc_ann, name_, codename_) &&
            !arg.find_annotation("localities", loc_ann, name_, codename_))
        {
            return common::transpose0d1d(std::move(arg));
        }

        // construct new tiling annotation
        arg.set_annotation(
            detail::transpose_1d_annotation(
                extract_localities_information(arg, name_, codename_),
                name_, codename_),
            name_, codename_);

        return std::move(arg);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        execution_tree::annotation transpose_2d_annotation(
            execution_tree::localities_information&& localities, bool transpose,
            std::string const& name, std::string const& codename)
        {
            execution_tree::tiling_information_2d tile_info(
                localities.tiles_[localities.locality_.locality_id_],
                name, codename);
            if (transpose)
            {
                tile_info.transpose();
            }

            localities.annotation_.name_ += "_transposed";
            ++localities.annotation_.generation_;

            auto locality_ann = localities.locality_.as_annotation();
            return execution_tree::localities_annotation(locality_ann,
                tile_info.as_annotation(name, codename), localities.annotation_,
                name, codename);
        }
    }

    template <typename T>
    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose2d(ir::node_data<T>&& arg,
        execution_tree::localities_information&& localities) const
    {
        // perform actual operation
        arg = blaze::trans(arg.matrix());

        execution_tree::primitive_argument_type result{std::move(arg)};

        // construct new tiling annotation
        result.set_annotation(
            detail::transpose_2d_annotation(
                std::move(localities), true, name_, codename_),
            name_, codename_);

        return result;
    }

    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose2d(
        execution_tree::primitive_argument_type&& arg) const
    {
        using namespace execution_tree;

        execution_tree::annotation localities;
        if (!arg.get_annotation_if("localities", localities, name_, codename_) &&
            !arg.find_annotation("localities", localities, name_, codename_))
        {
            return common::transpose2d(std::move(arg), name_, codename_);
        }

        auto localities_info =
            execution_tree::extract_localities_information(arg, name_, codename_);

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_int64:
            return transpose2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(
                extract_numeric_value(std::move(arg), name_, codename_),
                std::move(localities_info));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_transpose_operation::transpose2d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be a numeric data type"));
    }

    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose2d(
        execution_tree::primitive_argument_type&& arg,
        ir::node_data<std::int64_t>&& axes) const
    {
        using namespace execution_tree;

        auto v = axes.vector();
        for (auto& axis : v)
        {
            if (axis < 0)
                axis += 2;
        }

        execution_tree::annotation localities;
        if (!arg.get_annotation_if("localities", localities, name_, codename_) &&
            !arg.find_annotation("localities", localities, name_, codename_))
        {
            return common::transpose2d(std::move(arg), name_, codename_);
        }

        auto localities_info =
            extract_localities_information(arg, name_, codename_);

        if (v[0] == 0 && v[1] == 1)
        {
            arg.set_annotation(
                detail::transpose_2d_annotation(
                    std::move(localities_info), false, name_, codename_),
                name_, codename_);
            return std::move(arg);    // no-op
        }

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_int64:
            return transpose2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(
                extract_numeric_value(std::move(arg), name_, codename_),
                std::move(localities_info));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_transpose_operation::transpose2d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                "be a numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        execution_tree::annotation transpose_3d_annotation(
            execution_tree::localities_information&& localities,
            std::int64_t const* indices, std::size_t count,
            std::string const& name, std::string const& codename)
        {
            execution_tree::tiling_information_3d tile_info(
                localities.tiles_[localities.locality_.locality_id_],
                name, codename);
            tile_info.transpose(indices, count);

            localities.annotation_.name_ += "_transposed";
            ++localities.annotation_.generation_;

            auto locality_ann = localities.locality_.as_annotation();
            return execution_tree::localities_annotation(locality_ann,
                tile_info.as_annotation(name, codename), localities.annotation_,
                name, codename);
        }
    }

    template <typename T>
    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose3d(ir::node_data<T>&& arg,
        execution_tree::localities_information&& localities) const
    {
        static constexpr std::int64_t indices[] = {2, 1, 0};

        // perform actual operation
        arg = blaze::trans(arg.tensor());

        execution_tree::primitive_argument_type result{std::move(arg)};

        // construct new tiling annotation
        result.set_annotation(
            detail::transpose_3d_annotation(
                std::move(localities), indices, 3, name_, codename_),
            name_, codename_);

        return result;
    }

    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose3d(
        execution_tree::primitive_argument_type&& arg) const
    {
        using namespace execution_tree;

        execution_tree::annotation localities;
        if (!arg.get_annotation_if("localities", localities, name_, codename_) &&
            !arg.find_annotation("localities", localities, name_, codename_))
        {
            return common::transpose3d(std::move(arg), name_, codename_);
        }

        auto localities_info =
            extract_localities_information(arg, name_, codename_);

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose3d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_int64:
            return transpose3d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                std::move(localities_info));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose3d(
                extract_numeric_value(std::move(arg), name_, codename_),
                std::move(localities_info));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_transpose_operation::transpose3d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be numeric data type"));
    }

    template <typename T>
    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose3d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& axes,
        execution_tree::localities_information&& localities) const
    {
        // perform actual operation
        auto v = axes.vector();
        for (auto& axis : v)
        {
            if (axis < 0)
                axis += 3;
        }

        if (v[0] == 2 && v[1] == 1 && v[2] == 0)
        {
            arg = blaze::trans(arg.tensor());
        }
        else if (v[0] == 2 && v[1] == 0 && v[2] == 1)
        {
            arg = blaze::trans(arg.tensor(), {2, 0, 1});
        }
        else if (v[0] == 1 && v[1] == 2 && v[2] == 0)
        {
            arg = blaze::trans(arg.tensor(), {1, 2, 0});
        }
        else if (v[0] == 1 && v[1] == 0 && v[2] == 2)
        {
            arg = blaze::trans(arg.tensor(), {1, 0, 2});
        }
        else if (v[0] == 0 && v[1] == 2 && v[2] == 1)
        {
            arg = blaze::trans(arg.tensor(), {0, 2, 1});
        }

        execution_tree::primitive_argument_type result{std::move(arg)};

        // construct new tiling annotation
        result.set_annotation(
            detail::transpose_3d_annotation(
                std::move(localities), v.data(), v.size(), name_, codename_),
            name_, codename_);

        return result;
    }

    execution_tree::primitive_argument_type
    dist_transpose_operation::transpose3d(
        execution_tree::primitive_argument_type&& arg,
        ir::node_data<std::int64_t>&& axes) const
    {
        using namespace execution_tree;

        execution_tree::annotation localities;
        if (!arg.get_annotation_if("localities", localities, name_, codename_) &&
            !arg.find_annotation("localities", localities, name_, codename_))
        {
            return common::transpose3d(
                std::move(arg), std::move(axes), name_, codename_);
        }

        auto localities_info =
            extract_localities_information(arg, name_, codename_);

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose3d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                std::move(axes), std::move(localities_info));

        case node_data_type_int64:
            return transpose3d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                std::move(axes), std::move(localities_info));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose3d(
                extract_numeric_value(std::move(arg), name_, codename_),
                std::move(axes), std::move(localities_info));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_transpose_operation::transpose3d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                "be numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type>
    dist_transpose_operation::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_transpose_operation::dist_transpose_operation",
                generate_error_message(
                    "the dist_transpose_operation primitive requires"
                        "exactly one or two operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_transpose_operation::dist_transpose_operation",
                generate_error_message(
                    "the dist_transpose_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
            -> execution_tree::primitive_argument_type
            {
                using namespace execution_tree;

                auto a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (args.size() == 2 && valid(args[1]))
                {
                    // converting a range axes to a vector
                    if (is_list_operand_strict(args[1]))
                    {
                        ir::range axes = extract_list_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);

                        blaze::DynamicVector<std::int64_t> ops(axes.size());

                        auto&& list = axes.args();
                        for (std::size_t i = 0; i != axes.size(); ++i)
                        {
                            ops[i] = extract_scalar_integer_value_strict(
                                list[i], this_->name_, this_->codename_);
                        }

                        args[1] = primitive_argument_type{std::move(ops)};
                    }

                    if (this_->validate_axes(a_dims,
                            extract_integer_value_strict(
                                args[1], this_->name_, this_->codename_)))
                    {
                        switch (a_dims)
                        {
                        case 0:    // nothing to do
                            return std::move(args[0]);

                        case 1:
                            return this_->transpose1d(std::move(args[0]));

                        case 2:
                            return this_->transpose2d(std::move(args[0]),
                                extract_integer_value_strict(std::move(args[1]),
                                    this_->name_, this_->codename_));

                        case 3:
                            return this_->transpose3d(std::move(args[0]),
                                extract_integer_value_strict(std::move(args[1]),
                                    this_->name_, this_->codename_));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_transpose_operation::eval",
                                this_->generate_error_message(
                                    "left hand side operand has unsupported "
                                    "number of dimensions"));
                        }
                    }
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_transpose_operation::eval",
                        util::generate_error_message(
                            "At least one of the given axes is out of "
                            "bounds for the given array. Axes size should"
                            "be the same as array's number of dimensions."
                            "Having an n-d array each axis should be in "
                            "[-n, n-1]",
                            this_->name_, this_->codename_));
                }

                // no axes is given or axes=None
                switch (a_dims)
                {
                case 0:    // nothing to do
                    return std::move(args[0]);

                case 1:
                    return this_->transpose1d(std::move(args[0]));

                case 2:
                    return this_->transpose2d(std::move(args[0]));

                case 3:
                    return this_->transpose3d(std::move(args[0]));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_transpose_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
