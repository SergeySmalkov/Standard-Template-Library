#include <iostream>
#include <memory>
#include <functional>

size_t HashFunc(std::string s){
    size_t multiplicator = 3;
    size_t result = 0;
    for (auto&& a : s){
        result += a * multiplicator;
    }
    return result;
}


template<typename Key, typename Value>
class Element{
public:
    Element(): deleted(false), filled(false) {};
    explicit Element(const Key& key, const Value& val): key(key), val(val), filled(true), deleted(false) {}
    Key key;
    Value val;
    bool deleted;
    bool filled;
    bool operator == (const Element& other) {
        return key == other.key && val == other.val && deleted == other.deleted && filled == other.filled;
    }
    bool operator != (const Element& other) {
        return !(*this == other);
    }
};

template<typename Key, typename Value, typename Allocator = std::allocator<Element<Key, Value>>>
class HashMap{
public:
    template<bool isConst>
    friend class iterator;

    template<bool isConst>
    class iterator{
    public:
        friend class HashMap<Key, Value>;
        iterator(Element<Key, Value>* el, HashMap<Key, Value>* hm): to_(el), hashmap(hm) {
            while((!to_->filled || to_->deleted) && to_ != hashmap->data_.get() + hashmap->capacity_) {
                ++to_;
            }
        }
        iterator(const iterator<isConst>& other): to_(other.to_), hashmap(other.hashmap) {}

        iterator<isConst>& operator = (const iterator<isConst>& other) {
            if(this == std::addressof(other)) {
                return *this;
            }
            to_ = other.to_;
            hashmap = other.hashmap;
            return *this;
        }

        iterator<isConst>& operator ++ () {
            if(to_ == hashmap->data_.get() + hashmap->capacity_) {
                return *this;
            }
            do {
                ++to_;
            } while((!to_->filled || to_->deleted) && to_ != hashmap->data_.get() + hashmap->capacity_);
            return *this;
        }

        iterator<isConst> operator ++ (int) {
            auto copy = *this;
            if(to_ == hashmap->last_) {
                return copy;
            }
            ++*this;
            return copy;
        }

        iterator<isConst>& operator -- () {
            --to_;
            return *this;
        }

        iterator<isConst> operator -- (int) {
            auto copy = *this;
            --*this;
            return copy;
        }

        std::conditional_t<isConst, const Value&, Value&> operator * () {
            return to_->val;
        }

        std::conditional_t<isConst, const Value*, Value*> operator -> () {
            return std::addressof(to_->value);
        }

        bool operator == (const iterator& other) {
            return hashmap == other.hashmap && to_ == other.to_;
        }

        bool operator != (const iterator& other) {
            return hashmap != other.hashmap || to_ != other.to_;
        }

    private:
        Element<Key, Value>* ptr_() {
            return to_;
        }
        Element<Key, Value>* to_;
        HashMap<Key, Value>* hashmap;
    };

    using it = iterator<false>;
    using const_it = iterator<true>;

    it begin() {
        return it(data_.get(), this);
    }

    it end() {
        return it(data_.get() + capacity_, this);
    }

    const_it cbegin() {
        return const_it(data_.get(), this);
    }

    const_it cend() {
        return const_it(data_.get() + capacity_, this);
    }

    HashMap(size_t capacity = 8) : size_(0), capacity_(capacity),
                data_(std::allocator_traits<Allocator>::allocate(alloc_, capacity), [&](Element<Key, Value>* ptr){
                    for (size_t i = 0; i < size_; ++i){
                        std::allocator_traits<Allocator>::destroy(alloc_, ptr + i);
                    }
                    std::allocator_traits<Allocator>::deallocate(alloc_, ptr, capacity_);
                }) {
        for (size_t i = 0; i < capacity_; ++i){
            std::allocator_traits<Allocator>::construct(alloc_, data_.get() + i);
            //std::cout << " Constructed and empty element with number " << i << std::endl;
        }
    }

    HashMap<Key, Value>& operator = (const HashMap<Key, Value>& other) {
        if(this == std::addressof(other)) {
            return *this;
        }
        rehash_(other.capacity_);
        for (size_t i = 0; i < other.capacity_; ++i){
            insert_(other.data_[i]);
        }
        return *this;
    }

    void emplace_back(const Key& key, const Value& val) {
        if (size_ >= capacity_ / 2) {
            rehash_(capacity_ * 2);
        }
        size_t hash1 = HashFunc(key);
        size_t hash2 = std::hash<Key>{}(key);
        for (size_t i = 0; i < capacity_; ++i){
            size_t address = (hash1 + i * hash2) % capacity_;
            std::optional<size_t> has_found_deleted_cell;
            if (!data_[address].filled) {
                if(!has_found_deleted_cell) {
                    emplace_back_(address, key, val);
                    break;
                }
                else {
                    emplace_back_(has_found_deleted_cell.value(), key, val);
                    break;
                }
            }
            else {
                if(data_[address].deleted && !has_found_deleted_cell) {
                    has_found_deleted_cell = address;
                }
                if (data_[address].key == key) {
                    data_[address].val = val;
                    data_[address].deleted = false;
                    std::cout << "Override of existing deleted key" << std::endl;
                    break;
                }
            }
        }
    }



