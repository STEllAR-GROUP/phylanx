//  Copyright (c) 2017 Christopher Taylor 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_TRANSDUCER_HPP)
#define PHYLANX_AST_TRANSDUCER_HPP

#include <tuple>
#include <functional>
#include <map>

template<typename InputSymbolType, typename EntryLogicType>
struct transition_table {

   using entry_logic = EntryLogicType;

   using entry_key = std::tuple<size_t, InputSymbolType>;
   using entry_value = std::tuple<size_t, entry_logic, double>;

   using state_table = std::map<entry_key, entry_value>;

   state_table transition_tbl;

   transition_table() {
   }
 
   void add(size_t s, size_t ns, InputSymbolType is, entry_logic logic, double w=1.0) {
       entry_key ik = std::make_tuple(s, is);
       entry_value iv = std::make_tuple(ns, logic, w);
       transition_tbl[ik] = iv;
   }

};

// nodes in the expression tree 
//
template<typename InputSymbolType, typename EntryLogicType>
struct transducer {

    using entry_logic = EntryLogicType;

    transition_table<InputSymbolType, entry_logic> tableau;

    size_t current_state;

    transducer(transition_table<InputSymbolType, EntryLogicType> table)
        : tableau(table)
        , current_state(0) {
    }

    bool consume(InputSymbolType & symbol) {
       using entry_key = typename transition_table<InputSymbolType, EntryLogicType>::entry_key;
       entry_key key = std::make_pair(current_state, symbol);

       auto end = tableau.transition_tbl.end();
       auto value = tableau.transition_tbl.find(key);

       // is the input symbol in the transition table
       //
       if(end != value) { 
           size_t next_state = std::get<0>(std::get<1>(*value));
           entry_logic fn = std::get<1>(std::get<1>(*value));
           //double weight = std::get<2>(std::get<1>(*value));

           // mutate input symbol
           fn(symbol);

           // update state of transducer
           current_state = next_state; 

           return true;
       }

       // exception
       //
       return false;
   }

};

// add a new token to the grammar for transducer creation
// same grammar except add ') :: double' 
// as a requirement for a transducer 
// if :: does not appear, append 1.0 as a value for that
// state
//
//transducer generate_transducer(char const *str);

//transducer generate_transducer(ast::expression const& expr);
//transducer compose_transducer(transducer &current, transducer &input);
//transducer compose_transducer(transducer &current, ast::expression &input);
//

#endif
