#ifndef VIEWS_HPP
#define VIEWS_HPP

/* Shared Variable Library
 * Views
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

// The closest to the original var while still receiving
// var-network updates
template <shared::storable T, typename Key>
class var_view_t {
public:
    using value_type = T;
    
// ==== constructors ====

    var_view_t() = default;
    
    var_view_t(const shared::var_view_t<T, Key> & src) {
        this->clone(src);
    }
    
    // Always needs to unsubscribe, moving is not an option.
    var_view_t(shared::var_view_t<T, Key> && src) = delete;
    
    // Create an optimized var.
    var_view_t(shared::map_type<Key> & mp, const std::type_identity_t<Key> & key) {
        this->init(mp, key);
    }
    
    // Create an optimized var from info.
    var_view_t(shared::map_type<Key> & mp, shared::info_t<Key> * info) {
        this->init(mp, info);
    }
    
    // Just unsubscribes the var
    ~var_view_t() {
        this->unsubscribe();
    }
    
// ==== operators ====
    
    var_view_t<T, Key> & operator =(const shared::var_view_t<T, Key> & rhs) {
        *data_ptr_ = *rhs.data_ptr_;
        return *this;
    }
    
    // Always needs to unsubscribe, moving is not an option.
    var_view_t<T, Key> & operator =(shared::var_view_t<T, Key> && rhs) = delete;
    
    // Assign a value to the variable
    value_type & operator =(const value_type & rhs) {
        *data_ptr_ = rhs;
        return *data_ptr_;
    }
    
    // Assign a value to the variable
    value_type & operator =(value_type && rhs) {
        *data_ptr_ = std::move(rhs);
        return *data_ptr_;
    }
    
    // Access the variable (read only)
    constexpr operator const value_type &() const {
        return *data_ptr_;
    }
    
    // Calls the stored var operator()
    template <typename ... Args>
    auto operator ()(Args && ... args) {
        // The callable object
        T & callable = this->ref();
        
        // Call operator() with the forwarded args
        return callable(std::forward<Args>(args)...);
    }
    
// ==== var initialization ====
    
    shared::var_view_t<T, Key> & init(shared::map_type<Key> & mp, const std::type_identity_t<Key> & key) {
        // Self destruct
        this->unsubscribe();
        
        // Copy all members
        data_ptr_ = shared::get_ptr<T>(mp, key);
        map_      = &mp;
        key_      = key;
        
        // Then subscribe
        this->subscribe();
    }
    
    shared::var_view_t<T, Key> & init(shared::map_type<Key> & mp, shared::info_t<Key> * info) {
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
    
    shared::var_view_t<T, Key> & clone(const shared::var_view_t<T, Key> & rhs) {
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
    constexpr value_type & ref() {
        return *data_ptr_;
    }
    
    // Access the variable (read only).
    constexpr const value_type & ref() const {
        return *data_ptr_;
    }
    
    // Get the pointer to the shared var.
    constexpr T * ptr() {
        return data_ptr_;
    }
    
    // Get the pointer to the shared var (read only).
    constexpr const T * ptr() const {
        return data_ptr_;
    }
    
// ==== info ====
    
    // True if not pointing to anything
    constexpr bool is_empty() const {
        return data_ptr_ == nullptr;
    }
    
    // True if not pointing to anything
    void clear() {
        this->unsubscribe();
        
        data_ptr_ = nullptr;
        map_ = nullptr;
        key_ = Key();
    }
    
private:
// ==== internal vars ====
    T * data_ptr_ = nullptr;
    shared::map_type<Key> * map_ = nullptr;
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

// The closest to the original var while still receiving
// var-network updates
template <shared::storable T, typename Key>
class obj_view_t {
public:
    using value_type = T;
    
// ==== constructors ====

    obj_view_t() = default;
    
    obj_view_t(const shared::obj_view_t<T, Key> & src) {
        this->clone(src);
    }
    
    // Always needs to unsubscribe, moving is not an option.
    obj_view_t(shared::obj_view_t<T, Key> && src) = delete;
    
    // Create an optimized var.
    obj_view_t(shared::map_type<Key> & mp, const std::type_identity_t<Key> & key) {
        this->init(mp, key);
    }
    
    // Create an optimized var from info.
    obj_view_t(shared::map_type<Key> & mp, shared::info_t<Key> * info) {
        this->init(mp, info);
    }
    
    // Just unsubscribes the var
    ~obj_view_t() {
        this->unsubscribe();
    }
    
// ==== operators ====
    
    obj_view_t<T, Key> & operator =(const shared::obj_view_t<T, Key> & rhs) {
        *data_ptr_ = *rhs.data_ptr_;
        return *this;
    }
    
    // Always needs to unsubscribe, moving is not an option.
    obj_view_t<T, Key> & operator =(shared::obj_view_t<T, Key> && rhs) = delete;
    
    // Assign a value to the variable
    value_type & operator =(const value_type & rhs) {
        *data_ptr_ = rhs;
        return *data_ptr_;
    }
    
    // Assign a value to the variable
    value_type & operator =(value_type && rhs) {
        *data_ptr_ = std::move(rhs);
        return *data_ptr_;
    }
    
    // Calls the stored var operator()
    template <typename ... Args>
    auto operator ()(Args && ... args) {
        // The callable object
        T & callable = this->ref();
        
        // Call operator() with the forwarded args
        return callable(std::forward<Args>(args)...);
    }
    
// ==== var initialization ====
    
    shared::obj_view_t<T, Key> & init(shared::map_type<Key> & mp, const std::type_identity_t<Key> & key) {
        // Self destruct
        this->unsubscribe();
        
        // Copy all members
        data_ptr_ = shared::get_ptr<T>(mp, key);
        map_      = &mp;
        key_      = key;
        
        // Then subscribe
        this->subscribe();
    }
    
    shared::obj_view_t<T, Key> & init(shared::map_type<Key> & mp, shared::info_t<Key> * info) {
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
    
    shared::obj_view_t<T, Key> & clone(const shared::obj_view_t<T, Key> & rhs) {
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
    constexpr value_type & ref() {
        return *data_ptr_;
    }
    
    // Access the variable (read only).
    constexpr const value_type & ref() const {
        return *data_ptr_;
    }
    
    // Access the members of the shared var
    constexpr T & operator *() {
        return *data_ptr_;
    }
    
    // Access the members of the shared var (read only)
    constexpr const T & operator *() const {
        return *data_ptr_;
    }
    
    // Access the members of the shared var
    constexpr T * operator ->() {
        return data_ptr_;
    }
    
    // Access the members of the shared var (read only)
    constexpr const T * operator ->() const {
        return data_ptr_;
    }
    
    // Get the pointer to the shared var.
    constexpr T * ptr() {
        return data_ptr_;
    }
    
    // Get the pointer to the shared var (read only).
    constexpr const T * ptr() const {
        return data_ptr_;
    }
    
// ==== info ====
    
    // True if not pointing to anything
    constexpr bool is_empty() const {
        return data_ptr_ == nullptr;
    }
    
    // True if not pointing to anything
    void clear() {
        this->unsubscribe();
        
        data_ptr_ = nullptr;
        map_ = nullptr;
        key_ = Key();
    }
    
private:
// ==== internal vars ====
    T * data_ptr_ = nullptr;
    shared::map_type<Key> * map_ = nullptr;
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

} // namespace shared


#endif // VIEWS_HPP
