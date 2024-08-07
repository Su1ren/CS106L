# 介绍
完成了简单版的 HashMap，现在我们来正式进入完整版。相比之下完整版的编程内容要多出很多，包括：

- 再散列的实现
- 自己实现 4 种运算符重载
- 初始化列表构造和范围构造的实现
- 实现迭代器类
- 用迭代器类简化重载运算符

另外还有一些基于现代 C++ 的拓展，其中的难度可能相当大，我打算另开一篇来逐步实现；这里我们只需要实现 HashMap 的基本功能即可。
## 文件说明
完整版的文件与之前有所不同，简单介绍如下：

- hashmap.h：包含了最简单的 HashMap 类原型，以及 rehash 函数的原型
- hashmap.cpp：包含了 HashMap 部分方法的实现，以及 rehash 函数的框架
- student_main.cpp：当 test_harness 关闭时，student_main 将被执行。这个可以用来执行自己编写的测试程序
- test_settings.cpp：包含了 test_harness 宏，开启时，将会使用 tests.cpp 中的样例。同时也可以关闭部分测试样例，在执行时会被跳过，可以规避未实现方法导致的编译错误。
- tests.cpp：该文件包含每个测试的代码，可以加上一些 map.debug 语句，也可以额外加入自己编写的测试用例。
# 实现
## 步骤
我们的实现过程分为五步：

1. 实现完整的再散列
2. 重载 4 种运算符，检查 const 限制
3. 实现拷贝和移动语义
4. 实现初始化列表和范围构造
5. 实现迭代器类

其中第三步和第二步的后半属于简单版已经完成的内容，最复杂的是实现迭代器。
## 任务一：再散列
要求：rehash 函数不能分配新内存空间、也不能导致内存泄漏，而且必须复用现有代码。相关测试如下：

- Test 1A：检查 rehash 的外部正确性，即不同 rehash 的参数 bucket size 下 HashMap 是否仍能正常工作。需要利用 insert 和 erase 方法。在输入不合法时，会抛出异常，例如 rehash(0)。
- Test 1B：检查 rehash 的内部逻辑。思想为 buckets 中的链表越长，则搜索的耗时越长。也就是检查在不同长度下搜索的用时，是否满足规律。但是由于个人 PC 的性能各异，很小概率会出现测试不通过的情况。

函数原型如下：
```cpp
/*
    * Resizes the array of buckets, and rehashes all elements. new_buckets could
    * be larger than, smaller than, or equal to the original number of buckets.
    *
    * Parameters: new_buckets - the new number of buckets. Must be greater than 0.
    * Return value: none
    *
    * Usage:
    *      map.rehash(30)
    *
    * Exceptions: std::out_of_range if new_buckets = 0.
    *
    * Complexity: O(N) amortized average case, O(N^2) worst case, N = number of elements
    *
    * Notes: our minimal HashMap implementation does not support automatic rehashing, but
    * std::unordered_map will automatically rehash, even if you rehash to
    * a very small number of buckets. For this reason, std::unordered_map.rehash(0)
    * is allowed and forces an unconditional rehash. We will not require this behavior.
    * If you want, you could implement this.
    *
    * This function is incomplete, and for milestone 1 you should complete the implementation.
    * The test cases provided use a probabilistic time test. If the test fails very occasionally,
    * that is totally fine, as long as most times you run the test cases, it passes that test.
    */
void rehash(size_t new_buckets);
```
可以看出，rehash 函数会修改当前的 buckets 大小，而 buckets array 实际上是一个保存了结点的动态数组。
给出实现如下：
```cpp
template <typename K, typename M, typename H>
void HashMap<K, M, H>::rehash(size_t new_bucket_count) {
    if (new_bucket_count == 0) {
        throw std::out_of_range("HashMap<K, M, H>::rehash: new_bucket_count must be positive.");
    }

    std::vector<node*> new_buckets_array(new_bucket_count);
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
```
由于电脑硬件的迭代，4 年前的参数指标已经不能适配了，每次执行测试 1B 都会失败。不过有时失败的语句不同。
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722849043561-f634e9f8-a382-4a96-b485-4970bc07b257.png#averageHue=%23f6f4f4&clientId=u38d08a0d-5b1e-4&from=paste&height=89&id=u8c74b1cd&originHeight=133&originWidth=1688&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=13997&status=done&style=none&taskId=u1c74fb3e-a464-499d-b258-9570d48e023&title=&width=1125.3333333333333) ![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722849065582-ef346bea-59ea-459b-8f07-1b069248e9f4.png#averageHue=%23f3f1f0&clientId=u38d08a0d-5b1e-4&from=paste&height=67&id=u641fb916&originHeight=101&originWidth=1677&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=13488&status=done&style=none&taskId=uda4cf28a-3e71-4cfc-9048-a7f2ac649c2&title=&width=1118)
## 任务二：运算符重载和 const 检查
需要重载的运算符有：

