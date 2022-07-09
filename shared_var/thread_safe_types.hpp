#ifndef SHARED_VAR_LIB__THREAD_SAFE_TYPES_HPP
#define SHARED_VAR_LIB__THREAD_SAFE_TYPES_HPP

/* Shared Variable Library
 * Multithread
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

// var manipulation
#include "functions.hpp"


// The lib namespace
namespace shared::thread_safe {

// Stores information about the shared variables 
// and associates variable names and data.
// Added in 2.9.0
template <typename Key>
class ts_var_map_t {
public:
    // The underlying map type
    using storage_type = std::map<Key, shared::info_t<Key>>;
    
// ==== std::map types ====
    
    using key_type       = typename storage_type::key_type;
    
    using iterator       = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;
    
    using size_type      = typename storage_type::size_type;
    
// ==== multithreading ====
    
    using mutex_type = std::shared_mutex;
    
    using read_guard_type = std::shared_lock<mutex_type>;
    using write_guard_type = std::unique_lock<mutex_type>;
    
// ==== custom constructors and assignment operators ====
    
    // Allows creation of empty maps
    ts_var_map_t() = default;
    
    // Copies should be explicit to prevent unintended behaviour
    ts_var_map_t(const ts_var_map_t &) = delete;
    
    // Moving would break vars with pointers to this map
    ts_var_map_t(ts_var_map_t && var_map) = delete;
    
    // Copies should be explicit to prevent unintended behaviour
    ts_var_map_t & operator =(const ts_var_map_t &) = delete;
    
    // Moving would break vars with pointers to this map
    ts_var_map_t & operator =(ts_var_map_t && var_map) = delete;
    
// ==== std::map functions ====
    
    // Same as std::map::clear
    void clear() noexcept {
        std::scoped_lock lock(mutex_);
        map_.clear();
    }
    
    // Same as std::map::contains
    template <typename K>
    bool contains(const K & key) const {
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
    
    // Same as std::map::cend1
    const_iterator cend() const noexcept {
        return map_.cend();
    }
    
    std::shared_mutex & mutex() {
        return mutex_;
    }
    
private:
    // The real map
    storage_type map_;
    
    // 2 levels of thread access
    mutable std::shared_mutex mutex_;
};

} // namespace shared::thread_safe


#endif // SHARED_VAR_LIB__THREAD_SAFE_TYPES_HPP
