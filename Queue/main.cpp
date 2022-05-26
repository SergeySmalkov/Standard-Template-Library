#include <iostream>
#include <memory>
#include <functional>

template<typename T, typename Allocator = std::allocator<T>>
class Queue {
public:
    explicit Queue(size_t capacity_ = 8): capacity_(capacity_), begin_(0), end_(0), data_(std::allocator_traits<Allocator>::allocate(alloc_, capacity_),
                                                                                          [&](T* ptr){std::allocator_traits<Allocator>::destroy(alloc_, ptr);
                                                                                              std::allocator_traits<Allocator>::deallocate(alloc_, ptr, capacity_);
                                                                                          }) {}

    Queue<T>& operator = (const Queue<T>& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        resize(other.capacity_);
        begin_ = 0;
        end_ = 0;
        for (size_t i = 0; i < other.end_ - other.begin_; ++i) {
            emplace_back_(i , other.data_[other.begin_ + i]);
        }
        return *this;
    }

    explicit Queue(const Queue<T>& other): begin_(0), end_(0), capacity_(other.capacity_), data_(std::allocator_traits<Allocator>::allocate(alloc_, other.capacity_),
                                                                                                 [&](T* ptr){std::allocator_traits<Allocator>::destroy(alloc_, ptr);
                                                                                                     std::allocator_traits<Allocator>::deallocate(alloc_, ptr, capacity_);
                                                                                                 }) {
        for (size_t i = 0; i < other.end_ - other.begin_; ++i) {
            emplace_back_(i, other.data_[other.begin_ + i]);
        }
    }

    Queue<T>& operator = (Queue<T>&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }
        data_ = std::move(other.data);
        begin_ = other.begin_;
        end_ = other.end_;
        capacity_ = other.capacity_;
        return *this;
    }

    explicit Queue(Queue<T>&& other) noexcept : data_(std::move(other.data_)), begin_(other.begin_), end_(other.end_), capacity_(other.capacity_) {
    }

    void emplace_back(const T& element) {
        if(end_ == capacity_) {
            resize(capacity_ * 2);
        }
        emplace_back_(end_, element);
    }

    T pop() {
        if (begin_ == end_) {
            return 0;
        }
        return data_[begin_++];
    };

    friend std::ostream& operator << (std::ostream &os, const Queue<T> &queue ) {
        for (size_t i = queue.begin_; i < queue.end_; ++i) {
            os << queue.data_[i] << " ";
        }
        return os;
    }
    void clear() {
        begin_ = 0;
        end_ = 0;
    }

    T& get(const size_t& index) {
        if (begin_ + index < end_) {
            return data_[begin_ + index];
        }
    }

    bool empty() {
        return end_ - begin_ == 0;
    }

    T& operator [] (const size_t& index) {
        return get(index);
    }

    bool operator == (const Queue<T>& other) {
        if (capacity_ != other.capacity_ || (end_ - begin_) != (other.end_ - other.begin_)){
            return false;
        }
        for (size_t i = begin_; i < end_; ++i) {
            if (data_[begin_ + i] != other.data_[other.begin_ + i]) {
                return false;
            }
        }
        return true;
    }

    template < bool isConst>
    class Iterator {
    public:
        typedef typename std::conditional<isConst, typename std::allocator_traits<Allocator>::const_pointer, typename std::allocator_traits<Allocator>::pointer>::type pointer;
        typedef typename std::conditional<isConst, const typename std::allocator_traits<Allocator>::value_type&, typename std::allocator_traits<Allocator>::value_type&>::type reference;
        Iterator(): queue_(nullptr), index_(4294967296) {}
        Iterator(const Iterator<isConst>& other) : index_(other.index_), queue_(other.queue_) {}
        explicit Iterator(Iterator<isConst>&& other) noexcept : index_(other.index_), queue_(other.queue_) {}
        Iterator<isConst>& operator = (const Iterator<isConst>& other) {
            if (this == std::addressof(other)) {
                return *this;
            }
            index_ = other.index_;
            queue_ = other.queue_;
        }
        Iterator<isConst>& operator = (Iterator<isConst>&& other) noexcept {
            if (this == std::addressof(other)) {
                return *this;
            }
            index_ = other.index_;
            queue_ = other.queue_;
            other.index = 4294967296;
            other.queue_ = nullptr;
            return *this;
        }

        Iterator<isConst>& operator ++ () {
            ++index_;
            return *this;
        }

        Iterator<isConst>& operator -- () {
            --index_;
            return *this;
        }

        Iterator<isConst>& operator +=  (int64_t delta) {
            index_ += delta;
            return *this;
        }

        Iterator<isConst>& operator -=  (int64_t delta) {
            return *this += -delta;
        }

        Iterator<isConst> operator ++ (int) {
            auto copy = *this;
            ++index_;
            return copy;
        }

        Iterator<isConst> operator -- (int) {
            auto copy = *this;
            --index_;
            return copy;
        }

        reference operator * () {
            return queue_->get(index_ );
        }

        pointer operator -> () {
            return queue_->data_.get() + index_;
        }

        reference operator [] (size_t shift_it) {
            return queue_->get(index_ + shift_it);
        }

        Iterator<isConst> operator + (int64_t delta) {
            auto copy = *this;
            copy += delta;
            return copy;
        }

        Iterator<isConst> operator - (int64_t delta) {
            return *this + - delta;
        }

        size_t operator - (const Iterator<isConst> other) {
            return index_ - other.index_;
        }

        friend Iterator<isConst> operator + (int64_t delta, const Iterator<isConst>& it) {
            return it + delta;
        }

        bool operator < (const Iterator<isConst>& other) {
            return index_ < other.index;
        }

        bool operator > (const Iterator<isConst>& other) {
            return index_ < other.index;
        }

        bool operator >= (const Iterator<isConst>& other) {
            return index_ >= other.index_;
        }

        bool operator <= (const Iterator<isConst>& other) {
            return index_ <= other.index_;
        }

        bool operator == (const Iterator<isConst>& other) {
            return index_ == other.index_;
        }

        bool operator != (const Iterator<isConst>& other) {
            return !(*this == other);
        }

    private:
        size_t index_;
        Queue<T>* queue_;
        Iterator(Queue<T>* queue_, size_t index_) : index_(index_), queue_(queue_) {}
        friend Queue<T>;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    iterator begin() {
        return iterator(this, begin_);
    }

    iterator end() {
        return iterator(this, end_);
    }

    iterator cbegin() const {
        return const_iterator(this, begin_);
    }

    iterator cend() const {
        return const_iterator(this, end_);
    }

private:
    void emplace_back_(const size_t& address, const T& element) {
        std::allocator_traits<Allocator>::construct(alloc_, data_.get() + address,element);
        ++end_;
    }

    void resize(size_t new_capacity) {
        Queue<T> new_queue(new_capacity);
        for(size_t i = begin_; i < end_; ++i) {
            new_queue.emplace_back_(new_queue.end_, data_[i]);
        }
        data_ = std::move(new_queue.data_);
        capacity_ = new_queue.capacity_;
        begin_ = new_queue.begin_;
        end_ = new_queue.end_;
    }

    size_t begin_;
    size_t end_;
    size_t capacity_;
    std::unique_ptr<T[], std::function<void(T*)>> data_;
    Allocator alloc_;

};

int main() {
    Queue<int> Q(2);
    Q.emplace_back(5);
    Q.clear();
    Q.emplace_back(6);
    Q.emplace_back(7);
    std::cout << Q;
    Queue<int> P(std::move(Q));

    //std::cout << P;

    for (auto& a : P) {
        std::cout << a;
    }
    return 0;
}
