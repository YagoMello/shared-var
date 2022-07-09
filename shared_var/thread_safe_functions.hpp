#ifndef SHARED_VAR_LIB__THREAD_SAFE_FUNCTIONS_HPP
#define SHARED_VAR_LIB__THREAD_SAFE_FUNCTIONS_HPP

/* Shared Variable Library
 * Thread safe functions
 * Author:  Yago T. de Mello
 * e-mail:  yago.t.mello@gmail.com
 * Version: 2.11.0 2022-07-09
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

// classic locks
#include <mutex>

// shared locks
#include <shared_mutex>

// default lib includes and definitions
#include "includes.hpp"

// main lib types
#include "types.hpp"

// var manipulation
#include "functions.hpp"

// main lib thread safe types
#include "thread_safe_types.hpp"


// The lib namespace
namespace shared::thread_safe {

// Creates a new shared var, stored in the map "mp".
// A pointer to the shared var info is returned.
template <shared::storable T, typename Map, typename Key = typename Map::key_type, shared::assignable_to<T> Value = T>
inline shared::info_t<Key> * create(
    Map & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T(),
    const bool overwrite = false
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::create<T>(mp, key, std::forward<T>(default_value), overwrite);
}

// Copies the src var to the dest var.
// Creates a new dest var if needed.
template <typename Map, typename Key = typename Map::key_type>
inline shared::info_t<Key> * copy(
    Map & mp, 
    const std::type_identity_t<Key> & key_src, 
    const std::type_identity_t<Key> & key_dest, 
    const bool overwrite = false
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::copy(mp, mp, key_src, key_dest, overwrite);
}

// Connects two variables, making them share the same memory
template <typename Map, typename Key = typename Map::key_type>
inline shared::bind_t bind(
    Map & mp, 
    const std::type_identity_t<Key> & key_L, 
    const std::type_identity_t<Key> & key_R
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::bind(mp, key_L, key_R);
}

// Disconnects two variables, allocating new memory 
// but keeping the original value.
template <typename Map, typename Key = typename Map::key_type>
inline void unbind(
    Map & mp, 
    const std::type_identity_t<Key> & key1, 
    const std::type_identity_t<Key> & key2
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    shared::unbind(mp, key1, key2);
}

// Destroy all links between nodes, moving each variable
// to its own group
template <typename Map, typename Key = typename Map::key_type>
inline void unbind_all(Map & mp) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    shared::unbind_all(mp);
}

// Deletes a variable and removes its references from other variables
template <typename Map, typename Key = typename Map::key_type>
inline void remove(
    Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    shared::remove(mp, key);
}

// Deletes every var in the map
template <typename Map, typename Key = typename Map::key_type>
inline void remove_all(Map & mp) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    shared::remove_all(mp);
}

// Breaks all links with other vars
template <typename Map, typename Key = typename Map::key_type>
inline void isolate(
    Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::write_guard_type;
    
    lock_type lock(mp.mutex());
    shared::isolate(mp, key);
}

// Finds whether an element with the given key and type exists
template <typename T, typename Map, typename Key = typename Map::key_type>
inline shared::exists_t exists(
    const Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::read_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::exists(mp, key);
}

// Finds whether an element with the given key and type exists
template <typename T, typename Map, typename Key = typename Map::key_type>
inline bool contains(
    const Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::read_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::contains(mp, key);
}

// Finds whether an element with the given key exists
template <typename Map, typename Key = typename Map::key_type>
inline bool contains_key(
    const Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::read_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::contains_key(mp, key);
}

// Searches the map for the key, if the key is found a copy of the object is returned,
// else a new object is constructed (but not saved to the shared map)
template <shared::storable T, typename Map, typename Key = typename Map::key_type>
inline T get(
    const Map & mp, 
    const std::type_identity_t<Key> & key
) {
    using lock_type = typename Map::read_guard_type;
    
    lock_type lock(mp.mutex());
    return shared::get<T>(mp, key);
}

// Searches the map for the key, if the key is found the value is set.
template <shared::storable T, typename Map, typename Key = typename Map::key_type, shared::assignable_to<T> Value>
inline void set(
    const Map & mp, 
    const std::type_identity_t<Key> & key,
    Value && value
) {
    // Read: We are not modifying the map.
    using lock_type = typename Map::read_guard_type;
    
    lock_type lock(mp.mutex());
    shared::set<T>(mp, key, std::forward<T>(value));
}

} // namespace shared::thread_safe


#endif // THREAD_SAFE_FUNCTIONS_HPP
