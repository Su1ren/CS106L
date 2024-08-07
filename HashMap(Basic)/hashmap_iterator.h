#ifndef HASHMAPITERATOR_H
#define HASHMAPITERATOR_H
#include <functional>
#include <iterator>
/* Milestone 6 (optional): implement iterators for your HashMap class */

template <typename K, typename M, typename H> class HashMap;

/*
 * Template class for a HashMapIterator
 * Map = type of HashMap this class is an iterator for.
 * IsConst = whether this is a const_iterator class.
 *
 * Using meta template programming.
 * Concept requirements:
 * - Map must be a valid claa HashMap<K, M, H>
 */
template<typename Map, bool IsConst = true>
class HashMapIterator {
private:
    /*
     * First we should consider what a iterator consists of.
     * Traversal requires a pointer-like val, knowing the border of traversal.
     * So a iterator can be constructed with a node in HashMap, a bucket array pointer to spread the stage,
     * and the size of bucket array to determine the range.
     */
    using node = typename Map::node;
    using bucket_array_type = decltype(Map::_buckets_array);

    bucket_array_type* _buckets_array; // the HashMap pointer of which the iterator is iterating
    node* _node; // the pointer stores the element is pointing to
    size_t _bucket; // which bucket the _node resides in

    /*
     * Private constructor for a Iterator
     * Friend class can access the constructor, like begin() in HashMap.
     * Setting ctor as private also prevents user from abusing ctor.
     */
    HashMapIterator(bucket_array_type* _buckets_array, node* _node, size_t bucket);

public:
    // conditional compile, if the template parameter is true, choose the second operand, otherwise the third.
    using value_type = std::conditional_t<IsConst, const typename Map::value_type, typename Map::value_type>;

    /*
     * Public aliases for the iterator class. So that STL functions like std::iterator_traits
     * can parse this class like its iterator category.
     *
     * For HashMapIterator, we tag it as forward iterator.
     */
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*; // for dereference
    using reference = value_type&; // for derefernce

    /*
     * For both const_iterator and iterator, we should declare them as friend.
     */
    friend Map;
    friend class HashMapIterator<Map, true>;
    friend class HashMapIterator<Map, false>;

    /*
     * Conversion operator: converts any iterator (iterator or const_iterator) to a const_iterator.
     *
     * Usage:
     *      iterator iter = map.begin();
     *      const_iterator c_iter = iter;    // implicit conversion
     *
     * Implicit conversion operators are usually frowned upon, because they can cause
     * some unexpected behavior. This is a rare case where a conversion operator is
     * very convenient. Many of the iterator functions in the HashMap class are
     * secretly using this conversion.
     *
     * Note: conversion in the opposite direction (const to non-const) is not safe
     * because that gives the client write access the map itself is const.
     */
    operator HashMapIterator<Map, true>() const {
        return HashMapIterator<Map, true>(_buckets_array, _node, _bucket);
    }

    /*
     * Dereference operators: defines the behavior of dereferencing an iterator.
     *
     * Usage:
     *      auto [key, value] = *iter;
     *      auto value = iter->second;
     *      iter->second = 3; // if iter is a regular iterator (not a const_iterator)
     *
     * Note that dereferencing an invalid or end() iterator is undefined behavior.
     */
    reference operator*() const;
    pointer operator->() const;

    /*
     * Increment operators: moves the iterator to point to the next element, or end().
     *
     * Usage:
     *      ++iter;         // prefix
     *      iter++;         // postfix
     *
     * Note: the prefix operator first increments, and the returns a reference to itself (which is incremented).
     * The postfix operator returns a copy of the original iterator, while the iterator itself is incremented.
     *
     * Note that incrementing an invalid or end() iterator is undefined behavior.
     */
    HashMapIterator<Map, IsConst>& operator++();
    HashMapIterator<Map, IsConst> operator++(int);

    /*
     * Equality operator: decides if two iterators are pointing to the same element.
     *
     * Usage:
     *      if (iter == map.end()) {...};
     * Supposed to be a friend member function.
     */
    template<typename Map_, bool IsConst_>
    friend bool operator==(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs);

    /*
     * Inequality operator: decides if two iterators are pointing to different elements.
     *
     * Usage:
     *      if (iter != map.end()) {...};
     */
    template<typename Map_, bool IsConst_>
    friend bool operator!=(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs);

    /*
     * Special member functions: we explicitly state that we want the default compiler-generated functions.
     * Here we are following the rule of zero. You should think about why that is correct.
     */
    HashMapIterator(const HashMapIterator<Map, IsConst>& rhs) = default;
    HashMapIterator<Map, IsConst>& operator=(const HashMapIterator<Map, IsConst>& rhs) = default;

    HashMapIterator(HashMapIterator<Map, IsConst>&& rhs) = default;
    HashMapIterator<Map, IsConst>& operator=(HashMapIterator<Map, IsConst>&& rhs) = default;
};

template <typename Map, bool IsConst>
HashMapIterator<Map, IsConst>::HashMapIterator(bucket_array_type* _buckets_array, node* _node, size_t bucket):
    _buckets_array(_buckets_array),
    _node(_node),
    _bucket(bucket) {

}

template <typename Map, bool IsConst>
typename HashMapIterator<Map, IsConst>::reference HashMapIterator<Map, IsConst>::operator*() const {
    return this->_node->value;
}

template <typename Map, bool IsConst>
typename HashMapIterator<Map, IsConst>::pointer HashMapIterator<Map, IsConst>::operator->() const {
    return &(this->_node->value); // return a pointer taking the _node's address
}

template <typename Map, bool IsConst>
HashMapIterator<Map, IsConst>& HashMapIterator<Map, IsConst>::operator++() {
    _node = _node->next;
    if(_node == nullptr) {
        for(++_bucket;_bucket < _buckets_array->size();++_bucket) {
            _node = (*_buckets_array)[_bucket];
            if(_node != nullptr)
                break;
        }
    }

    return *this;
}

template <typename Map, bool IsConst>
HashMapIterator<Map, IsConst> HashMapIterator<Map, IsConst>::operator++(int) {
    HashMapIterator<Map, IsConst> re = *this;
    ++(*this);
    return re;
}

template <typename Map_, bool IsConst_>
bool operator==(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs) {
    return lhs._node == rhs._node;
}

template <typename Map_, bool IsConst_>
bool operator!=(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs) {
    return !(lhs == rhs);
}
// "White. A blank page or canvas. His favorite. So many possibilities...""


#endif // HASHMAPITERATOR_H
