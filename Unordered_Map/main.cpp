#include <iostream>
#include <list>
#include <vector>
#include <functional>
#include <string>
#include <string_view>
 
size_t my_hash(const std::string& s) {
    size_t result = 0;
    size_t p = 3;
    for (size_t i = 0; i < s.size() && i < 30; ++i) {
        result *= p;
        result += s[i];
    }
    return result;
}
 
template <typename KeyT, typename ValueT>
class UnorderedMap {
    class BucketElement {
    public:
        BucketElement(KeyT k, ValueT v) : key(k), val(v) {}
        KeyT key;
        ValueT val;
    };
public:
    UnorderedMap() : data_(8, std::list<BucketElement>()), inserted_(0) {
    }
 
    void insert(KeyT key, ValueT val) {
        if (inserted_ > data_.size() / 2) {
            rehash(2 * data_.size());
        }
        if (!contains(key)) {
            insert_(data_, key, val);
            ++inserted_;
        }
    }
 
    bool contains( KeyT key) {
        size_t hash = my_hash(key) % data_.size();
        return std::find_if(
                data_[hash].begin(),
                data_[hash].end(),
                [&key](const BucketElement& el) {
                     return key == el.key;
                }) != data_[hash].end();
    }
 
    void remove(KeyT key) {
        size_t hash = my_hash(key) % data_.size();
        auto it = std::find_if(
                data_[hash].begin(),
                data_[hash].end(),
                [&key](const BucketElement& el) {
                    return key == el.key;
                });
        if (it != data_[hash].end()) {
            data_[hash].erase(it);
            --inserted_;
        }
    }
 
    ValueT& get(KeyT key) {
        size_t hash = my_hash(key) % data_.size();
        return std::find_if(
                data_[hash].begin(),
                data_[hash].end(),
                [&key](const BucketElement& el) {
                    return key == el.key;
                })->val;
    }
 
    void clear() {
        inserted_ = 0;
        data_ = {8, {}};
    }
 
private:
 
    static void insert_(
            std::vector<std::list<BucketElement>>& data,
            const KeyT& key,
            const ValueT& val) {
        size_t hash = my_hash(key) % data.size();
        data[hash].emplace_back(key, val);
    }
 
    void rehash(size_t n) {
        std::vector<std::list<BucketElement>> new_data(n, std::list<BucketElement>());
        for (auto&& bucket : data_) {
            for (auto&& element : bucket) {
                insert_(new_data, element.key, element.val);
            }
        }
        data_ = std::move(new_data);
    }
    std::vector<std::list<BucketElement>> data_;
    size_t inserted_;
};
 
int main() {
    UnorderedMap<std::string, std::string> m;
    std::string s = "j";
    std::string s2 = "o";
    m.insert(s, s2);
    std::cout << m.get("j");
    m.get("j") = "1000";
    std::cout << m.get("j");
    for (int i = 0; i < 10000; ++i) {
        m.insert(std::to_string(i), "a");
    }
 
    std::cout << m.get("999");
    m.remove("999");
    if (!m.contains("999")) {
        std::cout << "DELETED";
    }
 
    return 0;
}
