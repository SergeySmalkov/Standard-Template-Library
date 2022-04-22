#include <iostream>
#include <string>

class BaseClass{
public:
    virtual ~BaseClass() = default;
    virtual BaseClass* MakeCopy() = 0;
    virtual const std::type_info& GetTypeInfo() = 0;
};

template <typename P>
class Value: public BaseClass{
public:
    explicit Value(P value_):value_(std::move(value_)){}
    ~Value() override = default;
    Value<P>* MakeCopy() override {
        return new Value<P>(value_);
    };
    const std::type_info& GetTypeInfo() override {
        return typeid(value_);
    }
    P value_ ;
};



class Any{
public:
    template<class T>
    Any(const T& val):bc_(new Value<T>(val)){}
    Any():bc_(nullptr){}
    ~Any(){
        delete bc_;
    }
    Any(const Any &other):bc_(other.bc_->MakeCopy()){}
    Any& operator = (const Any& other) {
        if(std::addressof(other) == this){
            return *this;
        }
        delete bc_;
        bc_ = other.bc_->MakeCopy();
        return *this;
    }

    Any& operator = (Any&& other) noexcept {
        if(std::addressof(other) == this){
            return *this;
        }
        delete bc_;
        bc_ = other.bc_;
        other.bc_ = nullptr;
        return *this;
    }

    Any(Any&& other) noexcept:bc_(other.bc_){
        other.bc_ = nullptr;
    }
    BaseClass* GetBC() const {
        return bc_;
    }
    bool HasValue(){
        return bc_ != nullptr;
    }
    const std::type_info& Type(){
        return bc_->GetTypeInfo();
    }

private:
    BaseClass* bc_;
};

template<typename T>
T AnyCast(const Any &obj){
    return dynamic_cast<Value<T>*>(obj.GetBC())->value_;
}

int main() {
    Any a = 5;
    Any b = Any(a);
    a = 6.24;
    std::cout << AnyCast<double>(a);
    std::cout << a.Type().name();
    return 0;
}
