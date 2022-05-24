#include <iostream>

#include "shared_var.hpp"

// for each element in the list, print the key, the addres and the group id
void print_list(auto & list, const std::string & comment = "") {
    std::cout << comment << std::endl;
    for(auto & [key, info] : list) {
        std::cout << key << ": " << info.ptr << " of group " << info.group_id << std::endl;
    }
}

// print the value of the var
template <typename T>
void print(auto & list, const auto & key) {
    std::cout << key << " = " << shared::get<T>(list, key) << std::endl;
}

int main() {
    // Creating a variable list. Each variable may be in only one list at a
    // given time
    shared::list_type list; // same as shared::list_type<std::string> list;
    
    // Creating many variables
    auto A1 = shared::make_var(list, "A1", 0.1f); // creates a variable "A1" of
                                                  // type float with value 0.1
    auto A2 = shared::make_var<float>(list, "A2");
    auto B1 = shared::make_var<float>(list, "B1", 1.1f);
    auto B2 = shared::make_var(list, "B2", 1.2f);
    auto B3 = shared::make_var(list, "B3", 1.3f);
    
    print_list(list, "after setup:");
    
    // binding A
    shared::bind(list, "A1", "A2");
    
    print_list(list, "after binding A:");
    
    // binding B using ADL
    bind(list, "B1", "B2");
    bind(list, "B1", "B3");
    bind(list, "B2", "B3");
    bind(list, "B2", "B1"); // Trying to bind twice, no problem.
    
    print_list(list, "after binding B:");
    
    // testing B
    std::cout << "Setting B2 to 123.45f\n";
    B2 = 123.45f;
    print<float>(list, "B3"); // should print 123.45
    
    // binding A and B
    shared::bind(list, "A2", "B1");
    print_list(list, "after binding A and B:");
    
    // testing merge
    std::cout << "Setting A2 to 777.77f\n";
    A2 = 777.77f;
    print<float>(list, "B2"); // should print 777.77
    print<float>(list, "A1"); // should print 777.77
    
    // deleting A2
    shared::remove(list, "A2");
    
    print_list(list, "after removing A2");
    
    std::cout << "Setting A1 to 135.79f\n";
    A1 = 135.79f;
    print<float>(list, "B2"); // should print 777.77
    print<float>(list, "A1"); // should print 135.79
    
    return EXIT_SUCCESS;
}
