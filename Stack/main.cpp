#include <iostream>
#include <memory>
#include <functional>

template<typename T, typename Allocator = std::allocator<T>>
class Stack {
public:
    template<bool isConst>
    class Iterator {
    public:
        typedef size_t difference_type;
        typedef typename std::conditional<isConst,
                typename std::allocator_traits<Allocator>::const_pointer,
                typename std::allocator_traits<Allocator>::pointer>::type pointer;

        typedef typename std::conditional<isConst,
                const typename std::allocator_traits<Allocator>::value_type&,
                typename std::allocator_traits<Allocator>::value_type&>::type reference;

        Iterator(): stack_(nullptr),  index_(std::numeric_limits<size_t>::max()) {}
        Iterator(const Iterator<isConst>& other) : index_(other.index_), stack_(other.stack_) {}
        Iterator(Iterator<isConst>&& other) noexcept : index_(other.index_), stack_(other.stack_) {}
        Iterator<isConst>& operator = (const Iterator<isConst>& other) {
            if (this == std::addressof(other)) {
                return *this;
            }
            index_ = other.index_;
            stack_ = other.ptr_;
        }
        Iterator<isConst>& operator = (Iterator<isConst>&& other) noexcept {
            if (this == std::addressof(other)) {
                return *this;
            }
            index_ = other.index_;
            stack_ = other.index_;
            other.index_ = std::numeric_limits<size_t>::max();
            other.ptr_ = nullptr;
            return *this;
        }
        Iterator<isConst>& operator++() {
            ++index_;
            return *this;
        }

        Iterator<isConst>& operator--() {
            --index_;
            return *this;
        }
        Iterator<isConst>& operator += (int64_t delta) {
            index_ += delta;
            return *this;
        }

        Iterator<isConst>& operator -= (int64_t delta) {
            return *this += -delta;
        }

        Iterator<isConst> operator++(int) {
            auto copy = *this;
            ++index_;
            return copy;
        }

        Iterator<isConst> operator--(int) {
            auto copy = *this;
            ++index_;
            return copy;
        }

        reference operator * () {
            return stack_->get(index_);
        }

        pointer operator -> () {
            return stack_->data_.get() + index_;
        }

        reference operator [] (size_t shift_it) {
            return stack_->get(index_ + shift_it);
        }

        Iterator<isConst> operator + (int64_t delta) {
            auto copy = *this;
            copy += delta;
            return copy;
        }

        Iterator<isConst> operator - (int64_t delta) {
            return *this + -delta;
        }

        difference_type operator-(const Iterator<isConst>& other) {
            return index_ - other.index_;
        }

        friend Iterator<isConst> operator + (int64_t delta, const Iterator<isConst>& it) {
            return it + delta;
        }

        bool operator < (const Iterator<isConst>& other) {
            return index_ < other.index_;
        }

        bool operator > (const Iterator<isConst>& other) {
            return other.index_ < index_;
        }

        bool operator >= (const Iterator<isConst>& other) {
            return !(other.index_ < index_);
        }

        bool operator <= (const Iterator<isConst>& other) {
            return !(other.index_ > index_);
        }

        bool operator == (const Iterator<isConst>& other) {
            return !(other.index_ > index_) && !(other.index_ < index_);
        }

        bool operator != (const Iterator<isConst>& other) {
            return !(*this == other);
        }


    private:
        size_t index_;
        Stack<T>* stack_;
        Iterator(Stack<T>* stack_, size_t index_) : index_(index_), stack_(stack_) {}
        friend Stack<T>;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, size_);
    }

    iterator cbegin() const {
        return const_iterator(begin_, 0);
    }

    iterator cend() const {
        return const_iterator(end_, size_);
    }


    explicit Stack(size_t capacity = 8) : size_(0), capacity_(capacity), data_(std::allocator_traits<Allocator>::allocate(alloc_, capacity), [&](T* ptr){
        std::allocator_traits<Allocator>::destroy(alloc_, ptr);
        std::allocator_traits<Allocator>::deallocate(alloc_, ptr, capacity_);
    }) {
        begin_ = data_.get();
        end_ = data_.get();
    }

    Stack<T>& operator = (const Stack<T>& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        resize_(other.capacity_);
        size_ = 0;
        end_ = begin_;
        for (size_t i = 0; i < other.size_; ++i) {
            emplace_back_(size_,other.data_[i]);
        }
        return *this ;
    }

    explicit Stack(const Stack<T>& other): size_(0), capacity_(other.capacity_), data_(std::allocator_traits<Allocator>::allocate(alloc_, other.capacity_), [&](T* ptr) {
        std::allocator_traits<Allocator>::destroy(alloc_, ptr);
        std::allocator_traits<Allocator>::deallocate(alloc_, ptr, capacity_);
    }) {
        begin_ = data_.get();
        end_ = data_.get();
        for (size_t i = 0; i < other.size_; ++i) {
            emplace_back_(size_, other.data_[i]);
        }
    }

    explicit Stack(Stack<T>&& other) noexcept : size_(other.size_), capacity_(other.capacity_), data_(std::move(other.data_)) {
        begin_ = data_.get();
        end_ = data_.get() + size_;
    }

    Stack<T>& operator = (Stack<T>&& other) noexcept {
        if(this == std::addressof(other)) {
            return *this;
        }
        data_ = std::move(other.data_);
        begin_ = data_.get();
        size_ = other.size_;
        end_ = data_.get() + size_;
        capacity_ = other.capacity_;
        return *this;

    }

    T pop() {
        --size_;
        --end_;
        return data_[size_];
    }
    void clear() {
        size_ = 0;
        end_ = begin_;
    }

    void emplace_back(const T element) {
        if (size_ == capacity_) {
            resize_(capacity_ * 2);
        }

        emplace_back_(size_, element);

    }

    T& get(size_t index) {
        return data_[index];
    }
    void printer() {
        for (size_t i = 0; i < size_; ++i) {
            std::cout << data_[i] << " ";
        }
    }

    bool empty() {
        return size_ == 0;
    }

    T& operator [] (const size_t& index) {
          return get(index);
    }

    bool operator == (const Stack<T>& other) {
        if (capacity_ != other.capacity_ || size_ != other.size_) {
            return false;
        }
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] != other.data_[i]) {
                return false;
            }
        }
        return true;
    }

private:
    void resize_(const size_t& capacity) {
        Stack<T> s(capacity);
        for(size_t i = 0; i < size_; ++i) {
            s.emplace_back_(s.size_,data_[i]);
        }
        data_ = std::move(s.data_);
        capacity_ = capacity;
        begin_ = s.begin_;
        end_ = s.end_;
        size_ = s.size_;
    }

    void emplace_back_(const size_t& address, const T& element){
        std::allocator_traits<Allocator>::construct(alloc_, data_.get() + address, element);
        ++size_;
        ++end_;
    }

    size_t capacity_;
    size_t size_;
    std::unique_ptr<T[], std::function<void(T*)>> data_;
    Allocator alloc_;
    T* begin_;
    T* end_;
};

int main() {

    Stack<int> stack1(2);

    stack1.emplace_back(5);
    stack1.emplace_back(6);
    stack1.emplace_back(7);

    Stack<int> stack2(1);
    stack2 = stack1;

    stack2.printer();
    for (auto&& x : stack2) {
        std::cout << x << " ";
    }

    std::cout << (stack2 == stack1);
    return 0;
}
