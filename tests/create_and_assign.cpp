#include "../shared_var/shared_var.hpp"
#include "../shared_var/debug_tools.hpp"

#include <iostream>

int main() {
    shared::map_type<std::string> map;
    
    shared::create<int>(map, "A",   0);
    shared::create<int>(map, "B",   1);
    shared::create<double>(map, "123", 1.0/9.0);
    shared::create<const char *>(map, "HW", "Hello World");
    shared::create<void *>(map, "P", (void *)0xBEEF);
    
    std::cout << "A   = " << shared::get<int>(map, "A") << "\n";
    std::cout << "B   = " << shared::get<int>(map, "B") << "\n";
    std::cout << "123 = " << shared::get<double>(map, "123") << "\n";
    std::cout << "HW  = " << shared::get<const char *>(map, "HW") << "\n";
    std::cout << "P   = " << shared::get<void *>(map, "P") << "\n";
    
    shared::debug::print_map(map);
    
    return 0;
}
