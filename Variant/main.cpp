#include <iostream>
#include <variant>
#include <exception>

inline constexpr std::size_t variant_npos = -1;

template <typename ...args>
union Union;

template<typename Head>
union Union<Head>{
    typedef Head type;
    Head head;
    bool tail;
};

template <typename Head, typename ...Tail>
union Union<Head, Tail...>{
    typedef Head type;
    Head head;
    Union<Tail...> tail;
};


class BasicVariant{
public:
    virtual int64_t index() = 0;
    virtual void SetIndex(int64_t index) = 0;
};


template <typename ...Args>
class Variant ;

template<>
class Variant<> : public BasicVariant{
public:
    typedef bool UnionType;
};

template <typename H, typename ...T>
class Variant<H, T...> : public BasicVariant{
public:
    Variant():index_(variant_npos){}
    template<typename Initial>
    Variant(Initial element){
        try{
            GetFalse<Initial>(*this) = element;
        }
        catch (std::exception&){
            index_ = variant_npos;
        }
    }

    template<typename Other>
    Variant& operator = (const Other& element){
        GetFalse<Other>(*this) = element;
        return *this;
    }


    Variant& operator = (const Variant<H, T...>& other){
        if (std::addressof(other) == this){
            return *this;
        }
        try{
            U = other.U;
            index_ = other.index_;
            return *this;
        }
        catch(std::runtime_error&){}

    }

    void SetIndex(int64_t index){
        index_ = index;
    }

    int64_t index() override {
        return index_;
    }

    size_t index_;

    Union<H, T...> U ;
    typedef Union<H, T...> UnionType;
};



template <size_t MaxSize, bool except, bool F, typename Returntype, typename ...Args>
struct Finder;

template<size_t MaxSize, bool except, bool F, typename Returntype, typename Head, typename ...Tail >
struct Finder<MaxSize,except, F, Returntype, Head, Tail...>{
    static constexpr Returntype& FindType(BasicVariant& x, typename Variant<Head, Tail...>::UnionType& U){

        if constexpr (std::is_same_v<Returntype, Head>){
            return Finder<MaxSize, except, true, Returntype, Head, Tail...>::FindType(x, U);
        }

        else{
            return Finder<MaxSize, except, false, Returntype, Tail...>::FindType(x, U.tail);
        }

    }
};

template<size_t MaxSize, bool except, typename Returntype, typename Head, typename ...Tail >
struct Finder<MaxSize, except, true, Returntype, Head,  Tail...>{
    static constexpr Returntype& FindType(BasicVariant& x, typename Variant<Head, Tail...>::UnionType& U){
        if (except && MaxSize - sizeof...(Tail) - 1 != x.index()){
            throw std::runtime_error("Bad access");
        }
        if (!except){
            x.SetIndex(MaxSize - sizeof...(Tail) - 1);
        }
        return U.head;
    }
};

template<size_t MaxSize, bool except, bool F, typename Returntype >
struct Finder<MaxSize, except, F, Returntype>{
    static constexpr Returntype& FindType(BasicVariant& x, typename Variant<>::UnionType& U){

        throw std::runtime_error("Not Found");
    }
};

template<size_t Size, size_t index, typename H, typename ...Tail>
struct GetType;

template<typename Returntype, typename ...Args>
Returntype& Get(Variant<Args...>& v){
    return Finder<sizeof...(Args), true, false, Returntype, Args...>::FindType(v ,v.U);
}

template<typename Returntype, typename ...Args>
Returntype& GetFalse(Variant<Args...>& v){
    return Finder<sizeof...(Args), false, false, Returntype, Args...>::FindType(v, v.U);
}


template<size_t Size, size_t index, typename H, typename ...Tail>
struct GetType{
        typedef typename std::conditional<index == Size - sizeof...(Tail) - 1, H,
        typename GetType<Size, index, Tail...>::answer>::type answer;
    };

template<size_t Size, size_t index, typename H>
struct GetType<Size, index, H>{
    typedef typename std::conditional<index == Size - 1, H,
            void >::type answer;

};

template <typename T, typename T2>
struct IsSame{
    static constexpr bool result{false};
};

template <typename T>
struct IsSame<T, T>{
    static constexpr bool result{true};
};

template<size_t index, typename ...Args>
typename GetType<sizeof...(Args) ,index, Args...>::answer Get(Variant<Args...>& v){
    if constexpr(IsSame<typename GetType<sizeof...(Args) ,index, Args...>::answer, void>::result){
        throw std::runtime_error("Not index Found");
    }
    else{
        return Get<typename GetType<sizeof...(Args) ,index, Args...>::answer>(v);
    }
}



int main() {
    //std::variant<int, float> A = 5.12f;
    //std::variant<int, float> C = 5;
   // C = A;
    //std::cout << std::get<float>(C);
    //A = 5.f;
    //std::cout << std::get<float>(A);

    Variant<int, float> B(5.f);
    std::cout << Get<float>(B) << "     ";

    B = 5.5f;
    //std::cout << B.U.tail.head;

    //B.U.head = 5;

    std::cout << Get<float>(B);

    B = 4;
    std::cout << Get<int>(B);
    //Get<0>(B);


}
