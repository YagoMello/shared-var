# Shared Var
### A std::map on steroids.

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
shared::create<window>(windows, "config_window");        // a window running in the background

shared::isolate(windows, "active_window");               // un-setting the active window
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
