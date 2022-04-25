#include <iostream>

template <bool condition, typename T>
struct EnableIf{
    typedef T type;
};

template <typename T>
struct EnableIf<false, T>{};

template <typename T, typename T2>
struct IsSame{
    static constexpr bool value{false};
};

template <typename T>
struct IsSame<T, T>{
    static constexpr bool value{true};
};

template<typename T, typename T2>
constexpr bool IsSame_V = IsSame<T, T2>::value;


template <typename T>
struct ControlBlock{
    T* address_;
    explicit ControlBlock(T* address):address_(address){}

};

template <typename T>
struct ControlBlock<T[]>{
    T* address_;
    explicit ControlBlock(T* address):address_(address){}
};

template<typename T>
struct RemoveExtent{
    typedef T type_extent;
};

template<typename T>
struct RemoveExtent<T[]>{
    typedef T type_extent;
};


template<typename T>
class UniquePtr{
public:
    typedef typename RemoveExtent<T>::type_extent* ptr_type;
    typedef typename RemoveExtent<T>::type_extent type;
    typedef typename RemoveExtent<T>::type_extent& reference_type;

    explicit UniquePtr(ptr_type address):cb_(new ControlBlock<T>(address)){}
    ~UniquePtr(){
        DeleteValue();
        delete cb_;
    }
    template<typename ...Args>
    explicit UniquePtr(Args&& ...args):cb_(new ControlBlock<T>(new T( std::forward(args)...))){

    }
    UniquePtr(std::nullptr_t){};

    UniquePtr(const UniquePtr<T>& other) = delete;
    UniquePtr(UniquePtr<T>&& other) noexcept : cb_(other.cb_){
        other.cb_ = nullptr;
    };
    UniquePtr& operator = (const UniquePtr<T>& other) = delete;
    UniquePtr& operator = (UniquePtr<T>&& other) noexcept {
        if(std::addressof(other) == this){
            return *this;
        }
        cb_ = other.cb_;
        other.cb_ = nullptr;
    };
    ptr_type Get(){
        return cb_->address_;
    }

    reference_type operator * (){
        return *cb_->address_;
    }

    ptr_type operator -> (){
        return cb_->address_;
    }

    reference_type operator [] (size_t index){
        return cb_->address_[index];
    }

    reference_type operator * () const {
        return *cb_->address_;
    }

    ptr_type operator -> () const{
        return cb_->address_;
    }

    reference_type operator [] (size_t index) const{
        return cb_->address_[index];
    }

    ptr_type Release(){
        ptr_type temp = cb_->address;
        cb_->address = nullptr;
        return temp;
    }

    void Reset(ptr_type other){
        DeleteValue();
        cb_->address_ = other;
    }

protected:
    void DeleteValue(){
        if constexpr(IsSame_V<T, typename RemoveExtent<T>::type_extent>){
            delete cb_->address_;
        }
        else{
            delete []cb_->address_;
        }
    }
    ControlBlock<T>* cb_;

};

template<typename T>
struct SharedControlBlock: public ControlBlock<T>{
    explicit SharedControlBlock(typename RemoveExtent<T>::type_extent* address)
        : ControlBlock<T>(address), shared_counter_(1), weak_counter_(0){}
    size_t shared_counter_;
    size_t weak_counter_;
};

template <typename T>
class WeakPtr;

template<typename T>
class SharedPtr{
public:
    template <typename T1, typename ...Args>
    friend typename EnableIf<IsSame_V<T1,typename RemoveExtent<T1>::type_extent>, SharedPtr<T1>>::type MakeShared(Args&& ...args);

    typedef typename RemoveExtent<T>::type_extent* ptr_type;
    typedef typename RemoveExtent<T>::type_extent type;
    typedef typename RemoveExtent<T>::type_extent& reference_type;
    friend WeakPtr<T>;
    explicit SharedPtr(ptr_type address):cb_(new SharedControlBlock<T>(address)){}
    SharedPtr() = default;
    SharedPtr(std::nullptr_t){}



    ~SharedPtr(){
        LeaveControlBlock();
    }
    SharedPtr(const SharedPtr& other):cb_(other.cb_){
        if(cb_ != nullptr ) {
            ++cb_->shared_counter_;
        }
    }
    SharedPtr(SharedPtr&& other) noexcept:cb_(other.cb_)  {
        other.cb_ = nullptr;
    }
    SharedPtr& operator = (const SharedPtr& other){
        if(std::addressof(other) == this){
            return *this;
        }
        LeaveControlBlock();
        cb_ = other.cb_;
        if(cb_ != nullptr ){
            ++cb_->shared_counter_;
        }
        return *this;
    }
    SharedPtr& operator = (SharedPtr&& other) noexcept {
        if(std::addressof(other) == this){
            return *this;
        }
        LeaveControlBlock();
        cb_ = other.cb_;
        other.cb_ = nullptr;
    }

    ptr_type Get(){
        return cb_->address_;
    }

    reference_type operator * (){
        return *cb_->address_;
    }

    ptr_type operator -> (){
        return cb_->address_;
    }

    reference_type operator [] (size_t index){
        return cb_->address_[index];
    }

    reference_type operator * () const {
        return *cb_->address_;
    }

    ptr_type operator -> () const{
        return cb_->address_;
    }

    reference_type operator [] (size_t index) const{
        return cb_->address_[index];
    }

    size_t UseCount(){
        return cb_->shared_counter_;
    }

    bool Unique(){
        return cb_->shared_counter_ == 1;
    }

