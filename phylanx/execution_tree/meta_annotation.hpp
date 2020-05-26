//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_META_ANNOTATION_JUN_21_2019_0125PM)
#define PHYLANX_PRIMITIVES_META_ANNOTATION_JUN_21_2019_0125PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>

#include <hpx/futures/future.hpp>

#include <string>

namespace phylanx { namespace execution_tree
{
    PHYLANX_EXPORT hpx::future<annotation> meta_annotation(
        annotation const& locality_ann, annotation&& ann,
        std::string const& ann_name, std::string const& name,
        std::string const& codename);

    PHYLANX_EXPORT annotation meta_annotation(hpx::launch::sync_policy,
        annotation const& locality_ann, annotation&& ann,
        std::string const& ann_name, std::string const& name,
        std::string const& codename);

    PHYLANX_EXPORT annotation localities_annotation(annotation& locality_ann,
        annotation&& ann, annotation_information const& ann_info,
        std::string const& name, std::string const& codename);
}}

#endif

