#include <iostream>



template<typename T>
class Function;

template<typename ReturnType, typename ...InputType>
class Function<ReturnType(InputType...)>{
public:
    template<typename FunctType>
    Function(FunctType func){
        CBC = new CallableClass<FunctType>(func);
    }
    ~Function(){
        delete CBC;
    }

    class CallableBaseClass{
    public:
        virtual ReturnType operator () (InputType&&...) = 0;
        virtual ~CallableBaseClass() = default;
    };

    template <typename FunctType>
    class CallableClass : public CallableBaseClass{
    public:
        CallableClass(FunctType F):F(F){

        }
        ReturnType operator ()  (InputType&&... args) override {
            return F(std::forward<InputType>(args)...);
        }

    private:
        FunctType F;
    };

    ReturnType operator () (InputType&& ... args){
        return (*CBC)(std::forward<InputType>(args)...);
    }

private:
    CallableBaseClass* CBC;
};

void printer(size_t a){
    std::cout << a;
}
int main() {
    Function<void(int)> func = printer;
    func(5);
    Function<int(int)> lambda = [](int x) -> int {
        return x * x;
    };
    std::cout << lambda(2);
    return 0;
}
