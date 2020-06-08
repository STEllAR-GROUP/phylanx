//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/collectives.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

HPX_REGISTER_ALLTOALL(
    phylanx::execution_tree::annotation, meta_annotation_all_to_all);

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <std::size_t N>
        bool validate_dimension(std::vector<tiling_information> const& tiles)
        {

            auto it_min = std::min_element(tiles.begin(), tiles.end(),
                [&](tiling_information const& v,
                    tiling_information const& smallest) {
                    if ((N < v.spans_.size() && v.spans_[N].is_valid()))
                        return v.spans_[N].start_ < smallest.spans_[N].start_;

                    return false;
                });

            // make sure the whole span starts from 0
            if (it_min->spans_[N].start_ != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::localities_annotation",
                "the span does not start from zero");
            }

            auto it_max = std::max_element(tiles.begin(), tiles.end(),
                [&](tiling_information const& largest,
                    tiling_information const& v)
                {
                    if (N < v.spans_.size() && v.spans_[N].is_valid())
                        return largest.spans_[N].stop_ < v.spans_[N].stop_;

                    return false;
                });

            // make sure there are no gaps between spans. overlaps are allowed
            std::int64_t end = it_max->spans_[N].stop_;
            for (auto const& tile : tiles)
            {
                std::int64_t stop_point = tile.spans_[N].stop_;
                if (stop_point != end &&
                    !std::any_of(tiles.begin(), tiles.end(),
                        [stop_point](tiling_information v)
                        {
                            return stop_point < v.spans_[N].stop_ &&
                                stop_point >= v.spans_[N].start_;
                        }))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::localities_annotation",
                        "the span is not consecutive");
                }
            }
            return true;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    hpx::future<annotation> meta_annotation(annotation const& locality_ann,
        annotation&& ann, std::string const& ann_name,
        std::string const& name, std::string const& codename)
    {
        locality_information locality_info =
            extract_locality_information(locality_ann, name, codename);

        hpx::future<std::vector<annotation>> f =
            hpx::all_to_all(("all_to_all_" + ann_name).c_str(), std::move(ann),
                locality_info.num_localities_, std::size_t(-1),
                locality_info.locality_id_);

        return f.then(hpx::launch::sync,
            [](hpx::future<std::vector<annotation>>&& f) -> annotation
            {
                std::vector<annotation> && data = f.get();

                primitive_arguments_type result;
                result.reserve(data.size() + 1);

                result.emplace_back("localities");

                for(auto&& d : data)
                {
                    result.emplace_back(std::move(d.get_range()));
                }

                return annotation{ir::range{std::move(result)}};
            });
    }

    annotation meta_annotation(hpx::launch::sync_policy,
        annotation const& locality_ann, annotation&& ann,
        std::string const& ann_name, std::string const& name,
        std::string const& codename)
    {
        return meta_annotation(
            locality_ann, std::move(ann), ann_name, name, codename).get();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Distributed annotations are expected to have a certain structure. This
    // structure is (mostly) created here.
    //
    // The overall structure is (assuming N localities, M is calling locality):
    //
    //      ("localities",
    //          ("locality", M, N)
    //          ("meta_0",
    //              (... supplied by the user on locality 0 ...)
    //          ),
    //          ("meta_1",
    //              (... supplied by the user on locality 1 ...)
    //          ),
    //          ...
    //          ("meta_<N-1>",
    //              (... supplied by the user on locality N-1 ...)
    //          )
    //      )
    //
    annotation localities_annotation(annotation& locality_ann,
        annotation&& ann, annotation_information const& ann_info,
        std::string const& name, std::string const& codename)
    {
        // defaults to locality_id and num_localities
        execution_tree::locality_information loc;

        // If the annotation contains information related to the
        // locality of the data we should perform an all_to_all
        // operation to collect the information about all connected
        // objects.
        if (locality_ann.get_type() == "locality")
        {
            std::size_t default_loc = loc.locality_id_;
            loc = execution_tree::extract_locality_information(
                locality_ann, name, codename);
            if (default_loc != loc.locality_id_)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "supplied locality differs from default");
            }

        }
        else
        {
            locality_ann = annotation{"locality",
                static_cast<std::int64_t>(loc.locality_id_),
                static_cast<std::int64_t>(loc.num_localities_)};
        }

        // wrap the user-supplied annotation into a meta-tag
        auto meta_locality_ann =
            annotation{
                hpx::util::format("meta_{}", loc.locality_id_),
                std::move(ann)
            };

        // communicate with all other localities to generate set of all meta
        // entries
        auto localities_ann = meta_annotation(hpx::launch::sync, locality_ann,
            std::move(meta_locality_ann), ann_info.generate_name(), name,
            codename);

        // now generate the overall annotation
        localities_ann.add_annotation(std::move(locality_ann), name, codename);

        // attach globally unique name to returned annotation
        localities_ann.add_annotation(ann_info.as_annotation(), name, codename);

        return localities_ann;
    }

    annotation localities_annotation(primitive_argument_type const& arg,
        annotation& locality_ann, annotation&& ann,
        annotation_information const& ann_info, std::string const& name,
        std::string const& codename)
    {
        // defaults to locality_id and num_localities
        execution_tree::locality_information loc;

        // If the annotation contains information related to the
        // locality of the data we should perform an all_to_all
        // operation to collect the information about all connected
        // objects.
        if (locality_ann.get_type() == "locality")
        {
            std::size_t default_loc = loc.locality_id_;
            loc = execution_tree::extract_locality_information(
                locality_ann, name, codename);
            if (default_loc != loc.locality_id_)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "supplied locality differs from default");
            }

        }
        else
        {
            locality_ann = annotation{"locality",
                static_cast<std::int64_t>(loc.locality_id_),
                static_cast<std::int64_t>(loc.num_localities_)};
        }

        // annotation should have the same dimension as array
        std::size_t ann_ndim = ann.get_data().size();
        if (ann_ndim != extract_numeric_value_dimension(arg, name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::localities_annotation",
                "dimension of supplied annotation is not the same as array's "
                "number of dimensions");
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> target_dims =
            extract_numeric_value_dimensions(arg, name, codename);
        switch (ann_ndim)
        {
        case 1:
        {
            std::size_t span_size;
            if (ann.has_key("columns"))
            {
                span_size = extract_span(ann, "columns", name, codename).size();
            }
            else if (ann.has_key("rows"))
            {
                span_size = extract_span(ann, "rows", name, codename).size();
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the vector annotation should have one of the row or "
                    "column spans");
            }

            if (span_size != target_dims[0])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the annotation span size differs from the vector's size");
            }
            break;
        }

        case 2:
        {
            std::size_t row_span_size =
                extract_span(ann, "rows", name, codename).size();
            std::size_t col_span_size =
                extract_span(ann, "columns", name, codename).size();
            if (target_dims[1] == 0)
            {
                // shape of [[]] is (1, 0) while it is shown with ("rows", 0, 0)
                // and ("columns", 0, 0) spans
                if (row_span_size != 0 || col_span_size != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::localities_annotation",
                        "annotations do not represent an ampty matrix");
                }
            }
            else if (row_span_size != target_dims[0] ||
                col_span_size != target_dims[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "at least in one of the row or column dimension, the "
                    "dimension does not comply with its corresponding in the "
                    "given matrix");
            }
            break;
        }

        case 0: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        case 4: HPX_FALLTHROUGH;
        default:

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::localities_annotation",
                "array's number of dimension is not supported");
        }

        // wrap the user-supplied annotation into a meta-tag
        auto meta_locality_ann =
            annotation{
                hpx::util::format("meta_{}", loc.locality_id_),
                std::move(ann)
            };

        // communicate with all other localities to generate set of all meta
        // entries
        auto localities_ann = meta_annotation(hpx::launch::sync, locality_ann,
            std::move(meta_locality_ann), ann_info.generate_name(), name,
            codename);

        std::size_t num_localities = loc.num_localities_;
        std::vector<tiling_information> tiles;
        tiles.reserve(num_localities);
        for (std::size_t i = 0; i != num_localities; ++i)
        {
            annotation meta;
            localities_ann.find(
                hpx::util::format("meta_{}", i), meta, name, codename);

            annotation tile;
            meta.find("tile", tile, name, codename);
            tiles.emplace_back(std::move(tile), name, codename);
        }

        if (tiles.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::localities_annotation",
                "there is no tile available");
        }

        switch (ann_ndim)
        {
        case 1:
        {
            bool type = false; // false:columns, true:rows
            annotation vector_span;
            bool flag = false;
            // making sure we don't have a vector with both row and column ann
            for (auto const& tile : tiles)
            {
                if (flag)
                {
                    if (!type)    // columns
                    {
                        if (tile.spans_.size() > 1 && tile.spans_[1].is_valid())
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::localities_"
                                "annotation",
                                "the vector can be either a row-vector or a "
                                "column-vector, not a mix of both");
                    }
                    else    // rows
                    {
                        if (tile.spans_[0].is_valid())
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::localities_"
                                "annotation",
                                "the vector can be either a row-vector or a "
                                "column-vector, not a mix of both");
                    }
                }
                else
                {
                    if (tile.spans_[0].is_valid())
                    {
                        type = false;
                        flag = true;
                    }
                    else if (tile.spans_.size() > 1 && tile.spans_[1].is_valid())
                    {
                        type = true;
                        flag = true;
                    }
                }
            }

            if (type == false && !detail::validate_dimension<0>(tiles))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the column vector does not contain a span of consecutive "
                    "integers starting from zero");
            }

            if (type == true && !detail::validate_dimension<1>(tiles))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the row vector does not contain a span of consecutive "
                    "integers starting from zero");
            }
            break;
        }

        case 2:
        {
            if (!detail::validate_dimension<0>(tiles))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the row dimension of matrix  does not contain a span of "
                    "consecutive integers starting from zero");
            }

            if (!detail::validate_dimension<1>(tiles))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::localities_annotation",
                    "the column dimension of matrix does not contain a span of "
                    "consecutive integers starting from zero");
            }
            break;
        }
        case 0: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        case 4: HPX_FALLTHROUGH;
        default:

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::localities_annotation",
                "array's number of dimension is not supported");
        }

        // now generate the overall annotation
        localities_ann.add_annotation(std::move(locality_ann), name, codename);

        // attach globally unique name to returned annotation
        localities_ann.add_annotation(ann_info.as_annotation(), name, codename);

        return localities_ann;
    }
}}


