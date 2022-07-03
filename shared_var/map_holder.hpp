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
    using storage_type = typename Map::storage_type;
    
// ==== std::map types ====
    
    using iterator       = typename Map::iterator;
    using const_iterator = typename Map::const_iterator;
    
    using size_type      = typename Map::size_type;
    
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

} // namespace shared


#endif // MAP_HOLDER_HPP
