/*
A custom unique pointer class - based on generic template
*/

template<typename T>
class UniquePtr{
private:
    T* ptr;

public:
    // Constructor
    explicit UniquePtr(T* p = nullptr): ptr(p) {};

    // Destructor
    ~UniquePtr() noexcept {
        delete ptr;
    };

    // Copy constructor - no allowed to
    UniquePtr(const UniquePtr& other) = delete;

    // Copy assignment operator
    UniquePtr& operator=(const UniquePtr& other) = delete;

    // Move constructor - steal
    UniquePtr(UniquePtr&& other) noexcept: ptr(other.ptr) {
        other.ptr = nullptr;
    }

    // Move assignment operator - steal
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if(this == &other) return *this;
        // delete, assign and steal
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    // Dereferencing methods
    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

    // Getter methods

    // Method: To get the pointer
    T* get() const {
        return ptr;
    }

    // Method: To realease the current pointer
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Method: To reset
    void reset(T* p = nullptr){
        delete ptr;
        ptr = p;
    }

    // Method: For bool checks of existence
    explicit operator bool() const {
        return ptr != nullptr;
    }
};