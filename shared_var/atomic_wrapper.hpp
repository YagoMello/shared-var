#ifndef SHARED_VAR_LIB__ATOMIC_WRAPPER_HPP
#define SHARED_VAR_LIB__ATOMIC_WRAPPER_HPP

/* Shared Variable Library
 * Atomic wrapper
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

// thread safe view
#include "thread_safe_views.hpp"

// std::atomic<T>
#include <atomic>


namespace shared::atomic {

template <typename T>
class atomic_wrapper_t {
public:
    using value_type = T;
    using storage_type = std::atomic<T>;
    
    atomic_wrapper_t() = default;
    
    atomic_wrapper_t(const atomic_wrapper_t & src) {
        value_.store(src.value_.load());
    }
    
    atomic_wrapper_t(const T & src) {
        value_.store(src);
    }
    
    atomic_wrapper_t(atomic_wrapper_t &&) = delete;
    
    atomic_wrapper_t & operator =(const atomic_wrapper_t & rhs) {
        value_.store(rhs.value_.load());
        return *this;
    }
    
    atomic_wrapper_t & operator =(const T & rhs) {
        value_.store(rhs);
        return *this;
    }
    
    atomic_wrapper_t & operator =(atomic_wrapper_t &&) = delete;
    
    ~atomic_wrapper_t() = default;
    
    operator T() const {
        return value_.load();
    }
    
private:
    storage_type value_;
};

template <typename T, typename Map>
using atomic_view_type = shared::thread_safe::ts_var_view_t<shared::atomic::atomic_wrapper_t<T>, Map>;

// Creates a shared var and the var_view_t wrapper.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_view_t
// will point to the existing var and will not overwrite the value.
template <shared::storable T, typename Map, typename Key = typename Map::key_type, shared::assignable_to<T> Value = T>
inline shared::atomic::atomic_view_type<T, Map> make_atomic_var(
    Map & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T()
) {
    // cannot return a dangling/bad view!
    constexpr bool should_overwrite = true;
    shared::info_t<Key> * info = shared::thread_safe::create<shared::atomic::atomic_wrapper_t<T>>(mp, key, std::forward<T>(default_value), should_overwrite);
    return shared::thread_safe::ts_var_view_t<shared::atomic::atomic_wrapper_t<T>, Map>(mp, info);
}

} // namespace shared::atomic


#endif // SHARED_VAR_LIB__ATOMIC_WRAPPER_HPP