- []
- <<
- == 和 !=

最后是 const 检查，需要判断哪些函数被限定为 const，这与简单的基本相同。
[]、<< 、==、!=、at 的 const 重载版本如下：
```cpp
template <typename K, typename M, typename H>
const M& HashMap<K, M, H>::at(const K &key) const {
    return static_cast<const M&>(const_cast<HashMap<K, M, H>*>(this)->at(key));
}

template <typename K, typename M, typename H>
M& HashMap<K, M, H>::operator [](const K& key) {
    return insert({key, {}}).first->second;
}

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
```
不得不说这个作业的设置指示确实很含糊，这个输出重载格式甚至需要自己去测试样例里面模仿。而且没有迭代器的容器确实很难用。
测试结果：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722860266741-26b02589-3f07-4b5e-a693-cbdea05c37dd.png#averageHue=%23edeae9&clientId=ud61ea492-1ee8-4&from=paste&height=116&id=ud007464a&originHeight=174&originWidth=640&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=14563&status=done&style=none&taskId=u5e8960b3-e38f-4c4d-8e86-c7aeb8ba34c&title=&width=426.6666666666667)
## 任务三：拷贝和移动语义
这一部分的内容我们之前完成过，这里可以快速过一遍。
不过这里不能使用迭代器。
```cpp
4template <typename K, typename M, typename H>
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
```
测试结果：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722864349663-b8fc8edd-6d1b-412e-b91d-d6c298f2ebc3.png#averageHue=%23edebe9&clientId=u4159576e-99b9-4&from=paste&height=171&id=u15c780d9&originHeight=257&originWidth=638&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=22155&status=done&style=none&taskId=ud8f1f437-9b09-4554-bd9f-7ac18920d13&title=&width=425.3333333333333)
移动构造耗时竟然为 0。
## 任务四：初始化列表和范围构造
第四步我们要实现 HashMap 的初始化列表构造方法，以及范围构造。
单纯的初始化列表还是非常简单的，注意范围构造时，输入迭代器使用另一种模板参数。这个任务就是了解 initializer_list 和迭代器范围初始化的编写规范。
```cpp
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
```
测试结果：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722868034564-9e86141c-1936-4c84-9943-79059ece638c.png#averageHue=%23f1efee&clientId=u4159576e-99b9-4&from=paste&height=85&id=u3760f9c7&originHeight=128&originWidth=660&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=8572&status=done&style=none&taskId=u999b3c43-87b0-4def-a5ce-cc7a57461c9&title=&width=440)
## 任务五：迭代器类
现在我们要完成最复杂的一部分了，也就是给 HashMap 实现了一个友元类：迭代器类，设置为友元类可以方便对私有成员的读取。
除此之外还有常迭代器，const Iterator、const const_Iterator。为了减低代码重复率，可能需要用到类型萃取、模板元编程，我们在简单版已经看过了一些。之后就可以返回更新一些方法，大幅简化其代码。
迭代器类的内容非常多，我们将其拆分为几个小部分。
### 类模板参数
首先我们要明确一点，迭代器是必须和匹配的容器类使用的，在成员方法的调用中产生和使用，不能由用户自己显式调用默认构造。
所以，类模板的第一参数设定为对应的 HashMap 类，以下简写为 Map。
然后是迭代器和常迭代器的区分，我们可以通过模板元编程，类模板参数来设定，例如：
`template<typename Map, bool IsConst = true> class HashMapIterator;`
这样在 HashMap 类定义中就可以利用模板参数 IsConst 来区分迭代器和常迭代器了。
`using iterator = HashMapIterator<HashMap, false>;`
`using const_iterator = HashMapIterator<HashMap, true>;`
为了方便迭代器和 HashMap 类的私有数据相互读取，我们将其互设为友元类。
最后是值类型的问题。对于常迭代器，值类型也必须对应为常类型。我们可以通过条件编译 `std::conditional_t` 来实现。
这样可以得到整个迭代器类模板的框架：
```cpp
template <typename Map, bool IsConst = true>
class HashMapIterator {
public；
    friend Map;
    friend class HashMapIterator<Map, true>;
    friend class HashMapIterator<Map, false>;

    using value_type = std::conditional_t<IsConst, const typename Map::value_type, typename Map::value_type>;
private:
    HashMapIterator(...); // constructor
};
```
注意，类模板的定义内部，为了方便在隐式转型（迭代器转常迭代器）时，两种不同迭代器类的成员读取，将两种迭代器彼此设为友元类。
### 成员变量
下面我们要考虑，一个迭代器对象无论是否为常，应该包含哪些成员。
首先我们知道迭代器主要是用来遍历容器的，依附于容器的类型而存在；而且具体类的迭代器不能作用于其他类。而我们使用的拉链法 HashMap 承载键值对的对象是 `std::vector<node*>`。因此我们可以用一个 `std::vector<node*>` 类型的指针，表明迭代器作用的对象。
接着，如何实现迭代器的遍历呢？我们可以效仿指针遍历字符串，由于装载键值对的是 `node`，因此我们可以用一个 `node*` 表示迭代器当前指向的键值对。
最后我们还需要一个类似于索引的变量，用来标记当前迭代器遍历到了 bucket array 的哪个桶中。你可能会想，这有什么用？实际上，我们需要标记迭代器所在桶的位置，是为了方便在当前桶走到尽头时，向下一个有效键值对位置的查询和转移。
综上，我们设置成员变量并完整构造函数：
```cpp
template <typename Map, bool IsConst = true>
class HashMapIterator {
private:
    using bucket_array_type = decltype(Map::_buckets_array);
    using node = typename Map::node;

    bucket_array_type* _buckets_array;
    node* _node;
    size_t bucket;

    HashMapIterator(bucket_array_type* _buckets_array, node* _node, size_t bucket);
};

template <typename Map, bool IsConst>
HashMapIterator<Map, IsConst>::HashMapIterator(bucket_array_type* _buckets_array, node* _node, size_t bucket):
    _buckets_array(_buckets_array),
    _node(_node),
    _bucket(bucket) {}
```
### 类型萃取
为了让我们自己编写的迭代器类能够被 STL 函数正确识别为迭代器，我们需要加上一些类型萃取有关的声明。
简单起见，我们将迭代器设为单向迭代器，对应的 `iterator_category` 就是 `std::forward_iterator_tag`。此外还需要显式指出迭代器运算结果的类型、指针和引用类型，在解引用运算时与迭代器类本身的引用区分开来。
需要声明的相关别名如下：
```cpp
template <typename Map, bool IsConst = true>
class HashMapIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*; // for dereference
    using reference = value_type&; // for derefernce
};
```
### 拷贝和移动语义
从构造函数可看出，我们的迭代器类并不涉及动态内存管理，所以遵循 Rule of Zero，拷贝和移动语义都可以使用编译器默认合成的方法。
```cpp
template <typename Map, bool IsConst = true>
class HashMapIterator {
public:
    HashMapIterator(const HashMapIterator<Map, IsConst>& rhs) = default;
    HashMapIterator<Map, IsConst>& operator=(const HashMapIterator<Map, IsConst>& rhs) = default;

    HashMapIterator(HashMapIterator<Map, IsConst>&& rhs) = default;
    HashMapIterator<Map, IsConst>& operator=(HashMapIterator<Map, IsConst>&& rhs) = default;
};
```
### 运算符重载
现在对于我们的单向迭代器 HashMapIterator，我们需要考虑重载以下运算符：