    Value get(const Key& key) {
        size_t hash1 = HashFunc(key);
        size_t hash2 = std::hash<Key>{}(key);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t address = (hash1 + i * hash2) % capacity_;
            if (!data_[address].filled) {
                std::cout << "Not exist " << std::endl;
                return 0;
            }
            else{
                if (data_[address].key == key) {
                    if (!data_[address].deleted) {
                        return data_[address].val;
                    }
                    else {
                        std::cout << "Key exist, but was deleted" << std::endl;
                        return 0;
                    }
                }
            }
        }
        std::cout << "Something wrong";
    }


    void erase(const Key& key) {
        size_t hash1 = HashFunc(key);
        size_t hash2 = std::hash<Key>{}(key);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t address = (hash1 + i * hash2) % capacity_;
            if(!data_[address].filled) {
                std::cout << "Does not exist or already deleted" << std::endl;
                break;
            }
            if(!data_[address].deleted && data_[address].key == key) {
                //std::cout << "It was " <<  data_[address].key << " Value " << data_[address].val << std::endl;
                data_[address].deleted = true;
                //std::cout << "Which became " <<  data_[address].filled << " Value " << data_[address].deleted << std::endl;
                break;
            }
        }
    }

    bool contains(const Key key) {
        size_t hash1 = HashFunc(key);
        size_t hash2 = std::hash<Key>{}(key);
        for (size_t i = 0; i < capacity_; ++i) {
            size_t address = (hash1 + i * hash2) % capacity_;
            if(!data_[address].filled) {
                return false;
            }
            else {
                if(data_[address].key == key) {
                    if(!data_[address].deleted) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }
        }
    }

    void clear() {
        HashMap<Key, Value> hm;
        data_ = std::move(hm.data_);
        capacity_ = hm.capacity_;
        size_ = hm.size_;
    }

    void print() {
        for (size_t i = 0; i < capacity_; ++i) {
            if(data_[i].filled && !data_[i].deleted) {
                std::cout << "Key " << data_[i].key << " and Value " << data_[i].val << std::endl;
            }
        }
    }

    void PrintFirstHash(const Key& key) {
        std::cout << HashFunc(key) % capacity_;
    }

    Value& operator [] (const Key& key) {
        return get(key);
    }

    bool operator == (const HashMap<Key, Value>& other) {
        if(capacity_ != other.capacity_) {
            return false;
        }
        for (size_t i = 0; i < other.capacity_; ++i){
            if(data_[i] != other.data_[i]) {
                return false;
            }
        }
        return true;
    }

private:
    void emplace_back_(const size_t& address, const Key& key, const Value& val){
        std::allocator_traits<Allocator>::construct(alloc_, data_.get() + address, key, val);
        ++size_;
        //std::cout << "Inserted Key " << key << " and Value " << val << std::endl;
        //std::cout << "Size = " << size_ << " Capacity = " << capacity_ << std::endl;
    }

    void insert_(const Element<Key, Value>& el) {
        size_t hash1 = HashFunc(el.key);
        size_t hash2 = std::hash<Key>{}(el.key);
        for (size_t i = 0; i < capacity_; ++i){
            size_t address = (hash1 + i * hash2) % capacity_;
            if (!data_[address].filled) {
                emplace_back_(address, el.key, el.val);
                data_[address].filled = el.filled;
                data_[address].deleted = el.deleted;
                break;
            }
            else {
                if (data_[address].key == el.key) {
                    data_[address].val = el.val;
                    data_[address].filled = el.filled;
                    data_[address].deleted = el.deleted;
                    break;
                }
            }
        }
    }

    void rehash_(size_t capacity) {
        HashMap<Key, Value> h (capacity);
        for (size_t i = 0; i < capacity_; ++i) {
            if(data_[i].filled && !data_[i].deleted) {
                h.insert_(data_[i]);
            }
        }
        data_ = std::move(h.data_);
        capacity_ = capacity;
    }

    std::unique_ptr<Element<Key, Value>[], std::function<void(Element<Key, Value>*)>> data_;
    Allocator alloc_;
    size_t size_;
    size_t capacity_;
};




int main() {
    HashMap<std::string, size_t> hm;
    HashMap<std::string, size_t> hm2;
    hm.emplace_back("C", 1);
    hm2.emplace_back("D", 1);
    std::cout << (hm == hm2) << std::endl;
    //hm.erase("C");
    //hm.get("C");
    hm.emplace_back("K", 5);
    hm.emplace_back("C", 2);
    hm.emplace_back("K", 3);
    hm.emplace_back("L", 4);
    hm.emplace_back("O", 4);
    hm.emplace_back("W", 4);
    //hm.clear();
    hm.print();
    //std::cout << hm.contains("C") << hm.contains("K");
    //hm.PrintFirstHash("E");
    //hm.PrintFirstHash("P");

    for (auto&& i : hm) {
        std::cout << i <<" ";
    }

    return 0;
}
