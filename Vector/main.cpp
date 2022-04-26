#include <iostream>
#include <memory>
#include <cstdlib>
#include <vector>
#include <list>
 
template <typename T>
class Mallocator {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef std::ptrdiff_t deference_type;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
 
    Mallocator() = default;
 
    pointer allocate(size_t n) {
        return reinterpret_cast<pointer>(malloc(n * sizeof(value_type)));
    }
 
    pointer allocate_at_least(size_t n) {
        return reinterpret_cast<pointer>(malloc(n * sizeof(value_type)));
    }
 
    void deallocate(pointer p, size_t n) {
        free(p);
    }
 
    static constexpr size_type max_size() {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }
 
    template<typename... Args>
    void construct(pointer p, Args&&... args) {
        new(p) T(std::forward<Args>(args)...);
    }
 
    void destroy(pointer p) {
        p->~T();
    }
 
    template<typename U>
    bool operator == (const Mallocator<U>& other) {
        return true;
    }
};
 
 
static int8_t memory[1000000];
static size_t shift;
 
 
template <typename T>
class StackAllocator {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef std::ptrdiff_t deference_type;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
 
    StackAllocator() = default;
 
    static pointer allocate(size_t n) {
        auto p = reinterpret_cast<pointer>(memory + shift);
        shift += n * sizeof(T);
        return p;
    }
 
    static pointer allocate_at_least(size_t n) {
        auto p = reinterpret_cast<pointer>(memory + shift);
        shift += n * sizeof(T);
        return p;
    }
 
    static void deallocate(pointer p, size_t n) {
    }
 
    static constexpr size_type max_size() {
        return 1000000;
    }
 
    template<typename... Args>
    static void construct(pointer p, Args&&... args) {
        new(p) T(std::forward<Args>(args)...);
    }
 
    static void destroy(pointer p) {
        p->~T();
    }
 
    template<typename U>
    bool operator == (const Mallocator<U>& other) {
        return true;
    }
};
 
template <typename T, typename Alloc = std::allocator<T>>
class Vector {
public:
    using Alloc_Tr = std::allocator_traits<Alloc>;
    typedef typename Alloc_Tr::value_type value_type;
 
    template<bool is_const>
    class Iterator {
    public:
 
        typedef size_t difference_type;
        typedef typename Alloc_Tr::value_type value_type;
        typedef typename std::conditional<is_const,
                                          typename Alloc_Tr::const_pointer,
                                          typename Alloc_Tr::pointer>::type pointer;
 
        typedef typename std::conditional<is_const,
                                        const typename Alloc_Tr::value_type&,
                                          typename Alloc_Tr::value_type&>::type reference;
 
        typedef std::random_access_iterator_tag iterator_category;
 
        Iterator() : ptr_(nullptr), index_(std::numeric_limits<size_t>::max()) {
        }
 
        Iterator(const Iterator<is_const>& it) : ptr_(it.ptr_), index_(it.index_) {
        }
 
        Iterator(Iterator<is_const>&& it) noexcept : ptr_(it.ptr_), index_(it.index_) {
            it.ptr_ = nullptr;
            it.index_ = std::numeric_limits<size_t>::max();
        }
 
        Iterator<is_const>& operator=(const Iterator<is_const>& it) {
            if (std::addressof(it) != this) {
                ptr_ = it.ptr_;
                index_ = it.index_;
            }
            return *this;
        }
 
        Iterator<is_const>& operator=(Iterator<is_const>&& it) noexcept {
            if (std::addressof(it) != this) {
                ptr_ = it.ptr_;
                index_ = it.index_;
                it.index_ = std::numeric_limits<size_t>::max();
                it.ptr_ = nullptr;
            }
            return *this;
        }
 
        Iterator<is_const>& operator++() {
            ++index_;
            return *this;
        }
 
        Iterator<is_const>& operator--() {
            --index_;
            return *this;
        }
 
        Iterator<is_const>& operator+=(int64_t delta) {
            index_ += delta;
            return *this;
        }
 
        Iterator<is_const>& operator-=(int64_t delta) {
            return *this += -delta;
        }
 
        Iterator<is_const> operator++(int) {
            auto copy = *this;
            ++index_;
            return copy;
        }
 
