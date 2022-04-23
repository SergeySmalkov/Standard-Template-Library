#include <iostream>
#include <memory>

template<typename T>
struct is_array {
    static constexpr bool result = false;
};

template<typename T>
struct is_array<T[]> {
    static constexpr bool result = true;
};

template<typename T>
struct remove_extent {
    using type = T;
};

template<typename T>
struct remove_extent<T[]> {
    using type = T;
};


template <typename T>
class SmartPtr {
public:
    using ptr_t = typename remove_extent<T>::type*;
    using value_t = typename remove_extent<T>::type;

    SmartPtr(ptr_t ptr) : ptr(ptr) {
    }

    virtual ~SmartPtr() {};

    value_t& operator*() {
        return *ptr;
    }

    ptr_t operator->() {
        return ptr;
    }

    value_t& operator [] (size_t index) {
        return ptr[index];
    }


protected:
    ptr_t ptr;
};

template <typename T>
class UniquePtr : public SmartPtr<T> {
    using ptr_t = typename SmartPtr<T>::ptr_t;
    using value_t = typename SmartPtr<T>::value_t;
public:
    UniquePtr(ptr_t ptr) : SmartPtr<T>(ptr) {}

    ~UniquePtr() override {

        if constexpr (is_array<T>::result) {
            delete []SmartPtr<T>::ptr;
        } else {
            delete SmartPtr<T>::ptr;
        }

    }
};

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr : public SmartPtr<T> {
public:
    using ptr_t = typename SmartPtr<T>::ptr_t;
    using value_t = typename SmartPtr<T>::value_t;
    SharedPtr(ptr_t ptr) : SmartPtr<T>(ptr), weak_cnt_(new size_t(0)), use_cnt_(new size_t(1))  {
    }

    SharedPtr(const SharedPtr& other) : SmartPtr<T>(other.ptr), weak_cnt_(other.weak_cnt_), use_cnt_(other.use_cnt_)  {
        ++*use_cnt_;
    }

    SharedPtr& operator = (const SharedPtr& other) {
        if (this != std::addressof(other)) {
            SmartPtr<T>::ptr = other.ptr;
            use_cnt_ = other.use_cnt_;
            ++*use_cnt_;
        }
        return *this;
    }

    size_t use_cnt() {
        return *use_cnt_;
    }

    ~SharedPtr() override {
        if (*use_cnt_ == 1) {
            if constexpr (is_array<T>::result) {
                delete []SmartPtr<T>::ptr;
            } else {
                delete SmartPtr<T>::ptr;
            }
            if (weak_cnt_ == 0) {
                delete use_cnt_;
                delete weak_cnt_;
            }
        } else {
            --*use_cnt_;
        }

    }

public:
    size_t* weak_cnt_;
    size_t* use_cnt_;
private:
    friend WeakPtr<T>;
    SharedPtr(ptr_t ptr, size_t* weak_cnt_, size_t* use_cnt_) : SmartPtr<T>(ptr), use_cnt_(use_cnt_) , weak_cnt_(weak_cnt_) {
        ++*use_cnt_;
    }

};


template <typename T>
class WeakPtr {
    using ptr_t = typename remove_extent<T>::type*;
    using value_t = typename remove_extent<T>::type;
public:
    WeakPtr(const SharedPtr<T>& other)
            : ptr(other.ptr), use_cnt_(other.use_cnt_), weak_cnt_(other.weak_cnt_) {
        ++*other.weak_cnt_;
    }

    ~WeakPtr() {
        if (*use_cnt_ == 0 && *weak_cnt_ == 1) {
            delete use_cnt_;
            delete weak_cnt_;
        } else {
            --*weak_cnt_;
        }
    }

    bool expired() {
        return *use_cnt_ == 0;
    }

    SharedPtr<T> lock() {
        return SharedPtr<T> (ptr, use_cnt_, weak_cnt_);
    }
private:
    ptr_t ptr;
    size_t* weak_cnt_;
    size_t* use_cnt_;
};

class Go {
public:
    void go() {

        std::cout << "HIHI";
    }
};

int main()
{

    SharedPtr<int[]> myp(new int[1000]);
    auto t = myp;
    myp[10] = 177;
    WeakPtr p = myp;
    std::cout << p.lock().use_cnt();




    return 0;
}
