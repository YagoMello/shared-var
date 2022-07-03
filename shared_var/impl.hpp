#ifndef IMPL_HPP
#define IMPL_HPP

/* Shared Variable Library
 * Impl
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


// Internal use
namespace shared::impl {

// Allocator template used by shared::create to create new groups when deleting
// nodes or un-binding variables.
// Contains the information needed to allocate and construct variables
template <shared::storable T>
inline std::shared_ptr<void> default_allocator(void * ptr_to_value) {
    std::shared_ptr ptr = std::make_shared<T>();
    
    if(ptr_to_value != nullptr) {
        const T & value = *reinterpret_cast<T *>(ptr_to_value);
        *ptr = value;
    }
    
    return ptr;
}

// Copies the value from dest to src.
// Does not check for nullptrs.
template <shared::storable T>
inline void default_copier(void * ptr_to_dest, void * ptr_to_src) {
    // The src is not const, because "operator =" may not be const.
    T & src = *reinterpret_cast<T *>(ptr_to_src);
    T & dest = *reinterpret_cast<T *>(ptr_to_dest);
    
    // Assign the value to the destination
    dest = src;
}

// Updates the subscribers to the new var address
template <typename Key>
inline void update_subscribers_var_ptr(shared::info_t<Key> & info) {
    void * new_ptr = info.ptr.get();
    // For each subscriber, update the pointer address to 
    // point to the new var
    for(void ** ptr_to_var_ptr : info.pointers_to_var) {
        *ptr_to_var_ptr = new_ptr;
    }
}

// Allocates memory and updates the subscribers to the new var address
template <typename Key>
inline void allocate_and_notify_subscribers(shared::info_t<Key> & info, void * ptr_to_value) {
    // Allocate
    info.ptr = info.allocator(ptr_to_value);
    // Notify subscribers
    shared::impl::update_subscribers_var_ptr(info);
}

// Verifies if types are equal
template <shared::storable T, typename Key>
inline bool are_types_equal(const shared::info_t<Key> & info) {
    return *info.type_id == typeid(T);
}

// Verifies if types are equal
template <typename Key>
inline bool are_types_equal(const shared::info_t<Key> & info_lhs, const shared::info_t<Key> & info_rhs) {
    return *info_lhs.type_id == *info_rhs.type_id;
}

// Converts the std::map::find() result into shared::info_t<Key> data
template <typename Key>
inline shared::info_t<Key> & iter_to_info(typename shared::map_type<Key>::iterator & iter) {
    return iter->second;
}

// Creates a shared var based on the input var, then
// connects both while keeping the input-var group unchanged.
template <typename Key>
inline void make_reference(
    shared::map_type<Key> & mp, 
    shared::info_t<Key> & var_info, 
    const auto & ref_name
) {
    // The new var data
    shared::info_t<Key> new_info;
    
    // Copy almost everything from the input var,
    // except the key and refs.
    new_info.group_id  = var_info.group_id;
    new_info.type_id   = var_info.type_id;
    new_info.key       = ref_name;
    new_info.ptr       = var_info.ptr;
    new_info.allocator = var_info.allocator;
    new_info.copier    = var_info.copier;
    
    // Link the new var to the input var
    new_info.refs.insert(var_info.key);
    
    // Add the new var to the var map
    mp[ref_name] = new_info;
    
    // Link the input var to the new var
    var_info.refs.insert(ref_name);
}

// Applies the source group to the dest group.
// Faster than connecting two groups and autopropagating.
template <typename Key>
inline void propagate_group(shared::map_type<Key> & mp, shared::info_t<Key> & dest, const shared::info_t<Key> & src) {
    // Protect from cyclic references
    if(src.group_id != dest.group_id) {
        // Copy the group and data ptr
        dest.group_id = src.group_id;
        dest.ptr      = src.ptr;
        
        // Update dest subscribers to the new ptr
        shared::impl::update_subscribers_var_ptr(dest);
        
        // Then repeat the process to every connected node
        for(auto & key : dest.refs) {
            shared::impl::propagate_group(mp, mp[key], src);
        }
    }
}

// Propagates the group of a node to every connected node.
// Used by "unbind" and "remove" to find new groups when variables
// get decoupled.
template <typename Key>
inline void autopropagate_group(shared::map_type<Key> & mp, const shared::info_t<Key> & info) {
    for(auto & key : info.refs) {
        shared::impl::propagate_group(mp, mp[key], info);
    }
}

// Make vars reference one another
template <typename Key>
inline void link_vars(shared::info_t<Key> & info1, shared::info_t<Key> & info2) {
    info1.refs.insert(info2.key);
    info2.refs.insert(info1.key);
}

// Disconnect nodes from the selected node
// If "should_remove_node" is "true", the selected node is removed.
template <typename Key>
inline void detach_nodes(shared::map_type<Key> & mp, shared::info_t<Key> & info, const bool should_remove_node = false) {
    // disconnect the soon-to-be-deleted node drom it's refs
    for(const Key & ref_key : info.refs) {
        shared::info_t<Key> & ref = mp[ref_key]; 
        ref.refs.erase(info.key);
    }
    
    // set a new group for a ref, and allocate a new var (for the new group)
    // then propagate the new group
    // procede to the next ref
    // if it has the original group, repeat the process
    for(const Key & ref_key : info.refs) {
        shared::info_t<Key> & ref = mp[ref_key];
        if(ref.group_id == ref.key) {
            // do nothing, this branch is already solved
        }
        else if(ref.group_id == info.group_id) {
            ref.group_id = ref.key;
            shared::impl::allocate_and_notify_subscribers(ref, ref.ptr.get());
            shared::impl::autopropagate_group(mp, ref);
        }
    }
    
    // should delete node?
    if(should_remove_node) {
        // then delete the node
        mp.erase(info.key);
    }
    else{
        // move the node to a new group
        // because the other group may still be in use
        // due to the option to do nothing when ref.group_id == ref.key
        info.group_id = info.key;
        
        // allocate new memory for the new group
        shared::impl::allocate_and_notify_subscribers(info, info.ptr.get());
        
        // and clear the old references
        info.refs.clear();
    }
}

// Deletes a variable and removes its references from other variables
template <typename Key>
inline void remove(shared::map_type<Key> & mp, shared::info_t<Key> & info) {
    // remove references to this node then remove the node
    shared::impl::detach_nodes(mp, info, true);
}

// Get the pointer to the shared var.
// This pointer is invalidated when the shared-var group is modified
template <shared::storable T, typename Key>
inline T * info_to_data_ptr(shared::info_t<Key> & info) {
    return reinterpret_cast<T *>(info.ptr.get());
}

// Get the pointer to the shared var (read only).
// This pointer is invalidated when the shared-var group is modified
template <shared::storable T, typename Key>
inline const T * info_to_data_ptr(const shared::info_t<Key> & info) {
    return reinterpret_cast<const T *>(info.ptr.get());
}

// Subscribing the ptr to the var:
// The var ptr will follow the shared var ptr
// USE THE INTERNAL MAP POINTER
template <shared::storable T, typename Key>
inline void subscribe_view(
    shared::map_type<Key> * & mp_ptr, 
    const std::type_identity_t<Key> & key,
    T * & var_ptr
) {
    auto it = mp_ptr->find(key);
    
    if(it != mp_ptr->end()) {
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        void ** as_void_ptr_ptr = reinterpret_cast<void **>(&var_ptr);
        info.pointers_to_var.insert(as_void_ptr_ptr);
    }
}

// Unsubscribing the ptr to the var
// USE THE INTERNAL MAP POINTER
template <shared::storable T, typename Key>
inline void unsubscribe_view(
    shared::map_type<Key> * & mp_ptr, 
    const std::type_identity_t<Key> & key,
    T * & var_ptr
) {
    auto it = mp_ptr->find(key);
    
    if(it != mp_ptr->end()) {
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        void ** as_void_ptr_ptr = reinterpret_cast<void **>(&var_ptr);
        info.pointers_to_var.erase(as_void_ptr_ptr);
    }
}

} // namespace shared::impl 


#endif // IMPL_HPP
