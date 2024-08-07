/*
* Assignment 2: HashMap template implementation (BASIC SOLUTION)
*      Notes: this file is what we call a .tpp file. It's not a .cpp file,
*      because it's not exactly a regular source code file. Instead, we are
*      defining the template definitions for our HashMap class.
*
*      TODO: write a comment here.
*
*      You'll notice that the commenting provided is absolutely stunning.
*      It was really fun to read through the starter code, right?
*      Please emulate this commenting style in your code, so your grader
*      can have an equally pleasant time reading your code. :)
*/

#include "hashmap.h"


template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap() : HashMap(kDefaultBuckets) { }

template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(size_t bucket_count, const H& hash) :
   _size(0),
   _hash_function(hash),
   _buckets_array(bucket_count, nullptr) { }

template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(std::initializer_list<value_type> init, size_t bucket_count, const H& hash) :
    HashMap(bucket_count, hash) {
    for(auto kv: init) {
        this->insert(kv);
    }
}

template <typename K, typename M, typename H>
template <typename InputIt>
HashMap<K, M, H>::HashMap(InputIt start, InputIt end, size_t bucket_count, const H& hash) :
    HashMap(bucket_count, hash) {
    for(auto it = start;it != end;++it)
        this->insert(*it);
}

template <typename K, typename M, typename H>
HashMap<K, M, H>::~HashMap() {
   clear();
}

template <typename K, typename M, typename H>
inline size_t HashMap<K, M, H>::size() const noexcept {
   return _size;
}

template <typename K, typename M, typename H>
inline bool HashMap<K, M, H>::empty() const noexcept {
   return size() == 0;
}

template <typename K, typename M, typename H>
inline float HashMap<K, M, H>::load_factor() const noexcept {
   return static_cast<float>(size())/bucket_count();
};

template <typename K, typename M, typename H>
inline size_t HashMap<K, M, H>::bucket_count() const noexcept {
   return _buckets_array.size();
};

template <typename K, typename M, typename H>
bool HashMap<K, M, H>::contains(const K& key) const noexcept {
   return find_node(key).second != nullptr;
}

template <typename K, typename M, typename H>
void HashMap<K, M, H>::clear() noexcept {
   for (auto& curr : _buckets_array) {
       while (curr != nullptr) {
           auto trash = curr;
           curr = curr->next;
           delete trash;
       }
   }
   _size = 0;
}

template <typename K, typename M, typename H>
std::pair<typename HashMap<K, M, H>::value_type*, bool>
           HashMap<K, M, H>::insert(const value_type& value) {
   const auto& [key, mapped] = value;
   auto [prev, node_to_edit] = find_node(key);
   size_t index = _hash_function(key) % bucket_count();

   if (node_to_edit != nullptr) return {&(node_to_edit->value), false};
   _buckets_array[index] = new node(value, _buckets_array[index]);

   ++_size;
   return {&(_buckets_array[index]->value), true};
}

