#ifndef SHARED_VAR_LIB__MULTITHREAD_VIEWS_HPP
#define SHARED_VAR_LIB__MULTITHREAD_VIEWS_HPP

/* Shared Variable Library
 * Views
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

// default lib includes and definitions
#include "includes.hpp"

// main lib types
#include "types.hpp"

// var manipulation
#include "functions.hpp"

// subscribe and unsubscribe
#include "impl.hpp"

// main lib thread safe types
#include "thread_safe_types.hpp"

// thread safe var manipulation
#include "thread_safe_functions.hpp"


// The lib namespace
namespace shared::thread_safe {

// The closest to the original var while still receiving
// var-network updates
template <shared::storable T, typename Map, typename Key = typename Map::key_type>
class ts_var_view_t {
public:
    using key_type = Key;
    using value_type = T;
    
// ==== constructors ====

    ts_var_view_t() = default;
    
    ts_var_view_t(const shared::thread_safe::ts_var_view_t<T, Map> & src) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(src.map_.mutex());
        
        // Then copy-construct
        data_ptr_ = src.data_ptr_;
        map_      = src.map_;
        key_      = src.key_;
        
        this->subscribe();
    }
    
    // Always needs to unsubscribe, moving is not an option.
    ts_var_view_t(shared::thread_safe::ts_var_view_t<T, Map> && src) = delete;
    
    // Create an optimized var.
    ts_var_view_t(Map & mp, const std::type_identity_t<Key> & key) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(mp.mutex());
        
        // Copy all members
        data_ptr_ = shared::get_ptr<T>(mp, key);
        map_      = &mp;
        key_      = key;
        
        // Then subscribe
        this->subscribe();
    }
    
    // Create an optimized var from info.
    ts_var_view_t(Map & mp, shared::info_t<Key> * info) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(mp.mutex());
        
        // Copy all members
        data_ptr_ = shared::impl::info_to_data_ptr<T>(*info);
        map_      = &mp;
        key_      = info->key;
        
        // Then subscribe
        this->subscribe();
    }
    
    // Just unsubscribes the var
    ~ts_var_view_t() {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        this->unsubscribe();
    }
    
// ==== operators ====
    
    ts_var_view_t<T, Map> & operator =(const shared::thread_safe::ts_var_view_t<T, Map> & rhs) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        *data_ptr_ = *rhs.data_ptr_;
        return *this;
    }
    
    // Always needs to unsubscribe, moving is not an option.
    ts_var_view_t<T, Map> & operator =(shared::thread_safe::ts_var_view_t<T, Map> && rhs) = delete;
    
    // Assign a value to the variable
    template <shared::assignable_to<T> Value>
    shared::thread_safe::ts_var_view_t<T, Map> & operator =(Value && value) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        if constexpr(std::is_move_assignable<T>::value) {
            *data_ptr_ = std::forward<T>(value);
        }
        else {
            *data_ptr_ = value;
        }
        
        return *this;
    }
    
    // Access the variable (read only)
    constexpr operator value_type() const {
        // Atomic read
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        return *data_ptr_;
    }
    
// ==== var initialization ====
    
    shared::thread_safe::ts_var_view_t<T, Map> & init(Map & mp, const std::type_identity_t<Key> & key) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        // Self destruct
        this->unsubscribe();
        
        // Copy all members
        data_ptr_ = shared::get_ptr<T>(mp, key);
        map_      = &mp;
        key_      = key;
        
        // Then subscribe
        this->subscribe();
        return *this;
    }
    
    shared::thread_safe::ts_var_view_t<T, Map> & init(Map & mp, shared::info_t<Key> * info) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        // Self destruct
        this->unsubscribe();
        
        // Copy all members
        data_ptr_ = shared::impl::info_to_data_ptr<T>(*info);
        map_      = &mp;
        key_      = info->key;
        
        // Then subscribe
        this->subscribe();
        return *this;
    }
    
    shared::thread_safe::ts_var_view_t<T, Map> & clone(const shared::thread_safe::ts_var_view_t<T, Map> & rhs) {
        // Atomic write
        using lock_type = typename Map::write_guard_type;
        lock_type lock(map_->mutex());
        
        // Self destruct
        this->unsubscribe();
        
        // Then copy-construct
        data_ptr_ = rhs.data_ptr_;
        map_      = rhs.map_;
        key_      = rhs.key_;
        
        this->subscribe();
        return *this;
    }
    
// ==== access ====
    
    // Access the variable
    constexpr value_type load() const {
        // Atomic read
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        return *data_ptr_;
    }
    
    template <shared::assignable_to<T> Value>
    shared::thread_safe::ts_var_view_t<T, Map> & store(Value && value) {
        // Atomic read: We are not modifying the map
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        // If move operations are available, use them
        if constexpr(std::is_move_assignable<T>::value) {
            *data_ptr_ = std::forward<T>(value);
        }
        else {
            *data_ptr_ = value;
        }
        
        return *this;
    }
    
    // Get the pointer to the shared var.
    // POINTER IS NOT THREAD SAFE
    constexpr T * ptr() {
        // Atomic read
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        return data_ptr_;
    }
    
    // Get the pointer to the shared var (read only).
    // POINTER IS NOT THREAD SAFE
    constexpr const T * ptr() const {
        // Atomic read
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        return data_ptr_;
    }
    
// ==== info ====
    
    // True if not pointing to anything
    constexpr bool is_empty() const {
        // Atomic read
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        return data_ptr_ == nullptr;
    }
    
    // True if not pointing to anything
    void clear() {
        // Atomic read: We are not modifying the map
        using lock_type = typename Map::read_guard_type;
        lock_type lock(map_->mutex());
        
        this->unsubscribe();
        
        data_ptr_ = nullptr;
        map_ = nullptr;
        key_ = Key();
    }
    
private:
// ==== internal vars ====
    
    T * data_ptr_ = nullptr;
    Map * map_ = nullptr;
    Key key_;
    
// ==== helper functions ====
    
    void subscribe() {
        if(data_ptr_ != nullptr) {
            shared::impl::subscribe_view(map_, key_, data_ptr_);
        }
    }
    
    void unsubscribe() {
        if(data_ptr_ != nullptr) {
            shared::impl::unsubscribe_view(map_, key_, data_ptr_);
        }
    }
};

} // namespace shared::thread_safe


#endif // SHARED_VAR_LIB__MULTITHREAD_VIEWS_HPP
