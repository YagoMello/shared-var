# Shared Var
### std::map on steroids.

## What

C++ 20 Header-Only library for high-performance generic maps with binding capabilities.

### One map, many data types
```cpp
shared::create(vars, "average temperature", 22.5f); // a float named average temperature
shared::create(vars, "average presure", 101200U);   // an unsigned int named average pressure
shared::create(vars, "opaque-data", some_obj);      // an object named opaque-data
```

### Abstracted variables
```cpp
auto players = shared::make_var<uint8_t>(vars, "players", 0); // created a shared var named players
players = 10;                // assigned 10 to "players"
uint8_t pairs = players / 2; // pairs = 5
```
```cpp
auto btn = shared::make_var<bool>(window, "ok_btn"); // accessing a GUI object
if(btn) { // the var is updated in real time, because it *is* the same var
  ...
```

### Variable binding
```cpp
auto diode_p    = shared::make_var<double>(components, "diode +"); // oh look a electrical pin
auto resistor_n = shared::make_var<double>(components, "res -");   // and another pin
shared::bind(components, "diode +", "res -");                      // lets connect both

resistor_n = 5.0;     // assigning a value to one pin
std::cout << diode_p; // assigns to the other too; prints 5.0
```

### Variable alias
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
```cpp
shared::list<int> vars; // using *int* as key type

shared::create<someobj>(vars, 0); // key = 0
shared::create<someobj>(vars, 1); // key = 1

shared::bind(vars, 0, 1); // binding 0 and 1
```

### Sharing
```cpp
auto cows = shared::make_var<int>(vars, "cows");
auto dogs = shared::make_var<int>(vars, "cows"); // the var name still "cows"

cows = 5;
dogs = 8; // cows == 8 too!
```
```cpp
// somewhere in the code
auto controller = shared::make_var<controller_t>(vars, "main-controller");
...
// in other file, function                            The same *vars* list
//                                                    vvvv
auto main_controller = shared::make_var<controller_t>(vars, "main-controller");
// Both controller and main_controller refer to the same variable (same memory address).
```

### Overriding
```cpp
shared::create<int>(vars, "X", 42); // "X" = 42
shared::create<int>(vars, "X", 84); // "X" = 42, because "X" is already initialized

shared::get<int>(vars, "X") = 37;   // "X" = 37, explicitly setting "X"

shared::create<double>(vars, "X", 3.14); // "X" = 3.14, the old "X" has been deleted
```

# How
## Installation
Download `shared_var.hpp`, move the file to your project folder and `#include` it.

## Functions
| Name                     | Description                                                                                    | Returns               |
|--------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`create<T>(list, key, value = T())`| Creates a new var with key `key` and value `value`. `T` may be omitted if a value `value` is given to the variable | Pointer to new `info` |
|`bind(list, key1, key2)  `| Binds vars `key1` and `key2`                                                                   | Bind status code      |
|`unbind(list, key1, key2)`| Unbinds vars `key1` and `key2`                                                                 | Nothing               |
|`unbind_all(list)        `| Unbinds every var in the list `list`                                                           | Nothing               |
|`remove(list, key)       `| Removes the var `key` from the list                                                            | Nothing               |
|`remove_all(list)        `| Removes every var from the list `list`                                                         | Nothing               |
|`isolate(list, key)      `| Removes every bind of `key`                                                                    | Nothing               |
|`contains(list, key)     `| True if `list` contains key `key`                                                              | `true` or `false`     |
|`get_ptr<T>(list, key)   `| Pointer to shared-var data                                                                     | `T *`                 |
|`get<T>(list, key)       `| Copy of shared-var data, if var doesnt exist one is default constructed                        | `T`                   |
|`auto_get<T>(list, key)  `| Reference to shared-var data, if var doesnt exist creates a new var, if fails to create throws | `T &`                 |
|`make_var<T>(list, key, value = T())`| Creates a shared var and the `var_t` wrapper. Deletes any variable with the same key but different type. `T` may be omitted if a value `value` is given to the variable | `var_t<T, Key>`|

## Types
| Name           | Description                                     | Type                       |
|----------------|-------------------------------------------------|----------------------------|
|`info_t<Key>   `| Stores information about the shared var         |`struct<Key>               `|
|`list_type<Key>`| Maps `key` to `info_t<Key>`                     |`std::map<Key, info_t<Key>>`|
|`bind_codes_t  `| Result of `shared::bind(list, key1, key2)`      |`enum : uint_fast8_t       `|
|`var_t<T, Key> `| Shared var abstraction, behaves as `T`          |`class<T, Key>             `|

## Var abstraction
`class shared::var_t<T, Key>`

This class simplifies the shared var manipulation, behaving as the variable itself.
| Function                  | Description                                                                                    | Returns               |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`var_t()                  `| Constructs an un-bindable var, similar to a normal var                                         |                       |
|`var_t(info_t<Key> * info)`| Constructs a bindable var, the constructor used by `shared::make_var`                          |                       |
|`data()                   `| Reference to the shared var data                                                               | `value_type &`        |
|`operator value_type &()  `| Converts the object to (a reference of) the shared var                                         | `value_type &`        |
|`operator =(const value_type & value)`| Assigns a value to the shared var                                                   | `value_type &`        |
|`operator ->()            `| Access member functions of the shared var                                                      |                       |
|`info()                   `| Get the shared var `info` pointer                                                              | `info_t<Key> *`       |
|`get_var_ptr()            `| Get the shared var (data) pointer                                                              | `value_type *`        |

| Public member             | Description                                                                                    | Type                  |
|---------------------------|------------------------------------------------------------------------------------------------|-----------------------|
|`value_type               `| Exposes the type of the shared var                                                             | Type, `T`             |
