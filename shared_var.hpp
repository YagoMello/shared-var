#ifndef SHARED_VAR_HPP
#define SHARED_VAR_HPP

/* Shared Variable Library
 * Author:  Yago T. de Mello
 * e-mail:  yago.t.mello@gmail.com
 * Version: 2.5.0 2022-06-05
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

// shared::var_t::get_var_ptr uses assert()
#include <cassert>

// enum bind_codes_t -> uint_fast8_t
#include <cinttypes>

// shared::auto_get may throw
#include <exception>

// snapshots use std::list
#include <list>

// shared::list_type<Key> -> std::map
#include <map>

// shared::info_t<Key>::ptr -> std::shared_ptr
#include <memory>

// shared::info_t<Key>::refs -> std::set
#include <set>

// Key -> std::string
#include <string>

// shared::info_t<Key>::type_id -> std::type_info
#include <typeinfo>

// std::type_identity_t<Key>
#include <type_traits>

// shared functions use std::forward to forward args
// std::declval to use decltype<Func> in shared functions
#include <utility>


// The lib namespace
namespace shared {

template <typename Key>
struct info_t; // forward declaration

// Contains the shared var info
template <typename Key>
struct info_t {
    // The "shared var name" type, defaults to std::string but could be anything
    // accepted by std::map.
    using key_type = Key;
    using allocator_type = std::shared_ptr<void> (*)(void * ptr_to_value);
    using copier_type = void (*)(void * ptr_to_dest, void * ptr_to_src);
    
    std::shared_ptr<void> group_id; // The shared variable (pointer (and type erased))
    key_type key;              // This variable name
    const std::type_info * type_id; // The shared variable type (RTTI), used for type checking
    allocator_type allocator;  // Allocates memory when called
    copier_type copier;        // Copies the value of another var
    std::set<key_type> refs;   // Variables connected to this var
};

// Stores information about the shared variables 
// and associates variable names and data.
template <typename Key = std::string>
using list_type = std::map<Key, shared::info_t<Key>>;


// Internal use
namespace impl {

// Extracts the return type of 
// a function pointer of type FuncPtr
// called with forwarded args of type Args.
template <typename FuncPtr, typename ... Args>
using shared_func_return_type = decltype(
    std::declval<FuncPtr>()(
        std::forward<Args>(
            std::declval<Args>()...
        )...
    )
);

// Allocator template used by shared::create to create new groups when deleting
// nodes or un-binding variables.
// Contains the information needed to allocate and construct variables
template <typename T>
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
template <typename T>
inline void default_copier(void * ptr_to_dest, void * ptr_to_src) {
    // The src is not const, because "operator =" may not be const.
    T & src = *reinterpret_cast<T *>(ptr_to_src);
    T & dest = *reinterpret_cast<T *>(ptr_to_dest);
    
    // Assign the value to the destination
    dest = src;
}

// Constructs an object of Derived type
template <typename Base, typename Derived>
inline Base * default_builder() {
    return new Derived;
}

// Verifies if types are equal
template <typename T, typename Key>
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
inline shared::info_t<Key> & iter_to_info(typename shared::list_type<Key>::iterator & iter) {
    return iter->second;
}

// Creates a shared var based on the input var, then
// connects both while keeping the input-var group unchanged.
template <typename Key>
inline void make_reference(
    shared::list_type<Key> & ls, 
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
    new_info.allocator = var_info.allocator;
    new_info.copier    = var_info.copier;
    
    // Link the new var to the input var
    new_info.refs.insert(var_info.key);
    
    // Add the new var to the var list
    ls[ref_name] = new_info;
    
    // Link the input var to the new var
    var_info.refs.insert(ref_name);
}

// Applies the source group to the dest group.
// Faster than connecting two groups and autopropagating.
template <typename Key>
inline void propagate_group(shared::list_type<Key> & ls, shared::info_t<Key> & dest, const shared::info_t<Key> & src) {
    // Protect from cyclic references
    if(src.group_id != dest.group_id) {
        // Copy the group and data ptr
        dest.group_id = src.group_id;
        
        // Then repeat the process to every connected node
        for(auto & key : dest.refs) {
            shared::impl::propagate_group(ls, ls[key], src);
        }
    }
}

// Propagates the group of a node to every connected node.
// Used by "unbind" and "remove" to find new groups when variables
// get decoupled.
template <typename Key>
inline void autopropagate_group(shared::list_type<Key> & ls, const shared::info_t<Key> & info) {
    for(auto & key : info.refs) {
        shared::impl::propagate_group(ls, ls[key], info);
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
inline void detach_nodes(shared::list_type<Key> & ls, shared::info_t<Key> & info, const bool should_remove_node = false) {
    // disconnect the soon-to-be-deleted node drom it's refs
    for(const Key & ref_key : info.refs) {
        shared::info_t<Key> & ref = ls[ref_key]; 
        ref.refs.erase(info.key);
    }
    
    // set a new group for a ref, and allocate a new var (for the new group)
    // then propagate the new group
    // procede to the next ref
    // if it has the original group, repeat the process
    for(const Key & ref_key : info.refs) {
        shared::info_t<Key> & ref = ls[ref_key];
        if(ref.group_id == info.group_id) {
            ref.group_id = ref.allocator(ref.group_id.get());
            shared::impl::autopropagate_group(ls, ref);
        }
    }
    
    // should delete node?
    if(should_remove_node) {
        // then delete the node
        ls.erase(info.key);
    }
    else{
        // move the node to a new group
        // because the other group may still be in use
        // due to the option to do nothing when ref.group_id == ref.key
        info.group_id = info.allocator(info.group_id.get());
        
        // and clear the old references
        info.refs.clear();
    }
}

// Deletes a variable and removes its references from other variables
template <typename Key>
inline void remove(shared::list_type<Key> & ls, shared::info_t<Key> & info) {
    // remove references to this node then remove the node
    shared::impl::detach_nodes(ls, info, true);
}

// Get the pointer to the shared var.
// This pointer is invalidated when the shared-var group is modified
template <typename T, typename Key>
inline T * info_to_data_ptr(shared::info_t<Key> & info) {
    return reinterpret_cast<T *>(info.group_id.get());
}

// Get the pointer to the shared var (read only).
// This pointer is invalidated when the shared-var group is modified
template <typename T, typename Key>
inline const T * info_to_data_ptr(const shared::info_t<Key> & info) {
    return reinterpret_cast<const T *>(info.group_id.get());
}

// Wraps a function pointer, because a function pointer cannot be
// converted to void * according to ISO C++
template <typename FuncPtr>
struct func_ptr_wrapper_t {
    FuncPtr func_ptr;
    
    template <typename ... Args>
    auto operator ()(Args && ... args) -> decltype(func_ptr(std::forward<Args>(args)...)) {
        // Call the function with the forwarded args
        return func_ptr(std::forward<Args>(args)...);
    }
};

} // namespace impl


// Creates a new shared var, stored in the list "ls".
// A pointer to the shared var info is returned.
template <typename T, typename Key>
inline shared::info_t<Key> * create(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key, 
    const T & default_value = T(),
    const bool overwrite = false
) {
    // Check if the var already exists
    auto it = ls.find(key);
    
    if(it == ls.end()) {
        // The var doesnt exist, lets create it
        
        // The var info contains the shared var and
        // and the control variables
        shared::info_t<Key> info;
        
        // Allocate the memory
        auto data_ptr = std::make_shared<T>(default_value);
        
        // And assign the other parameters
        
        // The key must be unique, so it can be used as group id
        // Is anything more unique than the varible address?
        info.group_id  = data_ptr;
        
        // typeid() returns a reference to an object with
        // static storage durration, allowing us to store the pointer
        // without bothering with the object lifetime
        info.type_id   = &typeid(T);
        
        info.key       = key;
        info.allocator = shared::impl::default_allocator<T>;
        info.copier    = shared::impl::default_copier<T>;
        
        // Save the new var in the list of vars
        ls[key] = info;
        
        // And return the address
        return &ls[key];
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
            shared::impl::remove(ls, info);
            return shared::create(ls, key, default_value, overwrite);
        }
        else {
            // Types are different and can't overwrite, crap.
            return nullptr;
        }
    }
}

// Copies the src var in the src list to the dest var in the dest list.
// Creates a new dest var if needed.
template <typename Key>
inline shared::info_t<Key> * copy(
    shared::list_type<Key> & ls_src, 
    shared::list_type<Key> & ls_dest, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    // Check if the original var exists
    auto it_src = ls_src.find(key_src);
    
    // Check if the new var exists
    auto it_dest = ls_dest.find(key_dest);
    
    if(it_src != ls_src.end() && it_dest == ls_dest.end()) {
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
        
        // Allocate the memory and copy the value
        info_dest.group_id = info_dest.allocator(info_src.group_id.get());
        
        // Save the new var in the list of vars
        ls_dest[key_dest] = info_dest;
        
        // And return the address
        return &ls_dest[key_dest];
    }
    else if(it_src != ls_src.end() && it_dest != ls_dest.end()) {
        // The var exists, but may not be of the same type
        shared::info_t<Key> & info_src  = shared::impl::iter_to_info<Key>(it_src);
        shared::info_t<Key> & info_dest = shared::impl::iter_to_info<Key>(it_dest);
        
        // We cannot return info pointing to another type,
        // when accessing woult lead to UB
        if(shared::impl::are_types_equal(info_src, info_dest)) {
            // Types are equal, copying values and returning existing info
            void * src_ptr  = info_src.group_id.get();
            void * dest_ptr = info_dest.group_id.get();
            
            info_src.copier(dest_ptr, src_ptr);
            return &info_dest;
        }
        else if(overwrite) {
            // Overwrite is set, and types are not equal
            // Then we should delete the current var
            // And try to copy the value again
            shared::impl::remove(ls_dest, info_dest);
            return shared::copy(ls_src, ls_dest, key_src, key_dest, overwrite);
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
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    return shared::copy(ls, ls, key_src, key_dest, overwrite);
}

// Describes which path shared::bind took when binding variables
enum bind_codes_t : uint_fast8_t {
    BIND_FAILED_NONEXISTENT_VAR = 0,
    BIND_FAILED_DIFFERENT_TYPES = 1,
    BIND_CREATED_LHS            = 2,
    BIND_CREATED_RHS            = 3,
    BIND_PROPAGATED_LHS_GROUP   = 4
};

// Connects two variables, making them share the same memory
template <typename Key>
inline shared::bind_codes_t bind(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key_L, 
    const std::type_identity_t<Key> & key_R
) {
    // search for nodes
    auto it_key_L = ls.find(key_L);
    auto it_key_R = ls.find(key_R);
    
    if(it_key_L == ls.end() && it_key_R == ls.end()) {
        // if both nodes doesn't exist, we don't have
        // enough information to bind
        return shared::BIND_FAILED_NONEXISTENT_VAR;
    }
    else if(it_key_L == ls.end()) {
        // node L doesn't exist, and will be created using
        // node R data with impl::make_reference
        auto & info_R = shared::impl::iter_to_info<Key>(it_key_R);
        shared::impl::make_reference(ls, info_R, key_L);
        return shared::BIND_CREATED_LHS;
    }
    else if(it_key_R == ls.end()) {
        // node R doesn't exist, and will be created using
        // node L data with impl::make_reference
        auto & info_L = shared::impl::iter_to_info<Key>(it_key_L);
        shared::impl::make_reference(ls, info_L, key_R);
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
            shared::impl::propagate_group(ls, info_R, info_L);
            
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
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key1, 
    const std::type_identity_t<Key> & key2
) {
    // both nodes must exist
    if(!ls.contains(key1) || !ls.contains(key2)) return;
    
    // assign pretty names
    shared::info_t<Key> & info1 = ls[key1];
    shared::info_t<Key> & info2 = ls[key2];
    
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
        info2.group_id = info2.allocator(info2.group_id.get());
        shared::impl::autopropagate_group(ls, info2);
    }
    else {
        info1.group_id = info1.allocator(info1.group_id.get());
        shared::impl::autopropagate_group(ls, info1);
    }
}

// Destroy all links between nodes, moving each variable
// to its own group
template <typename Key>
inline void unbind_all(shared::list_type<Key> & ls) {
    // for every element in the list
    for(auto & [key, info] : ls) {
        // the group id is set to a different value,
        // breaking the groups
        info.group_id = info.allocator(info.group_id.get());
        
        // and every reference to other nodes is removed
        info.refs.clear();
    }
}

// Deletes a variable and removes its references from other variables
template <typename Key>
inline void remove(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    // Search for the var info
    auto it = ls.find(key);
    
    // If the info was found
    if(it != ls.end()) {
        // Assign a pretty name
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        // and remove the info
        shared::impl::remove(ls, info);
    }
}

// Deletes every var in the list
template <typename Key>
inline void remove_all(shared::list_type<Key> & ls) {
    ls.clear();
}

// Breaks all links with other vars
template <typename Key>
inline void isolate(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    // Search for the var info
    auto it = ls.find(key);
    
    // If the info was found
    if(it != ls.end()) {
        // Assign a pretty name
        shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
        
        // Then detach nodes
        shared::impl::detach_nodes(ls, info);
    }
}

// Finds whether an element with the given key exists
template <typename Key>
inline bool contains(
    const shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    return ls.contains(key);
}

// Searches the list "ls" then returns a pointer to the shared var
// This pointer is invalidated when the shared-var group is modified
template <typename T, typename Key>
inline T * get_ptr(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    auto it = ls.find(key);
    
    // Check if the var exists
    if(it != ls.end()) {
        return reinterpret_cast<T *>(ls[key].group_id.get());
    }
    else {
        return nullptr;
    }
}

// Searches the list for the key, if the key is found a copy of the object is returned,
// else a new object is constructed (but not saved to the shared list)
template <typename T, typename Key>
inline T get(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    T * ptr = shared::get_ptr<T>(ls, key);
    if(ptr != nullptr) {
        return *ptr;
    }
    else {
        return T();
    }
}

// Searches the list for the key, if the key is found a reference to the object is returned,
// else a new object is constructed
// The reference is invalidated when the shared-var group is modified
template <typename T, typename Key>
inline T & auto_get(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    // Try to find the var
    T * ptr = shared::get_ptr<T>(ls, key);
    
    // If var was found, return
    if(ptr != nullptr) {
        return *ptr;
    }
    
    // Var was not found, creating a new var
    shared::info_t<Key> & info = *shared::create<T>(ls, key);
    ptr = shared::impl::info_to_data_ptr<T>(info);
    
    // If var was uccessfully created, return
    if(ptr != nullptr) {
        return *ptr;
    }
    
    // Failed to create var, throw
    throw(std::runtime_error("<shared> auto_get failed to create var " + key));
}

// Shared var abstrcation, encapsulates a shared var and simplifies its manipulation.
// Can be used as a normal var when default constructed.
template <typename T, typename Key>
class var_t {
public:
    // The shared var type
    using value_type = T;
    
    // Default constructor, allows the shared var to be used as 
    // a normal var when not inserted in a shared var list
    var_t() {
        auto data_ptr = std::make_shared<T>();
        
        placeholder_info_.key       = Key();
        placeholder_info_.group_id  = data_ptr;
        placeholder_info_.type_id   = &typeid(T);
        placeholder_info_.allocator = shared::impl::default_allocator<T>;
        placeholder_info_.copier    = shared::impl::default_copier<T>;
        
        info_ = &placeholder_info_;
    }
    
    // Attach a var to a list
    var_t(shared::info_t<Key> * info) {
        info_ = info;
    }
    
    // Access the variable
    constexpr value_type & data() {
        return *this->get_var_ptr();
    }
    
    // Access the variable (read only)
    constexpr operator const value_type &() const {
        return *this->get_var_ptr();
    }
    
    // Assign a value to the variable
    constexpr value_type & operator =(const value_type & rhs) {
        T & value = *this->get_var_ptr();
        value = rhs;
        return value;
    }
    
    // Access the members of the shared var
    constexpr T * operator ->() {
        return this->get_var_ptr();
    }
    
    // Access the members of the shared var (read only)
    constexpr const T * operator ->() const {
        return this->get_var_ptr();
    }
    
    // Calls the stored var operator()
    template <typename ... Args>
    auto operator ()(Args && ... args) {
        // T should not be a function pointer, 
        // see shared::impl::func_ptr_wrapper_t
        
        // The callable object
        T & callable = this->data();
        
        // Call operator() with the forwarded args
        return callable(std::forward<Args>(args)...);
    }
    
    // Get the shared var info
    shared::info_t<Key> * info() {
        return info_;
    }
    
    // Get the shared var info (read only)
    const shared::info_t<Key> * info() const {
        return info_;
    }
    
    // Get the pointer to the shared var.
    // This pointer is invalidated when the shared-var group is modified
    constexpr T * get_var_ptr() {
        assert(info_ != nullptr);
        return reinterpret_cast<T *>(info_->group_id.get());
    }
    
    // Get the pointer to the shared var (read only).
    // This pointer is invalidated when the shared-var group is modified
    constexpr const T * get_var_ptr() const {
        assert(info_ != nullptr);
        return reinterpret_cast<const T *>(info_->group_id.get());
    }
    
private:
    // Points to the shared var info, which contains updated information
    // about the shared var
    shared::info_t<Key> * info_ = nullptr;
    
    // Used when no shared var is assigned, to prevent invalid memory access
    shared::info_t<Key> placeholder_info_;
};

// Creates a shared var and the var_t wrapper.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_t
// will point to the existing var and will not overwrite the value.
template <typename T, typename Key>
inline shared::var_t<T, Key> make_var(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key, 
    const T & default_value = T()
) {
    shared::info_t<Key> * info = shared::create(ls, key, default_value, true);
    return shared::var_t<T, Key>(info);
}

// Creates a shared (function) var and the var_t wrapper.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_t
// will point to the existing var and will not overwrite the value. 
template <typename FuncPtr, typename Key>
inline shared::var_t<shared::impl::func_ptr_wrapper_t<FuncPtr>, Key> make_func(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key,
    FuncPtr funcptr
) {
    using func_wrap_type = shared::impl::func_ptr_wrapper_t<FuncPtr>;
    
    // Wraps the function pointer to save as a void *
    func_wrap_type func_wrapper {
        .func_ptr = funcptr
    };
    
    // Create a shared var
    shared::info_t<Key> * info = shared::create<func_wrap_type>(ls, key, func_wrapper, true);
    
    // And the var_t
    return shared::var_t<func_wrap_type, Key>(info);
}

// Searches the list "ls" then returns the shared (function pointer) var
template <typename FuncPtr, typename Key>
inline FuncPtr get_func(shared::list_type<Key> & ls, const std::type_identity_t<Key> & key) {
    using func_wrap_type = shared::impl::func_ptr_wrapper_t<FuncPtr>;
    
    // Find the func wrapper
    auto it = ls.find(key);
    
    // If the func wrapper wasn't found, do nothing
    if(it == ls.end()) {
        return nullptr;
    }
    
    // Read the var info
    shared::info_t<Key> & info = shared::impl::iter_to_info<Key>(it);
    
    // Then assert that types are equal
    if(not shared::impl::are_types_equal<func_wrap_type>(info)) {
        return nullptr;
    }
    
    // The var exists and the types are equal, lets get the func wrapper
    func_wrap_type * wapper_ptr = shared::impl::info_to_data_ptr<func_wrap_type>(info);
    
    // And return the function pointer 
    return wapper_ptr->func_ptr;
}

// Searches the list "ls" then calls the shared (function pointer) var with the args.
// Throws if the search fails or the pointer is null.
template <typename FuncPtr, typename Key, typename ... Args>
inline auto call(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key, 
    Args && ... args
) -> shared::impl::shared_func_return_type<FuncPtr, Args...> {
    FuncPtr func_ptr = shared::get_func<FuncPtr>(ls, key);
    
    // And build
    if(func_ptr != nullptr) {
        return func_ptr(std::forward<Args>(args)...);
    }
    else {
        throw(std::runtime_error("<shared> Failed to call shared func " + key));
    }
}

// Creates a var builder.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_t
// will point to the existing var and will not overwrite the value.
template <typename Base, typename Derived = Base, typename Key>
inline shared::var_t<shared::impl::func_ptr_wrapper_t<Base * (*)()>, Key> make_builder(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    using func_ptr_type = Base * (*)();
    
    // The new object builder
    func_ptr_type builder = &shared::impl::default_builder<Base, Derived>;
    
    // 
    return shared::make_func(ls, key, builder);
}

// If the object builder exists, build an object, otherwise returns a nullptr.
template <typename Base, typename Key>
inline Base * safe_build(shared::list_type<Key> & ls, const std::type_identity_t<Key> & key) {
    using func_ptr_type = Base * (*)();
    
    // Find the builder
    func_ptr_type builder = shared::get_func<func_ptr_type>(ls, key);
    
    if(builder != nullptr) {
        // And build
        return builder();
    }
    else {
        /// Or not...
        return nullptr;
    }
}

} // namespace shared


#endif // SHARED_VAR_HPP
