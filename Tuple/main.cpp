#include <iostream>

template<typename ...Tail>
struct Tuple;

template<>
struct Tuple <>{
    void Swap(Tuple<>& other){}
    bool operator < (const Tuple<>& other){
        return false;
    }
    bool operator > (const Tuple<>& other){
        return false;
    }

    bool operator == (const Tuple<>& other){
        return true;
    }

    bool operator >= (const Tuple<>& other) {
        return true;
    }
    bool operator <= (const Tuple<>& other) {
        return true;
    }
    static constexpr size_t size_ = 0;
};


template<typename Head, typename ...Tail>
struct Tuple <Head, Tail...>: public Tuple<Tail...>{
public:
    Tuple(Head head, Tail... tail): Tuple<Tail...>(std::forward<Tail>(tail)...), head_(head){}
    ~Tuple() = default;
    Tuple(const Tuple<Head, Tail...>& other): Tuple<Tail...>(other.tail_), head_(other.head_){}
    Tuple(Tuple<Head, Tail...>&& other) noexcept : Tuple<Tail...>(std::move(other.tail_)), head_(std::move(other.head_)) {}
    Tuple<Head, Tail...>& operator = (const Tuple<Head, Tail...>& other){
        if (std::addressof(other) == this){
            return *this;
        }
        head_ = other.head_;
        static_cast<Tuple<Tail...>&>(*this) = other.tail_;
        return *this;
    }
    Tuple<Head, Tail...>& operator = (Tuple<Head, Tail...>&& other) noexcept{
        if (std::addressof(other) == this){
            return *this;
        }
        head_ = std::move(other.head_);
        tail_ = std::move(other.tail_);
        return *this;
    }

    Head head_;
    Tuple<Tail...> &tail_ = static_cast<Tuple<Tail...>&>(*this);
    static constexpr size_t size_ = sizeof...(Tail) + 1;

    void Swap(Tuple<Head, Tail...>& other){
        std::swap(head_, other.head_);
        tail_.Swap(other.tail_);
    }

    bool operator < (const Tuple<Head, Tail...>& other){
        return head_ == other.head_ ? tail_ < other.tail_ : head_ < other.head_;
    }

    bool operator > (const Tuple<Head, Tail...>& other){
        return other < *this;
    }

    bool operator == (const Tuple<Head, Tail...>& other){
        return !(*this < other) && !(*this > other);
    }

    bool operator >= (const Tuple<Head, Tail...>& other) {
        return !(*this < other);
    }
    bool operator <= (const Tuple<Head, Tail...>& other) {
        return !(*this > other);
    }

};

template<size_t MaxSize, size_t index, typename ...Args>
struct GetType;


template<size_t MaxSize, size_t index, typename Head, typename ...Tail>
struct GetType<MaxSize, index, Head, Tail...>{
    typedef typename std::conditional<index == MaxSize - sizeof...(Tail) - 1, Head,
    typename GetType<MaxSize, index, Tail...>::answer>::type answer;
};

template<size_t MaxSize, size_t index, typename Head>
struct GetType<MaxSize, index, Head>{
    typedef typename std::conditional<index == MaxSize - 1, Head,
            void>::type answer;
};

template<size_t MaxSize, size_t index, typename Head, typename ...Tail>
struct FinderByIndex{
    static constexpr typename GetType<MaxSize, index, Head, Tail...>::answer& Getter(Tuple<Head, Tail...>& tuple_) {
        if constexpr (index >= MaxSize){
            throw std::runtime_error("Out of range");
        }
        if constexpr(index == MaxSize - sizeof...(Tail) - 1){
            return tuple_.head_;
        }
        else{
            return FinderByIndex<MaxSize, index, Tail...>::Getter(tuple_.tail_);
        }
    };

    static constexpr const typename GetType<MaxSize, index, Head, Tail...>::answer& Getter(const Tuple<Head, Tail...>& tuple_) {
        if constexpr (index >= MaxSize){
            throw std::runtime_error("Out of range");
        }
        if constexpr(index == MaxSize - sizeof...(Tail) - 1){
            return tuple_.head_;
        }
        else{
            return FinderByIndex<MaxSize, index, Tail...>::Getter(tuple_.tail_);
        }
    };
};


template<size_t index, typename ...Args>
typename GetType<sizeof...(Args), index, Args...>::answer& Get(Tuple<Args...>& tuple){
    return FinderByIndex<sizeof...(Args), index, Args...>::Getter(tuple);
}

template<size_t index, typename ...Args>
const typename GetType<sizeof...(Args), index, Args...>::answer& Get(const Tuple<Args...>& tuple){
    return FinderByIndex<sizeof...(Args), index, Args...>::Getter(tuple);
}

template <typename ...Args>
auto MakeTuple(Args&&... args){
    return Tuple<typename std::remove_reference<Args>::type...>(std::forward<Args>(args)...);
}

template<typename tuple, int... indexes>
struct HelpConcater{
    template<typename ...Args>
    static auto Call(const tuple& first, Args&&... second){
        if constexpr(tuple::size_ == sizeof...(indexes)){
            return CallCopy(Get<indexes - 1> (first)..., second...);
        }
        else{
            return HelpConcater<tuple, indexes..., sizeof...(indexes) + 1>::Call(first, std::forward<decltype(second)>(second)...);
        }
    }

    template<typename ...Args>
    static auto CallCopy(Args... args){
        return MakeTuple(args...);
    }

};

template <typename tuple1, typename tuple2, int ...tuple2_indexes>
struct Concater{
    static auto Call(const tuple1& first, const tuple2& second){
        if constexpr(tuple2::size_ == sizeof...(tuple2_indexes)){
           return HelpConcater<tuple1>::Call(first, Get<tuple2_indexes - 1>(second)...);
        }
        else{
            return Concater<tuple1, tuple2, tuple2_indexes...,sizeof...(tuple2_indexes) + 1>::Call(first, second);
        }
    }
};


template<typename tuple1, typename tuple2>
auto Concat(tuple1 first, tuple2 second){
    return Concater<tuple1, tuple2>::Call(first, second);
}

template<typename first, typename ...tuples>
auto TupleCat(const first& tuple1, const tuples& ...tuple_n){
    if constexpr(sizeof...(tuples) == 0){
        return tuple1;
    }
    else{
        return Concat(tuple1 , TupleCat(tuple_n...));
    }
}

int main() {
    Tuple<int, double, float> A(1, 2.3, 5.4f);
    Tuple<int, double, float> F(5, 1.3, 7.4f);
    Tuple<int, float, float> B(2, 1.3f, 1.4f);
    Tuple<int, float, int> D (2, 1.3f, 1.4);
    auto C = TupleCat(A, B, D);
    std::cout << Get<7> (C);


    return 0;
}
