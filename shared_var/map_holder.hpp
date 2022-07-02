#ifndef MAP_HOLDER_HPP
#define MAP_HOLDER_HPP

/* Shared Variable Library
 * Map holder
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

// var manipulation
#include "functions.hpp"

// subscribe and unsubscribe
#include "impl.hpp"


// The lib namespace
namespace shared {

template <typename Key, typename Map = shared::map_type<Key>>
class map_holder_t {
public:
    // The underlying map type
    using storage_type = Map::storage_type;
    
// ==== std::map types ====
    
    using iterator       = Map::iterator;
    using const_iterator = Map::const_iterator;
    
    using size_type      = Map::size_type;
    
// ==== custom constructors and assignment operators ====
    
    // Allows creation of empty maps
    map_holder_t() {
        map_ = new Map;
    }
    
    // Copies should be explicit to prevent unintended behaviour
    map_holder_t(const map_holder_t &) = delete;
    
    // Moving a map view is fine
    // The pointer to the map remains the same
    map_holder_t(map_holder_t && var_map) {
        using std::swap;
        swap(map_, var_map.map_);
    }
    
    ~map_holder_t() {
        delete map_;
    }
    
    // Copies should be explicit to prevent unintended behaviour
    map_holder_t & operator =(const map_holder_t &) = delete;
    
    // Moving a map view is fine
    // The pointer to the map remains the same
    map_holder_t & operator =(map_holder_t && var_map) {
        using std::swap;
        swap(map_, var_map.map_);
        return *this;
    }
    
    constexpr Map * ptr() {
        return map_;
    }
    
    constexpr Map & ref() {
        return *map_;
    }
    
    constexpr bool has_valid_map() const {
        return map_ != nullptr;
    }
    
    /*
    [[nodiscard]] Map copy() {
        // create a copy of the map
        // to help with undo
        // maybe fail if try to delete a var 
        // with subscribers
    }
    
    bool restore(const shared::map_holder_t<Key, Map> & mp) {
        // TODO
    }
    */
    
private:
    Map * map_;
};

/*
// Creates a new shared var, stored in the map "mp".
// A pointer to the shared var info is returned.
template <shared::storable T, typename Key, typename Map, std::convertible_to<T> Value = T>
inline shared::info_t<Key> * create(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T(),
    const bool overwrite = false
) {
    if(mp.has_valid_map()) {
        return shared::create<T>(mp.ref(), key, std::forward<Value>(default_value), overwrite);
    }
    else {
        return nullptr;
    }
}

// Copies the src var in the src map to the dest var in the dest map.
// Creates a new dest var if needed.
template <typename Key>
inline shared::info_t<Key> * copy(
    shared::map_holder_t<Key, Map> & mp_src, 
    shared::map_holder_t<Key, Map> & mp_dest, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    if(mp_src.has_valid_map() && mp_dest.has_valid_map()) {
        return shared::copy(mp_src.ref(), mp_dest.ref(), key_src, key_dest, overwrite);
    }
    else {
        return nullptr;
    }
}

// Copies the src var to the dest var.
// Creates a new dest var if needed.
template <typename Key>
inline shared::info_t<Key> * copy(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    if(mp.has_valid_map()) {
        return shared::copy(mp.ref(), key_src, key_dest, overwrite);
    }
    else {
        return nullptr;
    }
}

// Connects two variables, making them share the same memory
template <typename Key>
inline shared::bind_codes_t bind(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key_a, 
    const std::type_identity_t<Key> & key_b
) {
    if(mp.has_valid_map()) {
        return shared::bind(mp.ref(), key_a, key_b);
    }
    else {
        return nullptr;
    }
}

// Disconnects two variables, allocating new memory 
// but keeping the original value.
template <typename Key>
inline void unbind(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key_a, 
    const std::type_identity_t<Key> & key_b
) {
    if(mp.has_valid_map()) {
        return shared::unbind(mp.ref(), key_a, key_b);
    }
    else {
        return nullptr;
    }
}

// Destroy all links between nodes, moving each variable
// to its own group
template <typename Key>
inline void unbind_all(shared::map_holder_t<Key, Map> & mp) {
    if(mp.has_valid_map()) {
        shared::unbind_all(mp.ref());
    }
}

// Deletes a variable and removes its references from other variables
template <typename Key>
inline void remove(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        shared::remove(mp.ref(), key);
    }
}

// Deletes every var in the map
template <typename Key>
inline void remove_all(shared::map_holder_t<Key, Map> & mp) {
    if(mp.has_valid_map()) {
        shared::remove_all(mp.ref());
    }
}

// Breaks all links with other vars
template <typename Key>
inline void isolate(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        shared::isolate(mp.ref(), key);
    }
}

// Finds whether an element with the given key exists
template <typename Key>
inline bool contains(
    const shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        return shared::contains(mp.ref(), key);
    }
    else {
        return false;
    }
}

// Searches the map "mp" then returns a pointer to the shared var
// This pointer is invalidated when the shared-var group is modified
template <shared::storable T, typename Key>
inline T * get_ptr(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        return shared::get_ptr<T>(mp.ref(), key);
    }
    else {
        return nullptr;
    }
}

// Searches the map for the key, if the key is found a copy of the object is returned,
// else a new object is constructed (but not saved to the shared map)
template <shared::storable T, typename Key>
inline T get(
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        return shared::get<T>(mp.ref(), key);
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
    shared::map_holder_t<Key, Map> & mp, 
    const std::type_identity_t<Key> & key
) {
    if(mp.has_valid_map()) {
        return shared::auto_get<T>(mp.ref(), key);
    }
    else {
        throw;
    }
}
*/

} // namespace shared


#endif // MAP_HOLDER_HPP
