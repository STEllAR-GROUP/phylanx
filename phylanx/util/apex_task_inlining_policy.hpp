//  Copyright (c) 2019 M. A. H. Monil
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_APEX_TASK_INLINING_MAY_22_2019_0141PM)
#define PHYLANX_APEX_TASK_INLINING_MAY_22_2019_0141PM

#if defined(HPX_HAVE_APEX) && defined(PHYLANX_HAVE_TASK_INLINING_POLICY)

#include <hpx/config.hpp>
#include <hpx/runtime/config_entry.hpp>
#include <hpx/runtime/get_locality_id.hpp>
#include <hpx/runtime/get_num_localities.hpp>
#include <hpx/runtime/startup_function.hpp>
#include <hpx/util/thread_description.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <string>
#include "apex_api.hpp"
#include "apex_policies.hpp"


namespace phylanx { namespace util {

    struct apex_inlining_policy
    {
        apex_policy_handle* policy_handle_sample_counter;

        apex_tuning_request* request;
        int tuning_window;

        std::string counter_name_time_;
        std::string counter_name_count_;

        std::string policy_name_;
        std::string primitive_name_;
        std::size_t primitive_instance_number_;

        PHYLANX_API_EXPORT apex_policy_handle* policy_handle;
        PHYLANX_API_EXPORT apex_tuning_session_handle tuning_session_handle;

        PHYLANX_API_EXPORT apex_event_type custom_inlining_event;

        apex_event_type my_custom_event_2 = APEX_CUSTOM_EVENT_2;

        std::int64_t* eval_count_;
        std::int64_t* eval_duration_;
        std::int64_t* exec_threshold_;
        std::int64_t* execute_directly_;
        long threshold_ = 100000;

        std::mutex params_mutex;
        std::mutex count_mutex;
        std::mutex policy_mutex;

        void set_inlining_params_for_threshold()
        {
            std::shared_ptr<apex_param_long> chunk_threshold_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("threshold"));

            *exec_threshold_ = chunk_threshold_param->get_value();
        }

        void set_inlining_params_for_execute_directly()
        {
            std::shared_ptr<apex_param_long> execute_directly_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("execute_directly"));

            *execute_directly_ = execute_directly_param->get_value();
        }

        int direct_policy(const apex_context context)
        {
            if (!apex::has_session_converged(tuning_session_handle))
            {
                apex::custom_event(request->get_trigger(), nullptr);

                this->set_inlining_params_for_threshold();
            }

            return APEX_NOERROR;
        }

        apex_event_type apex_inlining_event(
            apex_event_type in_type = APEX_INVALID_EVENT)
        {
            static apex_event_type event_type;
            if (in_type != APEX_INVALID_EVENT)
            {
                event_type = in_type;
            }
            return event_type;
        }

        apex_inlining_policy(std::string primitive_name,
            std::int64_t& eval_count, std::int64_t& eval_duration,
            std::int64_t& exec_threshold, std::int64_t& execute_directly)

          : tuning_window(1)
          , policy_name_(primitive_name + "_policy")
          , primitive_name_(primitive_name)
          , eval_count_(&eval_count)
          , eval_duration_(&eval_duration)
          , exec_threshold_(&exec_threshold)
          , execute_directly_(&execute_directly)

        {
            phylanx::execution_tree::compiler::primitive_name_parts
                primitive_name_parts =
                    phylanx::execution_tree::compiler::parse_primitive_name(
                        primitive_name_);

            std::stringstream ss_time, ss_count;

            ss_time << "/phylanx{locality#" << hpx::get_locality_id();
            ss_time << "/total}/primitives/" << primitive_name_parts.primitive
                    << "/time/eval";
            counter_name_time_ = std::string(ss_time.str());

            ss_count << "/phylanx{locality#" << hpx::get_locality_id();
            ss_count << "/total}/primitives/" << primitive_name_parts.primitive
                     << "/count/eval";
            counter_name_count_ = std::string(ss_count.str());

            primitive_instance_number_ = primitive_name_parts.sequence_number;

            std::function<double(void)> metric = [=]() -> double {
                double result = 0;
                if (*eval_count_ > 0)
                    result = *eval_duration_ / *eval_count_;
                else
                    result = 0;

                return result;
            };

            request = new apex_tuning_request(policy_name_);
            request->set_metric(metric);
            request->set_strategy(apex_ah_tuning_strategy::PARALLEL_RANK_ORDER);

            //for threshold based policy
            request->add_param_long(
                "threshold", 200000, 200000, 1000000, 100000);
            request->set_trigger(apex::register_custom_event(policy_name_));
            tuning_session_handle = apex::setup_custom_tuning(*request);

            custom_inlining_event = apex_inlining_event(
                apex::register_custom_event("APEX direct vs nondirect event"));
            policy_handle = apex::register_policy(
                custom_inlining_event, [=](const apex_context context) -> int {
                    return this->direct_policy(context);
                });

            // To call the custom event include this header file
            // and use the example shown below in comments.
            // Do not uncomment the below line.
            // It is just an example how to use it elsewhere
            // apex::custom_event(policy_event, NULL);

            if (policy_handle == nullptr)
            {
                std::cerr << "Error registering policy!" << std::endl;
            }
        }

        apex_event_type return_apex_inlining_event()
        {
            return apex_inlining_event(custom_inlining_event);
        }
    };
}}
#endif    //HPX_HAVE_APEX && PHYLANX_HAVE_TASK_INLINING_POLICY
#endif    //PHYLANX_APEX_TASK_INLINING_MAY_22_2019_0141PM