    explicit operator bool() {
        return cb_->address_;
    }

    void Reset(ptr_type ptr_other){
        LeaveControlBlock();
        cb_ = new SharedControlBlock<T>(ptr_other);
    }

protected:
    void DeleteValue(){
        if constexpr(IsSame_V<T, typename RemoveExtent<T>::type_extent>){
            delete cb_->address_;
        }
        else{
            delete []cb_->address_;
        }
    }

    void LeaveControlBlock(){
        if(cb_ == nullptr){return;}
        if(cb_->shared_counter_ == 1){
            DeleteValue();
            if (cb_->weak_counter_ == 0){
                delete cb_;
            }
            else{
                --cb_->shared_counter_;
            }
        }
        else{
            --cb_->shared_counter_;
        }
        cb_ = nullptr;
    }
    SharedControlBlock<T>* cb_{nullptr};

private:
    template<typename ...Args>
    explicit SharedPtr(Args&& ...args):cb_(new SharedControlBlock<T>(new T( std::forward<Args>(args)...))){}

    explicit SharedPtr(WeakPtr<T> other):cb_(other.cb_){
        ++cb_->shared_counter_;
    }
};



template <typename T>
class WeakPtr{
public:
    friend SharedPtr<T>;
    WeakPtr() = default;
    WeakPtr(std::nullptr_t){};
    WeakPtr(SharedPtr<T> initial): cb_(initial.cb_){
        if(cb_ != nullptr ) {
            ++cb_->weak_counter_;
        }
    }
    ~WeakPtr(){
        LeaveControlBlock();
    }
    WeakPtr(const WeakPtr& other):cb_(other.cb_){
        if(cb_ != nullptr ) {
            ++cb_->weak_counter_;
        }
    }
    WeakPtr(WeakPtr&& other) noexcept :cb_(other.cb_) {
        other.cb_ = nullptr;
    }
    WeakPtr& operator = (const WeakPtr<T>& other){
        if(this == std::addressof(other)){
            return *this;
        }
        LeaveControlBlock();
        cb_ = other.cb_;
        if(cb_ != nullptr ) {
            ++cb_->weak_counter_;
        }
        return *this;
    }
    WeakPtr& operator  = (WeakPtr<T>&& other) noexcept {
        if(this == std::addressof(other)){
            return *this;
        }
        LeaveControlBlock();
        cb_ = other.cb_;
        other.cb_ = nullptr;
        return *this;
    }

    WeakPtr& operator  = (const SharedPtr<T>& other){
        LeaveControlBlock();
        cb_ = other.cb_;
        if(cb_ != nullptr ) {
            ++cb_->weak_counter_;
        }
        return *this;
    }


    SharedPtr<T> Lock(){
        return SharedPtr<T>(*this);
    }
    bool Expired(){
        return cb_->shared_counter_ == 0;
    }


private:
    void LeaveControlBlock(){
        if(cb_ == nullptr){return;}
        if(cb_->shared_counter_ == 0 && cb_->weak_counter_ == 1){
            delete cb_;
        }
        else{
            --cb_->weak_counter_;
        }
        cb_ = nullptr;
    }
    SharedControlBlock<T>* cb_ {nullptr};
};


template <typename T, typename ...Args>
typename EnableIf<IsSame_V<T,typename RemoveExtent<T>::type_extent>, UniquePtr<T>>::type MakeUnique(Args&& ...args){
    return UniquePtr<T>(std::forward(args)...);
}

template <typename T>
UniquePtr<typename RemoveExtent<T>::type_extent[]> MakeUnique(size_t n){
    return UniquePtr<typename RemoveExtent<T>::type_extent[]>(new typename RemoveExtent<T>::type_extent[n]);
}
//должен быть конструктор по умолчанию у T иначе не сработает new, выход: так как снизу с reinterpret cast

template <typename T, typename ...Args>
typename EnableIf<IsSame_V<T,typename RemoveExtent<T>::type_extent>, SharedPtr<T>>::type MakeShared(Args&& ...args){
    return SharedPtr<T>(std::forward<Args>(args)...);
}

template <typename T>
SharedPtr<typename RemoveExtent<T>::type_extent[]> MakeShared(size_t n){
    return SharedPtr<typename RemoveExtent<T>::type_extent[]>(
            reinterpret_cast<typename RemoveExtent<T>::type_extent*>(new int8_t[n * sizeof(typename RemoveExtent<T>::type_extent)])
    );
}

template <typename T>
class Node{
public:
    Node(T value):value_(value){}
    Node() = default;
    SharedPtr<Node> next_ptr_{nullptr};
    WeakPtr<Node> previous_ptr_;
    T value_;
};

template<typename T>
class List{
public:
    SharedPtr<Node<T>> begin_;
    WeakPtr<Node<T>> end_;
    void PushBack(T value){
        auto node = MakeShared<Node<T>>(value);
        if(size_ == 0){
            begin_ = node;
            end_ = node;
        }
        else{
            node->previous_ptr_ = end_;
            end_.Lock()->next_ptr_ = node;
            end_ = node;
        }
        ++size_;
    }
    size_t size_{0};
};

int main() {
    int b = 5;
    List<int> A;
    for (size_t i = 0; i < 5 ; ++i){
        A.PushBack(i);
    }
    auto it = A.begin_;
    for (size_t i = 0; i < 5 ; ++i){
        std::cout << it->value_ << " ";
        it = it->next_ptr_;
    }

    return 0;
}