- 解引用：*、->
- 类型转换：只实现迭代器到常迭代器的转换，后者到前者的转换不安全
- 自增运算符：分为前缀后缀
- 逻辑运算：比较两个迭代器指向的元素是否相同或不同

对于返回结果，会用到之前声明的 `pointer` 和 `reference`。
```cpp
template <typename Map, bool IsConst = true>
class HashMapIterator {
public:
    operator HashMapIterator<Map, true>() const {
        return HashMapIterator<Map, true>(_buckets_array, _node, _bucket);
    }

    reference operator*() const;
    pointer operator->() const;

    HashMapIterator<Map, IsConst>& operator++();
    HashMapIterator<Map, IsConst> operator++(int);

    template<typename Map_, bool IsConst_>
    friend bool operator==(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs);

    template<typename Map_, bool IsConst_>
    friend bool operator!=(const HashMapIterator<Map_, IsConst_>& lhs, const HashMapIterator<Map_, IsConst_>& rhs);
};

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
    _node = _node->next; // next node in the bucket
    if(_node == nullptr) { // not valid node, seek from the next bucket
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
```
### 补充声明
再回到 HashMap 的类声明文件中，补充关于迭代器的声明和定义如下。
```cpp
template <typename K, typename M, typename H = std::hash<K>>
class HashMap {
public:
    friend class HashMapIterator<HashMap, true>;
    friend class HashMapIterator<HashMap, false>;

    using iterator = HashMapIterator<HashMap, false>;
    using const_iterator = HashMapIterator<HashMap, true>;
};
```
### 调用迭代器
现在我们在 HashMap 内部声明了两种迭代器，但是要如何通过 HashMap 类对象调用迭代器呢？好像还没有接口能够调用。
接下来我们还要实现 `begin` 和 `end` 方法，来获取迭代容器的起止位置，以及它们的常方法重载版本。
`iterator begin();`
`const_iterator begin() const;`
`iterator end();`
`const_iterator end() const;`
这样又引出了另一个问题，如何实现迭代器的构造。我们在迭代器类定义中，将其构造函数声明为私有的。虽然 HashMap 与其互为友元类，但是每次调用构造函数比较麻烦。由于 HashMap 有一个成员 _buckets_array，而因此我们可以在 HashMap 类中加上一个私有的成员函数，专门用来将 `node*` 封装成迭代器。
对于单向迭代器而言，迭代器的构造只可能出现在 begin 和 end 两个位置。
end 非常简单，指向容器末尾后的位置，所以可以用 nullptr 封装；但是对于 begin，我们需要找到表中第一个有效的键值对节点，也就是通过遍历 vector 来找到第一个非空节点。我们将其封装为另一个函数。
整体的代码如下：
```cpp
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

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::iterator HashMap<K, M, H>::make_iterator(node *ptr) {
    if(ptr == nullptr) // if node is nullptr
        return {&_buckets_array, ptr, bucket_count()}; // as if in the [bucket_count()] bucket
    return {&_buckets_array, ptr, _hash_function(ptr->value.first) % bucket_count()}; // not nullptr, hash and get its bucket No.
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

```
以上代码都实现之后，测试第六组应该可以全部通过了，附上结果图：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722962332815-160f539b-2bbc-400d-8ab2-05cbcb03d4ad.png#averageHue=%23edebe9&clientId=u0b85c600-2d8f-4&from=paste&height=170&id=u4ff16edc&originHeight=255&originWidth=644&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=20256&status=done&style=none&taskId=uadf788d4-96c1-4d4c-809b-8ed73adc994&title=&width=429.3333333333333)
## 任务六：简化运算符重载
现在我们实现并验证了迭代器类的健壮性，遍历容器的复杂度大大降低了，可以用来简化之前复杂的 <<、== 和 != 运算符重载代码实现。
```cpp
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
```
这里用到了一点技巧：虽然我们不能截断 ostream，但是可以用 ostringstream 缓冲字符串，然后将其输出到字符串中，对字符串可以取其子串输出到 ostream，再补上 `}`。
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722964188077-c1ef26c5-7adc-4591-b898-aa199649b8ef.png#averageHue=%23edebea&clientId=u38e30f9b-bf05-4&from=paste&height=123&id=u48ff651c&originHeight=184&originWidth=633&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=14618&status=done&style=none&taskId=u02312016-a70d-4c83-9c18-b10fc406298&title=&width=422)
# 参考文章

- [https://blog.51cto.com/u_15346415/3674275](https://blog.51cto.com/u_15346415/3674275)
- [https://github.com/PKUFlyingPig/CS106L/blob/master/assignments/HashMap/](https://github.com/PKUFlyingPig/CS106L/blob/master/assignments/HashMap/)
- [https://www.cnblogs.com/youxin/p/3281107.html](https://www.cnblogs.com/youxin/p/3281107.html)
