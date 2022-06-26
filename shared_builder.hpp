#ifndef SHARED_BUILDER_HPP
#define SHARED_BUILDER_HPP

/* Shared Variable Library
 * Shared Builder Module
 * Author:  Yago T. de Mello
 * e-mail:  yago.t.mello@gmail.com
 * Version: 2.8.0 2022-06-26
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

#include "shared_var.hpp"

namespace shared::impl {

// Constructs an object of Derived type
template <typename Base, typename Derived>
inline Base * default_builder() {
    return new Derived;
}

} // namespace shared::impl


// The lib namespace
namespace shared {

template <typename Base>
using builder_type = Base * (*)();

// Creates a var builder.
// Deletes any variable with the same key but different type.
// If a variable with the same key and types exists, the var_view_t
// will point to the existing var and will not overwrite the value.
template <typename Base, typename Derived = Base, typename Key>
inline shared::var_view_t<builder_type<Base>, Key> make_builder(
    shared::list_type<Key> & ls, 
    const std::type_identity_t<Key> & key
) {
    // Add the builder to the list
    return shared::make_var<builder_type<Base>>(ls, key, &shared::impl::default_builder<Base, Derived>);
}

// If the object builder exists, build an object, otherwise returns a nullptr.
template <typename Base, typename Key>
inline Base * build(shared::list_type<Key> & ls, const std::type_identity_t<Key> & key) {
    // Find the builder
    builder_type<Base> builder = shared::get<builder_type<Base>>(ls, key);
    
    if(builder != nullptr) {
        // And build
        return builder();
    }
    else {
        // Or not...
        return nullptr;
    }
}

// If the object builder exists, build an object, otherwise returns a nullptr.
template <typename Base, typename Key>
inline std::unique_ptr<Base> build_unique(shared::list_type<Key> & ls, const std::type_identity_t<Key> & key) {
    return std::unique_ptr<Base>(shared::build<Base>(ls, key));
}

// If the object builder exists, build an object, otherwise returns a nullptr.
template <typename Base, typename Key>
inline std::shared_ptr<Base> build_shared(shared::list_type<Key> & ls, const std::type_identity_t<Key> & key) {
    return std::shared_ptr<Base>(shared::build<Base>(ls, key));
}

} // namespace shared


#endif // SHARED_BUILDER_HPP