        Iterator<is_const> operator--(int) {
            auto copy = *this;
            ++index_;
            return copy;
        }
 
        reference operator*() {
            return *(ptr_ + index_);
        }
 
        pointer operator->() {
            return ptr_ + index_;
        }
 
        reference operator[](size_t shift_it) {
            return *(ptr_ + index_ + shift_it);
        }
 
        Iterator<is_const> operator+(int64_t delta) {
            auto copy = *this;
            copy += delta;
            return copy;
        }
 
        Iterator<is_const> operator-(int64_t delta) {
            return *this + -delta;
        }
 
        difference_type operator-(const Iterator<is_const>& other) {
            return index_ - other.index_;
        }
 
        friend Iterator<is_const> operator+(int64_t delta, const Iterator<is_const>& it) {
            return it + delta;
        }
 
        bool operator < (const Iterator<is_const>& other) {
            return index_ < other.index_;
        }
 
        bool operator > (const Iterator<is_const>& other) {
            return other.index_ < index_;
        }
 
        bool operator >= (const Iterator<is_const>& other) {
            return !(other.index_ < index_);
        }
 
        bool operator <= (const Iterator<is_const>& other) {
            return !(other.index_ > index_);
        }
 
        bool operator == (const Iterator<is_const>& other) {
            return !(other.index_ > index_) && !(other.index_ < index_);
        }
 
        bool operator != (const Iterator<is_const>& other) {
            return !(*this == other);
        }
    private:
        Iterator(typename Alloc_Tr::pointer ptr, size_t index) : index_(index), ptr_(ptr) {
        }
 
        friend Vector<T, Alloc>;
        size_t index_;
        typename Alloc_Tr::pointer ptr_;
    };
 
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
 
    Vector(size_t n) : data_(Alloc_Tr::allocate(alloc_, n)), size_(n), capacity_(n) {
        for (size_t i = 0; i < n; ++i) {
            Alloc_Tr::construct(alloc_, data_ + i);
        }
    }
 
    Vector() : size_(0), capacity_(8), data_(Alloc_Tr::allocate(alloc_, 8)) {}
 
    Vector(const Vector<T, Alloc>& other)
        : size_(other.size_), capacity_(other.capacity_), data_(Alloc_Tr::allocate(alloc_, other.capacity_)) {
        if (Alloc_Tr::propagate_on_container_copy_assignment) {
            alloc_ = other.alloc_;
        }
        for (size_t i = 0; i < size_; ++i) {
            Alloc_Tr::construct(alloc_, data_ + i, other[i]);
        }
    }
 
    Vector(Vector<T, Alloc>&& other) noexcept : size_(other.size_), capacity_(other.capacity_) {
        if (Alloc_Tr::propagate_on_container_move_assignment) {
            alloc_ = other.alloc_;
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        } else {
            data_ = Alloc_Tr::allocate(alloc_, other.capacity_);
            for (size_t i = 0; i < size_; ++i) {
                Alloc_Tr::construct(alloc_, data_ + i, std::move(other[i]));
            }
            for (size_t i = 0; i < other.size_; ++i) {
                Alloc_Tr::destroy(other.alloc_, other.data_ + i);
            }
            Alloc_Tr::deallocate(other.alloc_, other.capacity_);
        }
    }
 
