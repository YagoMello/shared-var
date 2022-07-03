#include <iostream>

#include "../shared_var/shared_var.hpp"
#include "../shared_var/debug_tools.hpp"

int main() {
    // Creating a var map. Each variable may be in only one map at a
    // given time. This map is not copyable nor movable (see map_holder_t).
    shared::map_type<std::string> map;
    
// ===== Creating many variables =====
    
    auto A1 = shared::make_var<float>(map, "A1", 0.1f); // Creates a variable "A1" of
                                                        // type float with value 0.1.
    auto A2 = shared::make_var<float>(map, "A2"      ); // "A2" is default constructed.
    auto B1 = shared::make_var<float>(map, "B1", 1.1f);
    auto B2 = shared::make_var<float>(map, "B2", 1.2f);
    auto B3 = shared::make_var<float>(map, "B3", 1.3f);
    
    // Print the map state to the standard output
    shared::debug::print_map(map, "\nAfter setup, every var is in its own group:");
    // After setup, every var is in its own group:
    // map at 0x7ffede618610
    // A1:            0.1 of group A1 and type f at 0x603000000050
    // A2:              0 of group A2 and type f at 0x603000000080
    // B1:            1.1 of group B1 and type f at 0x6030000000b0
    // B2:            1.2 of group B2 and type f at 0x6030000000e0
    // B3:            1.3 of group B3 and type f at 0x603000000110
    
// ===== Binding A =====
    
    shared::bind(map, "A1", "A2");
    
    shared::debug::print_map(map, "\nAfter binding A1 and A2:");
    // After binding A1 and A2:
    // map at 0x7ffede618610
    // A1:            0.1 of group A1 and type f at 0x603000000050
    // A2:            0.1 of group A1 and type f at 0x603000000050
    // B1:            1.1 of group B1 and type f at 0x6030000000b0
    // B2:            1.2 of group B2 and type f at 0x6030000000e0
    // B3:            1.3 of group B3 and type f at 0x603000000110
    
// ===== Binding B =====
    
    shared::bind(map, "B1", "B2");
    shared::bind(map, "B1", "B3");
    shared::bind(map, "B2", "B3");
    shared::bind(map, "B2", "B1"); // Trying to bind twice, no problem.
    
    shared::debug::print_map(map, "\nAfter binding B1, B2 and B3:");
    // After binding B1, B2 and B3:
    // map at 0x7ffede618610
    // A1:            0.1 of group A1 and type f at 0x603000000050
    // A2:            0.1 of group A1 and type f at 0x603000000050
    // B1:            1.1 of group B1 and type f at 0x6030000000b0
    // B2:            1.1 of group B1 and type f at 0x6030000000b0
    // B3:            1.1 of group B1 and type f at 0x6030000000b0
    
// ===== Testing B =====
    
    B2 = 123.45f;
    
    shared::debug::print_map(map, "\nSetting B2 to 123.45f\nBn sould also be 123.45:");
    // Setting B2 to 123.45f
    // Bn sould also be 123.45:
    // map at 0x7ffede618610
    // A1:            0.1 of group A1 and type f at 0x603000000050
    // A2:            0.1 of group A1 and type f at 0x603000000050
    // B1:         123.45 of group B1 and type f at 0x6030000000b0
    // B2:         123.45 of group B1 and type f at 0x6030000000b0
    // B3:         123.45 of group B1 and type f at 0x6030000000b0
    
// ===== Binding A and B =====
    
    shared::bind(map, "A2", "B1");
    
    shared::debug::print_map(map, "\nAfter binding A2 and B1:");
    // After binding A2 and B1:
    // map at 0x7ffede618610
    // A1:            0.1 of group A1 and type f at 0x603000000050
    // A2:            0.1 of group A1 and type f at 0x603000000050
    // B1:            0.1 of group A1 and type f at 0x603000000050
    // B2:            0.1 of group A1 and type f at 0x603000000050
    // B3:            0.1 of group A1 and type f at 0x603000000050
    
// ===== Testing merge =====
    
    A2 = 777.77f;
    
    shared::debug::print_map(map, "\nSetting A2 to 777.77f\nEvery An and Bn == 777.77f:");
    // Setting A2 to 777.77f
    // Every An and Bn == 777.77f:
    // map at 0x7ffede618610
    // A1:         777.77 of group A1 and type f at 0x603000000050
    // A2:         777.77 of group A1 and type f at 0x603000000050
    // B1:         777.77 of group A1 and type f at 0x603000000050
    // B2:         777.77 of group A1 and type f at 0x603000000050
    // B3:         777.77 of group A1 and type f at 0x603000000050
    
// ===== Deleting A2 =====
    
    shared::remove(map, "A2");
    
    shared::debug::print_map(map, "\nAfter removing A2\nA2 was the link between A1 and B1, the groups have split:");
    // After removing A2
    // A2 was the link between A1 and B1, the groups have split:
    // map at 0x7ffede618610
    // A1:         777.77 of group A1 and type f at 0x603000000050
    // B1:         777.77 of group B1 and type f at 0x6030000001d0
    // B2:         777.77 of group B1 and type f at 0x6030000001d0
    // B3:         777.77 of group B1 and type f at 0x6030000001d0
    
// ===== Testing A1 =====

    A1 = 135.79f;
    
    shared::debug::print_map(map, "\nAfter setting A1 to 135.79f\nBn should remain 777.77f:");
    // After setting A1 to 135.79f
    // Bn should remain 777.77f:
    // map at 0x7ffede618610
    // A1:         135.79 of group A1 and type f at 0x603000000050
    // B1:         777.77 of group B1 and type f at 0x6030000001d0
    // B2:         777.77 of group B1 and type f at 0x6030000001d0
    // B3:         777.77 of group B1 and type f at 0x6030000001d0
    
    std::cout << "\n";
    return EXIT_SUCCESS;
}
