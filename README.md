# Shared Var
### std::map on steroids

## What

C++ 20 Header-Only library for high-performance generic maps with binding capabilities.

### One map, many data types
A shared var map can hold any data type:
```cpp
shared::create<float>(vars, "average temperature", 22.5f); // a float named average temperature
shared::create<unsigned int>(vars, "average presure", 101200U); // an unsigned int named average pressure
shared::create<obj_t>(vars, "opaque-data", some_obj); // an object named opaque-data
```

### Abstracted variables
Views can be used as variables:
```cpp
auto players = shared::make_var<uint8_t>(vars, "players", 0); // created a shared var named players
players = 10;                // assigned 10 to "players"
uint8_t pairs = players / 2; // pairs = 5
```
And they also auto-refresh:
```cpp
auto btn = shared::make_var<bool>(window, "ok_btn"); // accessing a GUI object
while(!btn); // the var is updated in real time, because it *is* the same var
// ...
```

### Variable binding
Vars can share the same memory:
```cpp
auto diode_p    = shared::make_var<double>(components, "diode +"); // oh look an electrical pin
auto resistor_n = shared::make_var<double>(components, "res -");   // and another pin
shared::bind(components, "diode +", "res -");                      // lets connect both

resistor_n = 5.0;     // assigning a value to one pin
std::cout << diode_p; // assigns to the other too; prints 5.0
```

### Variable alias
Bind also creates new vars when needed:
```cpp
shared::create<window>(windows, "config_window"); // a window running in the background

shared::remove(windows, "active_window");     // un-setting the active window
// shared::isolate(windows, "active_window"); // un-setting the active window (removing all binds)

shared::bind(windows, "config_window", "active_window"); // setting our config window as the active window
```

### Topology preserving
```cpp
// creating four variables
auto var_1 = shared::make_var<int>(vars, "1");
auto var_2 = shared::make_var<int>(vars, "2");
auto var_3 = shared::make_var<int>(vars, "3");
auto var_4 = shared::make_var<int>(vars, "4");

shared::bind(vars, "1", "2"); // 1 == 2 ;; 3 ;; 4
shared::bind(vars, "2", "3"); // 1 == 2 == 3 ;; 4
shared::bind(vars, "3", "4"); // 1 == 2 == 3 == 4

// 1 == 2 == 3 == 4

shared::unbind(vars, "2", "3");

// 1 == 2 ;; 3 == 4

```

### Generic keys
Anything comparable (<, >, ==) can be used as key:
```cpp
shared::map_type<int> vars; // using *int* as key type

shared::create<someobj>(vars, 0); // key = 0
shared::create<someobj>(vars, 1); // key = 1

shared::bind(vars, 0, 1); // binding 0 and 1
```

### Sharing
Different views of the same key have the same value, share the same memory:
```cpp
auto x = shared::make_var<int>(vars, "my-var");
auto y = shared::make_var<int>(vars, "my-var"); // the var name still "my-var"

x = 5; // x == 5 and y == 5
y = 8; // x == 8 and y == 8
```
```cpp
// somewhere in the code
auto controller = shared::make_var<controller_t>(vars, "main-controller");
...
// in other file, function                            The same *vars* map
//                                                    vvvv
auto main_controller = shared::make_var<controller_t>(vars, "main-controller");
// Both controller and main_controller refer to the same variable (same memory address).
```

### Overriding
Vars can be overriden with `shared::create` when `overwrite` is set to `true`, or by using `shared::make_var`/`shared::make_obj`:
```cpp
shared::create<int>(vars, "X", 42); // "X" = 42
shared::create<int>(vars, "X", 84); // "X" = 42, because "X" is already initialized

shared::get<int>(vars, "X") = 37;   // "X" = 37, explicitly setting "X"

shared::create<double>(vars, "X", 3.14, true); // "X" = 3.14, the old "X" has been deleted

shared::create<char>(vars, "X", 'c'); // "X" = 3.14, "X" has not been overwriten
```

