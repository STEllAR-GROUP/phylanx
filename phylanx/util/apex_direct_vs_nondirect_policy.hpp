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

        std::string counter_name_time_;
        std::string counter_name_count_;

        std::string policy_name_;
        std::string primitive_name_;
        std::size_t primitive_instance_number_;

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


        int direct_policy(const apex_context context)
        {

            apex::custom_event(request->get_trigger(), NULL);

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
          , policy_name_(primitive_name + "_policy")
          , primitive_name_(primitive_name)
        {

	    phylanx::execution_tree::compiler::primitive_name_parts primitive_name_parts = 
		phylanx::execution_tree::compiler::parse_primitive_name(primitive_name_);


	    std::cout << "primitive name: " << primitive_name_parts.primitive << std::endl;
	    std::cout << "Instance Number: " << primitive_name_parts.sequence_number << std::endl;

            std::stringstream ss_time, ss_count;

            ss_time << "/phylanx{locality#" << hpx::get_locality_id();
            ss_time << "/total}/primitives/" << primitive_name_parts.primitive << "/time/eval";
            counter_name_time_ = std::string(ss_time.str());
	    std::cout << "Time counter name: " << counter_name_time_ << std::endl;

            ss_count << "/phylanx{locality#" << hpx::get_locality_id();
            ss_count << "/total}/primitives/" << primitive_name_parts.primitive << "/count/eval";
            counter_name_count_ = std::string(ss_count.str());
	    std::cout << "Count counter name: " << counter_name_count_ << std::endl;

	    primitive_instance_number_ = primitive_name_parts.sequence_number;

            std::function<double(void)> metric = [=]() -> double {
		
           	std::string const time_pc_name(counter_name_time_);
	    	hpx::performance_counters::performance_counter time_pc(
			time_pc_name);

                std::string const count_pc_name(counter_name_count_);
                hpx::performance_counters::performance_counter count_pc(
          	      count_pc_name);

 
            	auto time_values =
                	time_pc.get_counter_values_array(hpx::launch::sync, false); 

            	auto count_values =
                	count_pc.get_counter_values_array(hpx::launch::sync, false); 

 	    
            	/*for (std::size_t i = 0; i != time_values.values_.size(); ++i)
            	{
                	//std::cout << "Time values: " << time_values.values_[i] << std::endl; 
	    	}*/ 
		
		double count_times = count_values.values_[primitive_instance_number_];
		double result = 0;

		if ( count_times > 0 ) 
                	result = time_values.values_[primitive_instance_number_] / 
				count_values.values_[primitive_instance_number_];
		else 
			result = 0;

                std::cout << "Counter current Value: " << result << "\n";
                return result;
            };

            request = new apex_tuning_request(policy_name_);
            request->set_metric(metric);
            //request->set_strategy(apex_ah_tuning_strategy::EXHAUSTIVE);
            request->set_strategy(apex_ah_tuning_strategy::PARALLEL_RANK_ORDER);
            //request->add_param_long("parcel_count", 20, 20, 26, 2);
            //request->add_param_long("parcel_count", start, min, max, step);
            request->add_param_long("chunk_threshold" + primitive_name_, 100000, 100000, 500000, 50000);
            //request->add_param_long("direct_vs_nondirect_hysteresis", 1000, 1000, 5000, 1000);
            request->set_trigger(apex::register_custom_event(policy_name_));
            tuning_session_handle = apex::setup_custom_tuning(*request);



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

