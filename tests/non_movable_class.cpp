#include "../shared_var/shared_var.hpp"
#include "../shared_var/multithread.hpp"
#include "../shared_var/atomic_wrapper.hpp"
#include "../shared_var/debug_tools.hpp"

#include <iostream>

struct test_t {
    test_t() = default;
    test_t(const test_t &) = default;
    test_t(test_t &&) = delete;
    
    test_t & operator =(const test_t &) = default;
    test_t & operator =(test_t &&) = delete;
    
    ~test_t() = default;
    
    int value = 0;
};

int main() {
    shared::map_type<std::string> map;
    
    test_t test;
    test.value = 10;
    
    shared::create<test_t>(map, "A", test);
    
    std::cout << "A   = " << shared::get<test_t>(map, "A").value << "\n";
    
    shared::debug::print_map(map);
    
    return 0;
}
