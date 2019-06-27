//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/components.hpp>
#include <hpx/collectives.hpp>

#include <cstdint>
#include <mutex>
#include <string>

HPX_REGISTER_ALLTOALL(
    phylanx::execution_tree::annotation, meta_annotation_all_to_all);

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    struct locality_information
    {
        locality_information(ir::range const& data, std::string const& name,
            std::string const& codename)
        {
            if (data.size() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "locality_information::locality_information",
                    util::generate_error_message(
                        "unexpected number of 'locality' annotation data elements",
                        name, codename));
            }

            using execution_tree::extract_scalar_integer_value;

            auto it = data.begin();
            locality_id_ = static_cast<std::uint32_t>(
                extract_scalar_integer_value(*it, name, codename));
            num_localities_ = static_cast<std::uint32_t>(
                extract_scalar_integer_value(*++it, name, codename));
        }

        std::uint32_t locality_id_;
        std::uint32_t num_localities_;
    };

    namespace detail
    {
        locality_information extract_locality_information(
            execution_tree::annotation const& ann, std::string const& name,
            std::string const& codename)
        {
            if (ann.get_type() != "locality")
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "detail::extract_locality_information",
                    util::generate_error_message(
                        "'locality' annotation type not given",
                        name, codename));
            }
            return locality_information(ann.get_data(), name, codename);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    hpx::future<annotation> meta_annotation(annotation const& locality_ann,
        annotation&& ann, std::string const& name, std::string const& codename)
    {
        locality_information locality_info =
            detail::extract_locality_information(locality_ann, name, codename);

        hpx::future<std::vector<annotation>> f = hpx::all_to_all(name.c_str(),
            std::move(ann), locality_info.num_localities_, std::size_t(-1),
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
        std::string const& name, std::string const& codename)
    {
        return meta_annotation(locality_ann, std::move(ann), name, codename)
            .get();
    }
}}


