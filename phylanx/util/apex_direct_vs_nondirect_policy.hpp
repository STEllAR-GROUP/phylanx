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
//#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

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
/*
namespace phylanx { namespace execution_tree
{
    namespace primitives
    {
	 struct primitive_component_base;
    }
}
}*/    

namespace phylanx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(PHYLANX_HAVE_DIRECT_VS_NONDIRECT_POLICY)

    struct apex_direct_vs_nondirect_policy
    {
        //std::shared_ptr<apex_policy_handle> policy_handle;
        //std::shared_ptr<apex_policy_handle> policy_handle_sample_counter;
        //std::shared_ptr<apex_tuning_request> request;
        apex_policy_handle* policy_handle_sample_counter;
        //phylanx::execution_tree::primitives::primitive_component_base* primitive_class_handle_;

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

	apex_event_type my_custom_event_2 = APEX_CUSTOM_EVENT_2;

        std::int64_t* eval_count_;
        std::int64_t* eval_duration_;
        std::int64_t* exec_threshold_;
        std::int64_t* execute_directly_;
	long threshold_ = 100000;


        std::mutex params_mutex;
        std::mutex count_mutex;
        std::mutex policy_mutex;

	void set_direct_vs_nondirect_params()
        {
            std::shared_ptr<apex_param_long> chunk_threshold_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("threshold" + primitive_name_));

            /*std::shared_ptr<apex_param_long> chunk_hysteresis_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("hysteresis" + primitive_name_)); */


            const int chunk_threshold = chunk_threshold_param->get_value();
            //const int chunk_hysteresis = chunk_hysteresis_param->get_value();

            /*apex::sample_value(
                "hpx.plugins.coalescing_message_handler.num_messages",
               parcel_count);
            apex::sample_value(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time); */

            std::cout << primitive_name_ + " policy is setting threshold: " 
		<< chunk_threshold << " hysteresis: "
		 /*<< chunk_hysteresis */ << "\n";

            hpx::set_config_entry(
                "phylanx.exec_time_threshold" + primitive_name_,
                chunk_threshold);

            /*px::set_config_entry(
                "phylanx.exec_time_hysteresis" + primitive_name_,
                chunk_hysteresis); */
 
       }

	void set_direct_vs_nondirect_params_for_threshold()
        {
            std::shared_ptr<apex_param_long> chunk_threshold_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("threshold")); 

            *exec_threshold_ = chunk_threshold_param->get_value();
            //*exec_threshold_ = threshold_;

            //std::cout << primitive_name_ + " policy is setting threshold: " 
		//<< *exec_threshold_ << "\n";
 
       }

	void set_direct_vs_nondirect_params_for_execute_directly()
        {
            std::shared_ptr<apex_param_long> execute_directly_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("execute_directly")); 

            *execute_directly_ = execute_directly_param->get_value();

            //std::cout << primitive_name_ + " policy is setting execute directly: " 
		//<< *execute_directly_ << "\n";
 
       }

        int direct_policy(const apex_context context)
        {
	    if (!apex::has_session_converged(tuning_session_handle)){
            	apex::custom_event(request->get_trigger(), NULL);
                
            	//apex::custom_event(my_custom_event_2, NULL);
            	
                //this->set_direct_vs_nondirect_params_directly_for_threshold();
                this->set_direct_vs_nondirect_params_for_execute_directly();
            }
            //else {
 		//apex::deregister_policy(policy_handle);
 		//std::cout << "As session is converged for: " + primitive_name_
			//+ " , policy is deregistered\n"; 

	    //}
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

        apex_direct_vs_nondirect_policy(std::string primitive_name
		, std::int64_t& eval_count
		, std::int64_t& eval_duration
		, std::int64_t& exec_threshold 
		, std::int64_t& execute_directly 
		) 
		//phylanx::execution_tree::primitives::primitive_component_base* primitive_class_handle)
          : tuning_window(1)
          , policy_name_(primitive_name + "_policy")
          , primitive_name_(primitive_name)
	  , eval_count_ (&eval_count)
	  , eval_duration_ (&eval_duration)
	  , exec_threshold_ (&exec_threshold)
	  , execute_directly_ (&execute_directly)
          //, primitive_class_handle_(primitive_class_handle)
        {

	    phylanx::execution_tree::compiler::primitive_name_parts primitive_name_parts = 
		phylanx::execution_tree::compiler::parse_primitive_name(primitive_name_);


	    //std::cout << "primitive name: " << primitive_name_parts.primitive << std::endl;
	    //std::cout << "Instance Number: " << primitive_name_parts.sequence_number << std::endl;
	    //std::cout << "Eval count: " << *eval_count_ << std::endl;

            std::stringstream ss_time, ss_count;

            ss_time << "/phylanx{locality#" << hpx::get_locality_id();
            ss_time << "/total}/primitives/" << primitive_name_parts.primitive << "/time/eval";
            counter_name_time_ = std::string(ss_time.str());
	    //std::cout << "Time counter name: " << counter_name_time_ << std::endl;

            ss_count << "/phylanx{locality#" << hpx::get_locality_id();
            ss_count << "/total}/primitives/" << primitive_name_parts.primitive << "/count/eval";
            counter_name_count_ = std::string(ss_count.str());
	    //std::cout << "Count counter name: " << counter_name_count_ << std::endl;

	    primitive_instance_number_ = primitive_name_parts.sequence_number;

            std::function<double(void)> metric = [=]() -> double {

		double result = 0;		
        	if ( *eval_count_ > 0 ) 
                	result = *eval_duration_ / 
				*eval_count_;
		else 
			result = 0;

                /*std::cout << primitive_name_ + " Counter current Value for: " << result << 
			" time: " << *eval_duration_  << " count: "
				<< *eval_count_ << "\n"; */

            return result;
            };

	    /*int num_inputs_2 = 1;
    	    long * inputs_2[1] = {0L};
    	    long mins_2[1] = {100000};
    	    long maxs_2[1] = {800000};
   	    long steps_2[1] = {100000};
    	    inputs_2[0] = &threshold_;
            my_custom_event_2 = apex::register_custom_event(policy_name_);
            tuning_session_handle = apex::setup_custom_tuning(metric, my_custom_event_2, 
		num_inputs_2, inputs_2, mins_2, maxs_2, steps_2); */

            request = new apex_tuning_request(policy_name_);
            request->set_metric(metric);
            request->set_strategy(apex_ah_tuning_strategy::EXHAUSTIVE);
            //request->set_strategy(apex_ah_tuning_strategy::PARALLEL_RANK_ORDER);

            //for threshold based policy
            //request->add_param_long("threshold", 100000, 100000, 1000000, 100000);
            //for direct vs nondirect execution policy
            request->add_param_long("execute_directly", 0, 0, 1, 1);
            //request->add_param_long("hysteresis" + primitive_name_, 50000, 50000, 200000, 50000);
            request->set_trigger(apex::register_custom_event(policy_name_));
            tuning_session_handle = apex::setup_custom_tuning(*request);




            custom_direct_vs_nondirect_event = 
		apex_direct_vs_nondirect_event(
		apex::register_custom_event("APEX direct vs nondirect event"));
            policy_handle = apex::register_policy(
		custom_direct_vs_nondirect_event, 
		[=](const apex_context context) -> int{
			return this->direct_policy(context);
			});


	    // To call the custom event include this header file and use this following commented line
	    // Do not uncomment the below line. It is just an example how to use it elsewhere
	    //apex::custom_event(policy_event, NULL);

            if (policy_handle == nullptr)
            {
                std::cerr << "Error registering policy!" << std::endl;
            }
            //else std::cout <<" Done registering policy: " << primitive_name_ << "\n";
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