    Vector<T, Alloc>& operator ==(const Vector<T, Alloc>& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        if (size_ != 0) {
            for (size_t i = 0; i < size_; ++i) {
                Alloc_Tr::destroy(alloc_, data_ + i);
            }
            Alloc_Tr::deallocate(alloc_, capacity_);
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = Alloc_Tr::allocate(alloc_, other.capacity_);
        if (Alloc_Tr::propagate_on_container_copy_assignment) {
            alloc_ = other.alloc_;
        }
        for (size_t i = 0; i < size_; ++i) {
            Alloc_Tr::construct(alloc_, data_ + i, other[i]);
        }
        return  *this;
    }
 
    Vector<T, Alloc>& operator==( Vector<T, Alloc>&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }
        if (size_ != 0) {
            for (size_t i = 0; i < size_; ++i) {
                Alloc_Tr::destroy(alloc_, data_ + i);
            }
            Alloc_Tr::deallocate(alloc_, capacity_);
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = Alloc_Tr::allocate(alloc_, other.capacity_);
        if (Alloc_Tr::propagate_on_container_move_assignment) {
            alloc_ = other.alloc_;
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        } else {
            data_ = Alloc_Tr::allocate(alloc_, other.capacity_);
            for (size_t i = 0; i < size_; ++i) {
                Alloc_Tr::construct(alloc_, data_ + i, std::move(other[i]));
            }
            for (size_t i = 0; i < other.size_; ++i) {
                Alloc_Tr::destroy(other.alloc_, other.data_ + i);
            }
            Alloc_Tr::deallocate(other.alloc_, other.capacity_);
        }
        return  *this;
    }
 
    size_t size() const {
        return size_;
    }
 
    iterator begin() {
        return iterator(data_, 0);
    }
 
    iterator end() {
        return iterator(data_, size_);
    }
 
    iterator cbegin() const {
        return const_iterator(data_, 0);
    }
 
    iterator cend() const {
        return const_iterator(data_, size_);
    }
 
    void insert(iterator it, const value_type& element) {
        emplace(it, element);
    }
 
    void push_back(const value_type& element) {
        emplace(end(), element);
    }
 
    template<typename... Args>
    void emplace(iterator it, Args&&... args) {
        if (size_ == capacity_) {
            resize(capacity_ == 0 ? 8 : capacity_ * 2);
        }
        for (size_t i = size_; i > it.index_; --i) {
            Alloc_Tr::construct(alloc_, data_ + i, std::move(*(data_ + i - 1)));
        }
        Alloc_Tr::construct(alloc_, data_ + it.index_, std::forward<Args>(args)...);
        ++size_;
    }
 
    template<typename... Args>
    void emplace_back(Args&&... args) {
        emplace(end(),  std::forward<Args>(args)...);
    }
 
    void erase(iterator it) {
        Alloc_Tr::destroy(alloc_, data_ + it.index_);
        for (size_t i = it.index_; i < size_ - 1; ++i) {
            Alloc_Tr::construct(alloc_, data_ + i, std::move(*(data_ + i + 1)));
        }
        --size_;
    }
 
    void pop_back() {
        erase(end());
    }
 
    value_type& front() {
        return *(begin());
    }
 
    value_type& back() {
        return *(--end());
    }
 
    value_type& operator [] (size_t i) {
        return *(begin() + i);
    }
 
    const value_type& operator [] (size_t i) const {
        return *(begin() + i);
    }
 
    void clear() {
        for (size_t i = 0; i < size_; ++i) {
            Alloc_Tr::destroy(alloc_, data_ + i);
        }
        Alloc_Tr::deallocate(alloc_, data_, capacity_);
        capacity_ = 8;
        size_ = 0;
        data_ = Alloc_Tr::allocate(alloc_, 8);
    }
private:
 
    void resize(size_t new_capacity) {
        typename Alloc_Tr::pointer new_data = Alloc_Tr::allocate(alloc_, new_capacity);
        size_t i = 0;
        for (auto&& el : *this) {
            Alloc_Tr::construct(alloc_, new_data + i++, std::move(el));
            Alloc_Tr::destroy(alloc_, data_ + i);
        }
        Alloc_Tr::deallocate(alloc_, data_, capacity_);
        capacity_ = new_capacity;
        data_ = new_data;
    }
 
    Alloc alloc_;
    typename Alloc_Tr::pointer data_;
    size_t capacity_;
    size_t size_;
};
 
int main() {
 
    std::vector<std::vector<int, StackAllocator<int>>, StackAllocator<std::vector<int, StackAllocator<int>>>> v;
    v.push_back({1,2 , 3});
    v.push_back({1,333 , -3});
    v.push_back({1,-767 , -3});
    for (auto&& x : v) {
        for(auto&& el : x) {
            std::cout << el << " ";
        }
        std::cout << std::endl;
    }
 
    std::list<int, Mallocator<int>> l;
    l.push_back(3);
 
    Vector<int, Mallocator<int>> v2;
    for (size_t i = 0; i < 10; ++i) {
        v2.push_back(10 - i);
    }
 
    for (auto&& x : v2) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    v2.erase(v2.begin() + 3);
    for (auto&& x : v2) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    std::sort(v2.begin(), v2.end());
    for (auto&& x : v2) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    return 0;
}