### Builders
Builders are useful for dynamically creatign polymorphic objects based on user input:
```cpp
// the base class
// class vehicle_t {...};
//
// the derived classes
// class car_t : public vehicle_t {...};
// class bus_t : public vehicle_t {...};
// ...
// 
// and the shared var map
// shared::map_type<std::string> vehicles;
// ...

// somewhere in the code, create the builders
shared::make_builder<vehicle_t, car_t>(vehicles, "car");
shared::make_builder<vehicle_t, bus_t>(vehicles, "bus");

// to create a new instance of a car:
vehicle_t * vehicle = shared::build<vehicle_t>(vehicles, "car");

// maybe unique pointers or shared pointers
std::unique_ptr<vehicle_t> unique_bus = shared::build_unique<vehicle_t>(vehicles, "bus");
std::shared_ptr<vehicle_t> shared_bus = shared::build_shared<vehicle_t>(vehicles, "bus");
```

# How
## Installation
Download or clone this repo, move the `shared_var` folder to your project folder and `#include` the files:

`shared_var/shared_var.hpp`     -> Core features (Types, Functions, Views)\
`shared_var/shared_builder.hpp` -> Builders\
`shared_var/multithread.hpp`    -> Thread safe maps and operations (not the vars)\
`shared_var/atomic_wrapper.hpp` -> Thread safe variables

## Functions
**shared_var.hpp**
| Name                    | Description                                                                                    | Returns               |
|-------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`create<T>(map, key, value = T(), overwrite = false)`| Creates a new var with key `key` and value `value`. `overwrite` allows the var to change type. | Pointer to new `info` |
|`copy(mp_src, mp_dest, key_src, key_dest, overwrite)`| Copies a variable from the src map to the dest map                 | Pointer to `dest` info|
|`copy(map, key_src, key_dest, overwrite)`| Copies a var from the same map                                                 | Pointer to `dest` info|
|`bind(map, key1, key2)  `| Binds vars `key1` and `key2`                                                                   | Bind status code      |
|`unbind(map, key1, key2)`| Unbinds vars `key1` and `key2`                                                                 | Nothing               |
|`unbind_all(map)        `| Unbinds every var in the map `map`                                                             | Nothing               |
|`remove(map, key)       `| Removes the var `key` from the map                                                             | Nothing               |
|`remove_all(map)        `| Removes every var from the map `map`                                                           | Nothing               |
|`isolate(map, key)      `| Removes every bind of `key`                                                                    | Nothing               |
|`exists(map, key)       `| Finds whether an element with the given key and type exists                                    | exists_t value        |
|`contains(map, key)     `| Finds whether an element with the given key and type exists                                    | `true` or `false`     |
|`contains_key(map, key) `| True if `map` contains key `key`                                                               | `true` or `false`     |
|`get_ptr<T>(map, key)   `| Pointer to shared-var data, if var doesnt exist returns a nullptr                              | `T *`                 |
|`get<T>(map, key)       `| Copy of shared-var data, if var doesnt exist one is default constructed                        | `T`                   |
|`set<T>(map, key, value)`| Searches the map for the key, if the key is found the value is set                             | Nothing               |
|`auto_get<T>(map, key)  `| Reference to shared-var data, if var doesnt exist creates a new var, if fails to create throws | `T &`                 |
|`make_var<T>(map, key, value = T())`| Returns a view of the var. Creates a new var if necessary. Deletes any variable with the same key but different type. | `var_view_t<T, Map>`|
|`make_obj<T>(map, key, value = T())`| Returns a view of the var. Creates a new var if necessary. Deletes any variable with the same key but different type. | `obj_view_t<T, Map>`|
<!--- |`make_func<FuncPtr, Key> `| Returns a view of the (func) var. Creates a new var if necessary. Deletes any variable with the same key but different type. | Func View | -->
<!--- |`get_func<FuncPtr>(map, key)`| Returns the function pointer | `FuncPtr` | -->
<!--- |`call<FuncPtr>(map, key, args...)`| Calls the function, returns the value returned by the function. | Varies | -->

