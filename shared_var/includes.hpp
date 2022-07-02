#ifndef INCLUDES_HPP
#define INCLUDES_HPP

/* Shared Variable Library
 * Includes
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

// shared::var_view_t::get_var_ptr uses assert()
#include <cassert>

// enum bind_codes_t -> uint_fast8_t
#include <cinttypes>

// concept std::convertible_to<T>
#include <concepts>

// shared::auto_get may throw
#include <exception>

// shared::map_type<Key> -> std::map
#include <map>

// shared::info_t<Key>::ptr -> std::shared_ptr
#include <memory>

// shared::info_t<Key>::refs -> std::set
#include <set>

// Key -> std::string
#include <string>

// shared::info_t<Key>::type_id -> std::type_info
#include <typeinfo>

// std::type_identity_t<Key> and std::is_reference<T>
#include <type_traits>

// std::forward, used in many places to forward args
#include <utility>

#endif // INCLUDES_HPP
