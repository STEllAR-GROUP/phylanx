//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>
#include <hpx/collectives.hpp>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <utility>

HPX_REGISTER_ALLTOALL(
    phylanx::execution_tree::annotation, meta_annotation_all_to_all);

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
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
}}


