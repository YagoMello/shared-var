#include <iostream>

#include "../shared_var/shared_var.hpp"
#include "../shared_var/debug_tools.hpp"

// print the value of the var
template <typename T>
void print(auto & map, const auto & key) {
    std::cout << key << " = " << shared::get<T>(map, key) << std::endl;
}

int main() {
    // Creating a var map. Each variable may be in only one map at a
    // given time. This map is not copyable nor movable (see map_holder_t).
    shared::map_type<std::string> map;
    
    // Creating many variables
    auto A1 = shared::make_var<float>(map, "A1", 0.1f); // creates a variable "A1" of
                                                        // type float with value 0.1
    auto A2 = shared::make_var<float>(map, "A2"      ); // "A2" is default constructed
    auto B1 = shared::make_var<float>(map, "B1", 1.1f);
    auto B2 = shared::make_var<float>(map, "B2", 1.2f);
    auto B3 = shared::make_var<float>(map, "B3", 1.3f);
    
    // Print the map state to the standard output
    shared::debug::print_map(map, "\nAfter setup, every var is in its own group:");
    
    // Binding A
    shared::bind(map, "A1", "A2");
    
    shared::debug::print_map(map, "\nAfter binding A1 and A2:");
    
    // Binding B
    shared::bind(map, "B1", "B2");
    shared::bind(map, "B1", "B3");
    shared::bind(map, "B2", "B3");
    shared::bind(map, "B2", "B1"); // Trying to bind twice, no problem.
    
    shared::debug::print_map(map, "\nAfter binding B1, B2 and B3:");
    std::cout << "\n";
    
    // Testing B
    std::cout << "Setting B2 to 123.45f\nB3 sould also be 123.45:\n";
    B2 = 123.45f;
    print<float>(map, "B3"); // should print 123.45
    
    // Binding A and B
    shared::bind(map, "A2", "B1");
    shared::debug::print_map(map, "\nAfter binding A2 and B1:");
    std::cout << "\n";
    
    // Testing merge
    std::cout << "Setting A2 to 777.77f\nEvery An and Bn == 777.77f\n";
    A2 = 777.77f;
    print<float>(map, "B2"); // should print 777.77
    print<float>(map, "A1"); // should print 777.77
    
    // Deleting A2
    shared::remove(map, "A2");
    
    shared::debug::print_map(map, "\nAfter removing A2\nA2 was the link between A1 and B1, the groups have split:");
    std::cout << "\n";
    
    std::cout << "Setting A1 to 135.79f\nBn should remain 777.77f:\n";
    A1 = 135.79f;
    print<float>(map, "A1"); // should print 135.79
    print<float>(map, "B2"); // should print 777.77
    
    shared::debug::print_map(map, "\nAfter setting A1 to 135.79f:");
    
    std::cout << "\n";
    return EXIT_SUCCESS;
}