template <typename K, typename M, typename H>
M& HashMap<K, M, H>::at(const K& key) {
   auto [prev, node_found] = find_node(key);
   if (node_found == nullptr) {
       throw std::out_of_range("HashMap<K, M, H>::at: key not found");
   }
   return node_found->value.second;
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::node_pair HashMap<K, M, H>::find_node(const K& key) const {
   size_t index = _hash_function(key) % bucket_count();
   auto curr = _buckets_array[index];
   node* prev = nullptr; // if first node is the key, return {nullptr, front}
   while (curr != nullptr) {
       const auto& [found_key, found_mapped] = curr->value;
       if (found_key == key) return {prev, curr};
       prev = curr;
       curr = curr->next;
   }
   return {nullptr, nullptr}; // key not found at all.
}

template <typename K, typename M, typename H>
void HashMap<K, M, H>::debug() const {
   std::cout << std::setw(30) << std::setfill('-') << '\n' << std::setfill(' ')
             << "Printing debug information for your HashMap implementation\n"
             << "Size: " << size() << std::setw(15) << std::right
             << "Buckets: " << bucket_count() << std::setw(20) << std::right
             << "(load factor: " << std::setprecision(2) << load_factor() << ") \n\n";

   for (size_t i = 0; i < bucket_count(); ++i) {
       std::cout << "[" << std::setw(3) << i << "]:";
       auto curr = _buckets_array[i];
       while (curr != nullptr) {
           const auto& [key, mapped] = curr->value;
           // next line will not compile if << not supported for K or M
           std::cout <<  " -> " << key << ":" << mapped;
           curr = curr->next;
       }
       std::cout <<  " /" <<  '\n';
   }
   std::cout << std::setw(30) << std::setfill('-') << '\n';
}

template <typename K, typename M, typename H>
bool HashMap<K, M, H>::erase(const K& key) {
    auto [prev, node_to_erase] = find_node(key);
    if (node_to_erase == nullptr) {
        return false;
    } else {
        size_t index = _hash_function(key) % bucket_count();
        (prev ? prev->next : _buckets_array[index]) = node_to_erase->next;
        delete node_to_erase;
        --_size;
        return true;
    }
}

template <typename K, typename M, typename H>
void HashMap<K, M, H>::rehash(size_t new_bucket_count) {
    if (new_bucket_count == 0) {
        throw std::out_of_range("HashMap<K, M, H>::rehash: new_bucket_count must be positive.");
    }

    std::vector<node*> new_buckets_array(new_bucket_count);
    // Hint: you should NOT call insert, and you should not call
    // new or delete in this function. You must reuse existing nodes.
    for(auto& cur: _buckets_array) {
        while(cur != nullptr) {
            const auto& [key, value] = cur->value;
            size_t newIdx = _hash_function(value) % new_bucket_count; // relocate
            // insert a node into the new bucket array at head

            node* temp = cur;
            cur = cur->next;
            temp->next = new_buckets_array[newIdx];
            new_buckets_array[newIdx] = temp; // renew the head
        }
    }
    _buckets_array = std::move(new_buckets_array); // transfer the ownership
}

/*
    Milestone 2-3: begin student code

    Here is a list of functions you should implement:
    Milestone 2
        - operator[]
        - operator<<
        - operator== and !=
        - make existing functions const-correct

    Milestone 3
        - copy constructor
        - copy assignment
        - move constructor
        - move assignment
*/
template <typename K, typename M, typename H>
M& HashMap<K, M, H>::operator [](const K& key) {
    return insert({key, {}}).first->second;
}

template <typename K, typename M, typename H>
const M& HashMap<K, M, H>::at(const K &key) const {
    return static_cast<const M&>(const_cast<HashMap<K, M, H>*>(this)->at(key));
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::node* HashMap<K, M, H>::firstKV() {
    if(_size == 0)
        return nullptr;
    for(auto cur: _buckets_array) {
        if(cur != nullptr)
            return cur;
    }
    return nullptr;
}
/*
 * Without Iterator class
template <typename K, typename M, typename H>
std::ostream& operator<<(std::ostream& out, const HashMap<K, M, H>& map) {
    out << "{";
    size_t count = 0;
    for(auto cur: map._buckets_array) {
        while(cur != nullptr) {
            if(++count != map._size)
                out << cur->value.first << ":" << cur->value.second << ", ";
            else
                out << cur->value.first << ":" << cur->value.second;
            cur = cur->next;
        }
    }
    out << "}";
    return out;
}

template <typename K, typename M, typename H>
bool operator==(const HashMap<K, M, H>& lhs, const HashMap<K, M, H>& rhs) {
    if(lhs.size() != rhs.size())
        return false;
    for(auto cur: lhs._buckets_array) {
        while(cur != nullptr) {
            const auto& [key, value] = cur->value;
            auto correspond = rhs.find_node(key).second;
            if(correspond == nullptr || correspond->value.second != value)
                return false;
            cur = cur->next;
        }
    }

    return true;
}

template <typename K, typename M, typename H>
bool operator!=(const HashMap<K, M, H>& lhs, const HashMap<K, M, H>& rhs) {
    return !(lhs == rhs);
}
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(const HashMap<K, M, H>& map) :
    HashMap(map.bucket_count(), map._hash_function) {
    for(auto cur: map._buckets_array) {
        while(cur != nullptr) {
            this->insert(cur->value);
            cur = cur->next;
        }
    }
}

template <typename K, typename M, typename H>
HashMap<K, M, H>& HashMap<K, M, H>::operator=(const HashMap<K, M, H>& map) {
    if(this != &map) {
        this->clear();
        _hash_function = map._hash_function;
        _buckets_array.resize(map.bucket_count(), nullptr);
        for(auto cur: map._buckets_array) {
            while(cur != nullptr) {
                this->insert(cur->value);
                cur = cur->next;
            }
        }
    }
    return *this;
}

template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(HashMap<K, M, H>&& map) noexcept:
    _size(std::move(map._size)),
    _hash_function(std::move(map._hash_function)),
    _buckets_array(map.bucket_count(), nullptr) {
    for(size_t i = 0;i < map.bucket_count();++i) {
        _buckets_array[i] = std::move(map._buckets_array[i]);
        map._buckets_array[i] = nullptr;
    }
    map._size = 0;
}

template <typename K, typename M, typename H>
HashMap<K, M, H>& HashMap<K, M, H>::operator=(HashMap<K, M, H>&& map) noexcept {
    if(this != &map) {
        this->clear();
        _size = std::move(map.size());
        _hash_function = std::move(map._hash_function);
        _buckets_array.resize(map.bucket_count(), nullptr);
        for(size_t i = 0;i < map.bucket_count();++i) {
            _buckets_array[i] = std::move(map._buckets_array[i]);
            map._buckets_array[i] = nullptr;
        }
    }
    return *this;
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::iterator HashMap<K, M, H>::make_iterator(node *ptr) {
    if(ptr == nullptr)
        return {&_buckets_array, ptr, bucket_count()};
    return {&_buckets_array, ptr, _hash_function(ptr->value.first) % bucket_count()};
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::iterator HashMap<K, M, H>::begin() {
    return this->make_iterator(this->firstKV());
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::const_iterator HashMap<K, M, H>::begin() const {
    return static_cast<const_iterator>(const_cast<HashMap<K, M, H>*>(this)->begin());
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::iterator HashMap<K, M, H>::end() {
    return make_iterator(nullptr);
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::const_iterator HashMap<K, M, H>::end() const {
    return static_cast<const_iterator>(const_cast<HashMap<K, M, H>*>(this)->end());
}

/*
 * Iterator Deployed Version
 */
template <typename K_, typename M_, typename H_>
std::ostream& operator<<(std::ostream& out, const HashMap<K_, M_, H_>& map) {
    std::ostringstream oss("{", std::ostringstream::ate);
    for(const auto& [key, value]: map) {
        oss << key << ":" << value << ", ";
    }
    std::string s = oss.str();
    out << s.substr(0, s.length() - 2) << "}";
    return out;
}

template <typename K, typename M, typename H>
bool operator==(const HashMap<K, M, H>& lhs, const HashMap<K, M, H>& rhs) {
    if(lhs.size() != rhs.size())
        return false;
    for(const auto& [key, value]: lhs) {
        const auto& correspond = rhs.find_node(key).second;
        if(correspond == nullptr || correspond->value.second != value)
            return false;
    }
    return true;
}

template <typename K, typename M, typename H>
bool operator!=(const HashMap<K, M, H>& lhs, const HashMap<K, M, H>& rhs) {
    return !(lhs == rhs);
}

