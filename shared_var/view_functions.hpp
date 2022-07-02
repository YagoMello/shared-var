#ifndef VIEW_FUNCTIONS_HPP
#define VIEW_FUNCTIONS_HPP

/* Shared Variable Library
 * View functions
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

// view definitions
#include "views.hpp"


// The lib namespace
namespace shared {

// Creates a shared var and the var_view_t wrapper.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_view_t
// will point to the existing var and will not overwrite the value.
template <shared::storable T, typename Key, std::convertible_to<T> Value = T>
inline shared::var_view_t<T, Key> make_var(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T()
) {
    // cannot return a dangling/bad view!
    constexpr bool should_overwrite = true;
    shared::info_t<Key> * info = shared::create<T>(mp, key, std::forward<T>(default_value), should_overwrite);
    return shared::var_view_t<T, Key>(mp, info);
}

// Creates a shared var and the obj_view_t wrapper.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the obj_view_t
// will point to the existing var and will not overwrite the value.
template <shared::storable T, typename Key, std::convertible_to<T> Value = T>
inline shared::obj_view_t<T, Key> make_obj(
    shared::map_type<Key> & mp, 
    const std::type_identity_t<Key> & key, 
    Value && default_value = T()
) {
    // cannot return a dangling/bad view!
    constexpr bool should_overwrite = true;
    shared::info_t<Key> * info = shared::create<T>(mp, key, std::forward<T>(default_value), should_overwrite);
    return shared::obj_view_t<T, Key>(mp, info);
}

} // namespace shared


#endif
