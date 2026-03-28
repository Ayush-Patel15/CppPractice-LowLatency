/*
To use different type of allocators - for the STL based data objects. Such that, they can be same or say equal.
Let your custom allocator inherit, from std::pmr::memory_resource and override this 3 virtual functions
- void* do_allocate(size_t bytes, size_t alignment) override
- void do_deallocate(void*, size_t bytes, size_t alignment) override
- bool do_is_equal(size_t bytes, size_t alignment) const noexcept override

The std::pmr also, has some inbuilt allocators to use
- std::pmr::new_delete_resource       : New/delete or malloc based
- std::pmr::null_memory_resouce       : To define undefined behaviour or memory null ptr error
- std::pmr::monotonic_buffer_resource :  Default Arena allocator
- std::pmr::unsynchronized_pool_resouce: Single threaded pool allocator
- std::pmr::synchronized_pool_resource : Thread-safe pool allocator
*/

// Headers
#include <memory_resource>
#include <iostream>
#include <vector>

int main(){
    // Stack based buffer
    char buffer[4096];
    // Initialize default arena allocator
    std::pmr::monotonic_buffer_resource arena(buffer, sizeof(buffer));
    // Use it in the vector, as an allocator
    std::pmr::vector<int> vec(&arena);
    // Use that vector class
    for(int i=0; i<100; i++){
        vec.push_back(i);
    }

    std::cout << "Size of vector: " << vec.size() << "\n";
    return 0;
}