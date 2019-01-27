//  Copyright (c) 2019 M. A. H. Monil 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once // prevent multiple inclusions of this header file.

#include <hpx/config.hpp>
#include <hpx/runtime/get_locality_id.hpp>
#include <hpx/runtime/get_num_localities.hpp>
#include <hpx/runtime/startup_function.hpp>
#include <hpx/runtime/config_entry.hpp>
#include <hpx/util/thread_description.hpp>
#include <iostream>

#ifdef HPX_HAVE_APEX
#include "apex_api.hpp"
#include "apex_policies.hpp"
#include <memory>
#include <mutex>
#include <cstdint>
#include <string>
#include <stdio.h>
#endif

namespace phylanx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(PHYLANX_HAVE_DIRECT_VS_NONDIRECT_POLICY)

    struct apex_direct_vs_nondirect_policy
    {
        //std::shared_ptr<apex_policy_handle> policy_handle;
        //std::shared_ptr<apex_policy_handle> policy_handle_sample_counter;
        //std::shared_ptr<apex_tuning_request> request;
        apex_policy_handle* policy_handle_sample_counter;
        apex_tuning_request* request;
        int tuning_window;
        //int send_count;

        std::string counter_name_time;
        std::string counter_name_eval_count;
        std::string name;
        std::string primitive_name_;

        PHYLANX_API_EXPORT apex_policy_handle* policy_handle;
        PHYLANX_API_EXPORT apex_tuning_session_handle tuning_session_handle;

        //PHYLANX_API_EXPORT apex_direct_vs_nondirect_policy* instance;
        PHYLANX_API_EXPORT apex_event_type custom_direct_vs_nondirect_event;

        std::mutex params_mutex;
        std::mutex count_mutex;
        std::mutex policy_mutex;
/*
        void set_coalescing_params()
        {
            std::shared_ptr<apex_param_long> parcel_count_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("parcel_count"));

            std::shared_ptr<apex_param_long> buffer_time_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("buffer_time"));

            const int parcel_count = parcel_count_param->get_value();
            const int buffer_time = buffer_time_param->get_value();


            apex::sample_value(
                "hpx.plugins.coalescing_message_handler.num_messages",
                parcel_count);
            apex::sample_value(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time);
            std::cout<<"now setting coalescing values Parcel Count: " << parcel_count << "  Buffer_time: " << buffer_time << "\n";
            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.num_messages",
                parcel_count);
            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time);
        }
*/
/*
        int direct_policy(const apex_context context)
        {
	    if (!apex::has_session_converged(tuning_session_handle)){
	    	apex_profile* profile = apex::get_profile(instance->counter_name);
            	//apex_profile* profile = apex::get_profile();
            	if (profile == nullptr) {
                	printf("Nullpointer coutername, %s  send count %d\n ",instance->counter_name.c_str(), instance->send_count);
                	fflush(stdout);
            	}
            	if (profile != nullptr && profile->calls >= instance->tuning_window)
            	{
                	apex::custom_event(instance->request->get_trigger(), NULL);
                	instance->set_coalescing_params();
                	apex::reset(instance->counter_name);
            	}
            }
            else {
 		printf("As session is converged, policy is deregistered\n"); fflush(stdout);
 		apex::deregister_policy(policy_handle);

	    }
            return APEX_NOERROR;
	
        }
*/

/*
        static int count_based_policy(const apex_context context)
        {
            if (instance->send_count < 50000)
            {
                if(instance->count_mutex.try_lock())
                {
                    instance->send_count++;
                    instance->count_mutex.unlock();
                }
                return APEX_NOERROR;
            }
            else
            {
                if(instance->policy_mutex.try_lock())
                {
                    instance->send_count=0;
                    instance->direct_policy(context);
                    instance->policy_mutex.unlock();
                }
                return APEX_NOERROR;
            }
        }

*/

        int direct_policy(const apex_context context)
        {

            //apex::custom_event(instance->request->get_trigger(), NULL);
            //instance->set_coalescing_params();
            //apex::reset(instance->counter_name);

	    //std::string const& name = hpx::util::get<1>(primitive_name_);
	    //std::string const& instance_number = hpx::util::get<2>(primitive_name_);

	    //std::cout << "called policy for:" << name << " " << instance_number << std::endl; 
            std::string const count_pc_name(
                "/phylanx{locality#0/total}/primitives/store/count/eval");
            hpx::performance_counters::performance_counter count_pc(
                count_pc_name);

            std::string const time_pc_name(
                "/phylanx{locality#0/total}/primitives/store/time/eval");
	    hpx::performance_counters::performance_counter time_pc(time_pc_name);

            auto time_values =
                time_pc.get_counter_values_array(hpx::launch::sync, false); 

            auto count_values =
                count_pc.get_counter_values_array(hpx::launch::sync, false); 


 	    //auto const info = count_pc.get_info(hpx::launch::sync);
 	    //auto const info = count_pc.get_info(hpx::launch::sync);
 	    //
 	    
            for (std::size_t i = 0; i != time_values.values_.size(); ++i)
            {
                std::cout << "Time values: " << time_values.values_[i] << std::endl; 
	    } 

            for (std::size_t i = 0; i != count_values.values_.size(); ++i)
            {
                std::cout << "Count Values: " << count_values.values_[i] << std::endl; 
	    } 


            return APEX_NOERROR;
	
        }


        apex_event_type apex_direct_vs_nondirect_event(
            apex_event_type in_type = APEX_INVALID_EVENT)
        {
            static apex_event_type event_type;
             if (in_type != APEX_INVALID_EVENT)
             {
                 event_type = in_type;
             }
             return event_type;
        }

        apex_direct_vs_nondirect_policy(std::string primitive_name)
          : tuning_window(1)
          , name(primitive_name + "policy")
          , primitive_name_(primitive_name)
        {
            std::stringstream ss;
//"/phylanx{locality#0/total}/primitives/" + name + "/time/eval"
            ss << "/phylanx{locality#" << hpx::get_locality_id();
            ss << "/total}/primitives/store/time/eval";
            //ss << "/total}/primitives" + primitive_name_ + "/time/eval";
            counter_name_time = std::string(ss.str());
	    std::cout << counter_name_time << std::endl;
            //policy_handle_sample_counter = apex::sample_runtime_counter(500000, counter_name_time);

/*
            std::function<double(void)> metric = [=]() -> double {
                apex_profile* profile = apex::get_profile(counter_name);
                if (profile == nullptr || profile->calls == 0)
                {
                    return 0.0;
                }
                double result = profile->accumulated / profile->calls;
                std::cout << "Counter current Value: " << result << "\n";
                return result;
            };

            request = new apex_tuning_request(name);
            request->set_metric(metric);
            //request->set_strategy(apex_ah_tuning_strategy::EXHAUSTIVE);
            request->set_strategy(apex_ah_tuning_strategy::PARALLEL_RANK_ORDER);
            //request->add_param_long("parcel_count", 20, 20, 26, 2);
            //request->add_param_long("parcel_count", start, min, max, step);
            request->add_param_long("direct_vs_nondirect_threshold", 100000, 100000, 500000, 50000);
            //request->add_param_long("direct_vs_nondirect_hysteresis", 1000, 1000, 5000, 1000);
            request->set_trigger(apex::register_custom_event(name));
            tuning_session_handle = apex::setup_custom_tuning(*request);

*/

	    // To register a periodic policy: uncomment the following line
            //policy_handle = apex::register_periodic_policy(500000, direct_policy);

	    // To register a custom event : uncomment the following two line
            custom_direct_vs_nondirect_event = 
		apex_direct_vs_nondirect_event(
		apex::register_custom_event("APEX direct vs nondirect event"));
            policy_handle = apex::register_policy(
		custom_direct_vs_nondirect_event, 
		[&](const apex_context context) -> int{
			return this->direct_policy(context);
			});


	    // To call the custom event include this header file and use this following commented line
	    // Do not uncomment the below line. It is just an example how to use it elsewhere
	    //apex::custom_event(hpx::util::apex_parcel_coalescing_policy::return_apex_parcel_coalescing_event(), NULL);

            if (policy_handle == nullptr)
            {
                std::cerr << "Error registering policy!" << std::endl;
            }
            else std::cout<<" Done registering policy.\n";
        }

        apex_event_type return_apex_direct_vs_nondirect_event()
        {
             return apex_direct_vs_nondirect_event(custom_direct_vs_nondirect_event);
        }


        void initialize()
        {
            /* if (instance == nullptr)
            {
                instance = new apex_direct_vs_nondirect_policy();
            } */
        }
        void finalize()
        {
            /*if (instance != nullptr)
            {
                delete instance;
                instance = nullptr;
            } */
        }
    };
#endif //HPX_HAVE_APEX && HPX_HAVE_DIRECT_VS_NONDIRECT_POLICY
}}