**shared_builder.hpp**
| Name                     | Description                                                                                    | Returns               |
|--------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`make_builder<Base, Derived>(map, key)`| Returns a view of the builder. Creates a new builder if necessary. Overrides builders with the same key. | Builder View |
|`build<Base>(map, key)`| Builds an object of Derived type registered by `shared::make_builder` | `Base *` |
|`build_unique<Base>(map, key)`| Builds an `std::unique_ptr<Base>` of Derived type registered by `shared::make_builder` | `std::unique_ptr<Base>` |
|`build_shared<Base>(map, key)`| Builds an `std::shared_ptr<Base>` of Derived type registered by `shared::make_builder` | `std::shared_ptr<Base>` |

**multithread.hpp**

TODO (see file)

**atomic_wrapper.hpp**

TODO (see file)

## Types
| Name               | Description                                    | Type                       |
|--------------------|------------------------------------------------|----------------------------|
|`info_t<Key>       `| Stores information about the shared var        |`struct<Key>               `|
|`map_type<Key>     `| Maps `key` to `info_t<Key>`                    |`shared::var_map_t<Key>    `|
|`bind_codes_t      `| Result of `shared::bind(map, key1, key2)`      |`enum : uint_fast8_t       `|
|`var_view_t<T, Map>`| Shared var abstraction, behaves as `T`         |`class<T, Map>             `|
|`obj_view_t<T, Map>`| Shared var abstraction, behaves as `obj*`      |`class<T, Map>             `|

## Var abstraction
`class shared::var_view_t<T, Map>`

This class simplifies the shared var manipulation, behaving as the variable itself.
| Function                  | Description                                                                                    | Returns               |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`var_view_t()             `| Constructs an un-bindable var, similar to a normal var                                         |                       |
|`var_view_t(info_t<Key> * info)`| Constructs a bindable var, the constructor used by `shared::make_var`                     |                       |
|`operator const value_type &() `| Converts the object to (a reference of) the shared var                                    | `var_view_t &`        |
|`operator =(U && value)   `| Assigns a value to the shared var                                                              | `var_view_t &`        |
|`init(map, key)           `|                                                                                                |                       |
|`init(map, info*)         `|                                                                                                |                       |
|`clone(const var_view_t & rhs) `|                                                                                           |                       |
|`ptr()                    `| Get the shared var (data) pointer                                                              | `value_type *`        |
|`ref()                    `| Reference to the shared var data                                                               | `value_type &`        |
|`is_empty()               `|                                                                                                | `bool`                |
|`clear()                  `|                                                                                                |  Nothing              |

| Public member             | Description                                                                                    | Type                  |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`key_type                 `| Exposes the key type                                                                           | Type, `Key`           |
|`value_type               `| Exposes the type of the shared var                                                             | Type, `T`             |

`class shared::obj_view_t<T, Map>`

This class simplifies the shared var manipulation, behaving as a (const) pointer to an object.
| Function                  | Description                                                                                    | Returns               |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`obj_view_t()             `| Constructs an un-bindable var, similar to a normal var                                         |                       |
|`obj_view_t(info_t<Key> * info)`| Constructs a bindable var, the constructor used by `shared::make_obj`                     |                       |
|`operator *               `|                                                                                                |                       |
|`operator ->()            `| Access member functions of the shared var                                                      |                       |
|`init(map, key)           `|                                                                                                |                       |
|`init(map, info*)         `|                                                                                                |                       |
|`clone(const obj_view_t & rhs) `|                                                                                           |                       |
|`ref()                    `| Reference to the shared var data                                                               | `value_type &`        |
|`ptr()                    `| Get the shared var (data) pointer                                                              | `value_type *`        |
|`is_empty()               `|                                                                                                | `bool`                |
|`clear()                  `|                                                                                                |  Nothing              |

| Public member             | Description                                                                                    | Type                  |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`key_type                 `| Exposes the key type                                                                           | Type, `Key`           |
|`value_type               `| Exposes the type of the shared var                                                             | Type, `T`             |
