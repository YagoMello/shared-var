#ifndef DEBUG_TOOLS_HPP
#define DEBUG_TOOLS_HPP

/* Shared Variable Library
 * Debug tools
 * Author:  Yago T. de Mello
 * e-mail:  yago.t.mello@gmail.com
 * Version: 2.10.0 2022-07-02
 * License: Apache 2.0
 * C++20
 */

/*
Copyright 2022 Yago Teodoro de Mello
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// std::cout
#include <iostream>

// std::setw
#include <iomanip>

// std::string
#include <string>

// the lib
#include "shared_var.hpp"

// TODO:
// - Improve print_map to use a function pointer from "info"
//   created with the shared var, which converts the var to
//   a string if it has a conversion function or member fn.
//
// - Comments


namespace shared::debug {

template <typename T>
inline void print(T * ptr) {
    std::cout << *ptr;
}

template <>
inline void print<const char * const>(const char * const * ptr) {
    std::cout << std::string("\"") + *ptr + "\"";
}

template <>
inline void print<const std::string>(const std::string * ptr) {
    std::cout << "\"" + *ptr + "\"s";
}

template <typename T, typename Key>
inline void print_info(shared::info_t<Key> & info) {
    const T * ptr = shared::impl::info_to_data_ptr<T, Key>(info);
    
    if(ptr != nullptr) {
        shared::debug::print(ptr);
    }
    else {
        std::cout << "[nullptr]";
    }
}

template <typename Key>
inline int key_size(shared::map_type<Key> & map, const size_t max) {
    size_t largest = 0;
    
    for(auto & [key, info] : map) {
        largest = std::max(largest, key.size());
    }
    
    return int(std::min(largest, max));
}

template <typename T, typename Key>
bool is_type(const shared::info_t<Key> & info) {
    const auto & type_id = *info.type_id;
    
    return 
        (type_id == typeid(T)) || 
        (type_id == typeid(const T));
}

// for each element in the map, print the key, value, group id and address
template <typename Key>
inline void print_map(shared::map_type<Key> & map, const std::string & comment = "") {
    std::cout << comment << std::endl;
    std::cout << "map at " << &map << std::endl;
    for(auto & [key, info] : map) {
        std::cout << 
            std::setw(shared::debug::key_size(map, 12)) << 
            key << ": ";
        
        std::cout << std::setw(14);
        
        if(     shared::debug::is_type<   uint8_t>(info)) 
                shared::debug::print_info<uint8_t>(info);
                
        else if(shared::debug::is_type<   uint16_t>(info)) 
                shared::debug::print_info<uint16_t>(info);
        
        else if(shared::debug::is_type<   uint32_t>(info)) 
                shared::debug::print_info<uint32_t>(info);
        
        else if(shared::debug::is_type<   uint64_t>(info)) 
                shared::debug::print_info<uint64_t>(info);
        
        else if(shared::debug::is_type<   int8_t>(info)) 
                shared::debug::print_info<int8_t>(info);
        
        else if(shared::debug::is_type<   int16_t>(info)) 
                shared::debug::print_info<int16_t>(info);
        
        else if(shared::debug::is_type<   int32_t>(info)) 
                shared::debug::print_info<int32_t>(info);
        
        else if(shared::debug::is_type<   int64_t>(info)) 
                shared::debug::print_info<int64_t>(info);
        
        else if(shared::debug::is_type<   float>(info)) 
                shared::debug::print_info<float>(info);
        
        else if(shared::debug::is_type<   double>(info)) 
                shared::debug::print_info<double>(info);
        
        else if(shared::debug::is_type<   char *>(info)) 
                shared::debug::print_info<const char *>(info);
        
        else if(shared::debug::is_type<   const char *>(info)) 
                shared::debug::print_info<const char *>(info);
        
        else if(shared::debug::is_type<   std::string>(info)) 
                shared::debug::print_info<std::string>(info);
        
        else {
            std::cout << "[unknown type]";
        }
        
        std::cout << 
            " of group " << info.group_id << 
            " and type " << info.type_id->name() <<
            " at " << info.ptr << 
            std::endl;
    }
    std::cout << "========" << std::endl;
}

} // namespace shared::debug


#endif // DEBUG_TOOLS_HPP
