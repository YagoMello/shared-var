#ifndef TYPES_HPP
#define TYPES_HPP

/* Shared Variable Library
 * Types
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

// default lib includes and definitions
#include "includes.hpp"


// The lib namespace
namespace shared {

// Contains the shared var info
template <typename Key>
struct info_t {
    // The "shared var name" type, defaults to std::string but could be anything
    // accepted by std::map.
    using key_type = Key;
    using allocator_type = std::shared_ptr<void> (*)(void * ptr_to_value);
    using copier_type = void (*)(void * ptr_to_dest, void * ptr_to_src);
    
    std::shared_ptr<void> ptr; // The shared variable (pointer (and type erased))
    key_type group_id;         // The group where the variable is shared
    key_type key;              // This variable name
    const std::type_info * type_id; // The shared variable type (RTTI), used for type checking
    allocator_type allocator;  // Allocates memory when called
    copier_type copier;        // Copies the value of another var
    std::set<key_type> refs;   // Variables connected to this var
    std::set<void **> pointers_to_var; // Vars with direct access to the data pointer
};

// Stores information about the shared variables 
// and associates variable names and data.
// Added in 2.9.0
template <typename Key>
class var_map_t {
public:
    // The underlying map type
    using storage_type = std::map<Key, shared::info_t<Key>>;
    
// ==== std::map types ====
    
    using iterator       = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;
    
    using size_type      = storage_type::size_type;
    
// ==== custom constructors and assignment operators ====
    
    // Allows creation of empty maps
    var_map_t() = default;
    
    // Copies should be explicit to prevent unintended behaviour
    var_map_t(const var_map_t &) = delete;
    
    // Moving would break vars with pointers to this map
    var_map_t(var_map_t && var_map) = delete;
    
    // Copies should be explicit to prevent unintended behaviour
    var_map_t & operator =(const var_map_t &) = delete;
    
    // Moving would break vars with pointers to this map
    var_map_t & operator =(var_map_t && var_map) = delete;
    
// ==== std::map functions ====
    
    // Same as std::map::clear
    void clear() noexcept {
        map_.clear();
    }
    
    // Same as std::map::contains
    template <typename K>
    bool contains(const K & key) {
        return map_.contains(key);
    }
    
    // Same as std::map::empty
    [[nodiscard]] bool empty() const noexcept {
        return map_.empty();
    }
    
    // Same as std::map::erase
    template <typename K>
    size_type erase(K && key) {
        return map_.erase(key);
    }
    
    // Same as std::map::find
    template <typename K>
    iterator find(const K & key) {
        return map_.find(key);
    }
    
    // Same as std::map::find
    template <typename K>
    const_iterator find(const K & key) const {
        return map_.find(key);
    }
    
    // Same as std::map::size
    size_type size() const noexcept {
        return map_.size();
    }
    
    // Same as std::map::operator[]
    template <typename K>
    shared::info_t<Key> & operator [](K && key) {
        return map_[key];
    }
    
    // Same as std::map::begin
    iterator begin() noexcept {
        return map_.begin();
    }
    
    // Same as std::map::end
    iterator end() noexcept {
        return map_.end();
    }
    
    // Same as std::map::begin
    const_iterator begin() const noexcept {
        return map_.begin();
    }
    
    // Same as std::map::end
    const_iterator end() const noexcept {
        return map_.end();
    }
    
    // Same as std::map::cbegin
    const_iterator cbegin() const noexcept {
        return map_.cbegin();
    }
    
    // Same as std::map::cend
    const_iterator cend() const noexcept {
        return map_.cend();
    }
    
private:
    // The real map
    storage_type map_;
};

// The shared-variables container
template <typename Key = std::string>
using map_type = shared::var_map_t<Key>;

// A type that can be stored and pointed
// to by a void *, so no function references
template <typename T>
concept storable = 
    not std::is_reference<T>::value && 
    not std::is_function<T>::value;

} // namespace shared


#endif // TYPES_HPP
