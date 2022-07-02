#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

/* Shared Variable Library
 * Functions
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

// main lib types
#include "types.hpp"

// internal functions
#include "impl.hpp"


// The lib namespace
namespace shared {

// Creates a new shared var, stored in the map "mp".
// A pointer to the shared var info is returned.
template <shared::storable T, typename Key, std::convertible_to<T> Value = T>
inline shared::info_t<Key> * create(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T(),
    const bool overwrite = false
) {
    // Check if the var already exists
    auto it = mp.find(key);
    
    if(it == mp.end()) {
        // The var doesnt exist, lets create it
        
        // The var info contains the shared var and
        // and the control variables
        shared::info_t<Key> info;
        
        // Allocate the memory
        auto data_ptr = std::make_shared<T>(std::forward<T>(default_value));
        
        // And assign the other parameters
        
        // The key must be unique, so it can be used as group id
        info.group_id  = key;
        
        // typeid() returns a reference to an object with
        // static storage durration, allowing us to store the pointer
        // without bothering with the object lifetime
        info.type_id   = &typeid(T);
        
        info.key       = key;
        info.ptr       = data_ptr;
        info.allocator = shared::impl::default_allocator<T>;
        info.copier    = shared::impl::default_copier<T>;
        
        // Save the new var in the map of vars
        mp[key] = info;
        
        // And return the address
        return &mp[key];
    }
    else {
        // The var exists, but may not be of the same type
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        // We cannot return info pointing to another type,
        // when accessing woult lead to UB
        if(shared::impl::are_types_equal<T>(info)) {
            // Types are equal, return existing info
            return &info;
        }
        else if(overwrite) {
            // Overwrite is set, and types are not equal
            // Then we should delete the current var
            // And create the new one on its place
            shared::impl::remove(mp, info);
            return shared::create<T>(mp, key, std::forward<T>(default_value), overwrite);
        }
        else {
            // Types are different and can't overwrite, crap.
            return nullptr;
        }
    }
}

// Copies the src var in the src map to the dest var in the dest map.
// Creates a new dest var if needed.
template <typename Key>
inline shared::info_t<Key> * copy(
    shared::map_type<Key> & mp_src, 
    shared::map_type<Key> & mp_dest, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    // Check if the original var exists
    auto it_src = mp_src.find(key_src);
    
    // Check if the new var exists
    auto it_dest = mp_dest.find(key_dest);
    
    if(it_src != mp_src.end() && it_dest == mp_dest.end()) {
        // The src exists and te dest doesn't
        
        // The src is just a ref
        shared::info_t<Key> & info_src = shared::impl::iter_to_info<Key>(it_src);
        
        // The dest is a new info to create a new var
        shared::info_t<Key> info_dest;
        
        // Copying some infrmation
        info_dest.type_id   = info_src.type_id;
        info_dest.allocator = info_src.allocator;
        info_dest.copier    = info_src.copier;
        
        // Setting some values that are different from the src
        info_dest.key = key_dest;
        info_dest.group_id = key_dest;
        
        // Allocate the memory and copy the value
        shared::impl::allocate_and_notify_subscribers(info_dest, info_src.ptr.get());
        
        // Save the new var in the map of vars
        mp_dest[key_dest] = info_dest;
        
        // And return the address
        return &mp_dest[key_dest];
    }
    else if(it_src != mp_src.end() && it_dest != mp_dest.end()) {
        // The var exists, but may not be of the same type
        shared::info_t<Key> & info_src  = shared::impl::iter_to_info<Key>(it_src);
        shared::info_t<Key> & info_dest = shared::impl::iter_to_info<Key>(it_dest);
        
        // We cannot return info pointing to another type,
        // when accessing woult lead to UB
        if(shared::impl::are_types_equal(info_src, info_dest)) {
            // Types are equal, copying values and returning existing info
            void * src_ptr  = info_src.ptr.get();
            void * dest_ptr = info_dest.ptr.get();
            
            info_src.copier(dest_ptr, src_ptr);
            return &info_dest;
        }
        else if(overwrite) {
            // Overwrite is set, and types are not equal
            // Then we should delete the current var
            // And try to copy the value again
            shared::impl::remove(mp_dest, info_dest);
            return shared::copy(mp_src, mp_dest, key_src, key_dest, overwrite);
        }
        else {
            // Types are different and can't overwrite, crap.
            return nullptr;
        }
    }
    else {
        // Couldn't find the src var, can't do anything.
        return nullptr;
    }
}

// Copies the src var to the dest var.
// Creates a new dest var if needed.
template <typename Key>
inline shared::info_t<Key> * copy(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    return shared::copy(mp, mp, key_src, key_dest, overwrite);
}

// Describes which path shared::bind took when binding variables
enum bind_t : uint_fast8_t {
    BIND_FAILED_NONEXISTENT_VAR = 0,
    BIND_FAILED_DIFFERENT_TYPES = 1,
    BIND_CREATED_LHS            = 2,
    BIND_CREATED_RHS            = 3,
    BIND_PROPAGATED_LHS_GROUP   = 4
};

// Connects two variables, making them share the same memory
template <typename Key>
inline shared::bind_t bind(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key_L, 
    const std::type_identity_t<Key> & key_R
) {
    // search for nodes
    auto it_key_L = mp.find(key_L);
    auto it_key_R = mp.find(key_R);
    
    if(it_key_L == mp.end() && it_key_R == mp.end()) {
        // if both nodes doesn't exist, we don't have
        // enough information to bind
        return shared::BIND_FAILED_NONEXISTENT_VAR;
    }
    else if(it_key_L == mp.end()) {
        // node L doesn't exist, and will be created using
        // node R data with impl::make_reference
        auto & info_R = shared::impl::iter_to_info<Key>(it_key_R);
        shared::impl::make_reference(mp, info_R, key_L);
        return shared::BIND_CREATED_LHS;
    }
    else if(it_key_R == mp.end()) {
        // node R doesn't exist, and will be created using
        // node L data with impl::make_reference
        auto & info_L = shared::impl::iter_to_info<Key>(it_key_L);
        shared::impl::make_reference(mp, info_L, key_R);
        return shared::BIND_CREATED_RHS;
    }
    else{
        // both nodes exist, but we dont know if the types match
        
        // get node information for type checking
        auto & info_L = shared::impl::iter_to_info<Key>(it_key_L);
        auto & info_R = shared::impl::iter_to_info<Key>(it_key_R);
        
        // check types
        if(shared::impl::are_types_equal(info_L, info_R)) {
            // both nodes exist and are of the same type, 
            // so a group has to take over
            shared::impl::propagate_group(mp, info_R, info_L);
            
            // and the nodes should reference each other to
            // complete the link
            shared::impl::link_vars(info_L, info_R);
            
            return shared::BIND_PROPAGATED_LHS_GROUP;
        }
        else {
            // cannot bind different types
            return shared::BIND_FAILED_DIFFERENT_TYPES;
        }
    }
}

// Disconnects two variables, allocating new memory 
// but keeping the original value.
template <typename Key>
inline void unbind(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key1, 
    const std::type_identity_t<Key> & key2
) {
    // both nodes must exist
    if(!mp.contains(key1) || !mp.contains(key2)) return;
    
    // assign pretty names
    shared::info_t<Key> & info1 = mp[key1];
    shared::info_t<Key> & info2 = mp[key2];
    
    // check if nodes are connected
    // if only one node references the other, the system is broken.
    // In this case we procede to remove the node, maybe fixing the problem?
    if(not (info1.refs.contains(key2) || info2.refs.contains(key1))) return;
    
    // disconnect both nodes
    info1.refs.erase(key2);
    info2.refs.erase(key1);
    
    // select a node with a group_id != key
    // put the node in a new group, allocate a new var
    // and propagate the new group
    if(info2.group_id != info2.key) {
        info2.group_id = info2.key;
        shared::impl::allocate_and_notify_subscribers(info2, info2.ptr.get());
        shared::impl::autopropagate_group(mp, info2);
    }
    else {
        info1.group_id = info1.key;
        shared::impl::allocate_and_notify_subscribers(info1, info1.ptr.get());
        shared::impl::autopropagate_group(mp, info1);
    }
}

// Destroy all links between nodes, moving each variable
// to its own group
template <typename Key>
inline void unbind_all(shared::map_type<Key> & mp) {
    // for every element in the map
    for(auto & [key, info] : mp) {
        // the group id is set to a different value,
        // breaking the groups
        info.group_id = info.key;
        
        // a new var is allocated for the new group
        shared::impl::allocate_and_notify_subscribers(info, info.ptr.get());
        
        // and every reference to other nodes is removed
        info.refs.clear();
    }
}

// Deletes a variable and removes its references from other variables
template <typename Key>
inline void remove(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    // Search for the var info
    auto it = mp.find(key);
    
    // If the info was found
    if(it != mp.end()) {
        // Assign a pretty name
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        // and remove the info
        shared::impl::remove(mp, info);
    }
}

// Deletes every var in the map
template <typename Key>
inline void remove_all(shared::map_type<Key> & mp) {
    mp.clear();
}

// Breaks all links with other vars
template <typename Key>
inline void isolate(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    // Search for the var info
    auto it = mp.find(key);
    
    // If the info was found
    if(it != mp.end()) {
        // Assign a pretty name
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        // Then detach nodes
        shared::impl::detach_nodes(mp, info);
    }
}

enum exists_t : uint_fast8_t {
    VAR_DOESNT_EXIST,
    VAR_EXISTS_TYPES_ARE_DIFFERENT,
    VAR_EXISTS_TYPES_ARE_EQUAL
};

// Finds whether an element with the given key and type exists
template <typename T, typename Key>
inline bool exists(
    const shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    auto it = mp.find(key);
    
    if(it != mp.end()) {
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        if(shared::impl::are_types_equal<T>(info)) {
            return shared::VAR_EXISTS_TYPES_ARE_EQUAL;
        }
        else {
            return shared::VAR_EXISTS_TYPES_ARE_DIFFERENT;
        }
    }
    else {
        return shared::VAR_DOESNT_EXIST;
    }
}

// Finds whether an element with the given key and type exists
template <typename T, typename Key>
inline bool contains(
    const shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    auto it = mp.find(key);
    
    if(it != mp.end()) {
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        if(shared::impl::are_types_equal<T>(info)) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

// Finds whether an element with the given key exists
template <typename Key>
inline bool contains_key(
    const shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    return mp.contains(key);
}

// Searches the map "mp" then returns a pointer to the shared var
// This pointer is invalidated when the shared-var group is modified
template <shared::storable T, typename Key>
inline T * get_ptr(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    auto it = mp.find(key);
    
    // Check if the var exists
    if(it != mp.end()) {
        return reinterpret_cast<T *>(mp[key].ptr.get());
    }
    else {
        return nullptr;
    }
}

// Searches the map for the key, if the key is found a copy of the object is returned,
// else a new object is constructed (but not saved to the shared map)
template <shared::storable T, typename Key>
inline T get(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    T * ptr = shared::get_ptr<T>(mp, key);
    if(ptr != nullptr) {
        return *ptr;
    }
    else {
        return T();
    }
}

// Searches the map for the key, if the key is found a reference to the object is returned,
// else a new object is constructed
// The reference is invalidated when the shared-var group is modified
template <shared::storable T, typename Key>
inline T & auto_get(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key
) {
    // Try to find the var
    T * ptr = shared::get_ptr<T>(mp, key);
    
    // If var was found, return
    if(ptr != nullptr) {
        return *ptr;
    }
    
    // Var was not found, creating a new var
    shared::info_t<Key> & info = *shared::create<T>(mp, key);
    ptr = shared::impl::info_to_data_ptr<T>(info);
    
    // If var was uccessfully created, return
    if(ptr != nullptr) {
        return *ptr;
    }
    
    // Failed to create var, throw
    throw(std::runtime_error("<shared> auto_get failed to create var " + key));
}

} // namespace shared


#endif // FUNCTIONS_HPP
