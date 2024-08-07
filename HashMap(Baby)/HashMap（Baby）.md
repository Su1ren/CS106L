# 介绍
在 STL 中，Map 这种数据结构一共存在两种实现方式：std::map 使用了 BST，std::unordered_map 则是使用了哈希表，将大多数映射操作的时间复杂度控制在常数级。
本作业中，我们给出一个接近完整的 HashMap 实现，但是它不支持拷贝和移动语义，因此还不算 STL 级别的抽象。我们的目标，就是完善 HashMap，让其足够高性能，足够健壮。
## HashMap 原理
HashMap 使用的数据结构非常复杂，可以概括为：

- hashmap 有一个成员序列，用 vector 存储，作为桶。
- 每个桶都是一个链表，链表由节点组成，定义在 hashmap.h 中。
- 每个节点包含一个键值对，表示为 std::pair<K, M>，是 hashmap 的 value_type 类型变量。
- 当用户使用 hashmap 来查询或键入键值对时，类对象首先使用哈希函数（私有成员 _hash_function）将键转型为整型 int。之后类对象将哈希结果对桶数（_buckets_array）取余来定位桶，遍历桶中的链表来查询键值对。
- hashmap 有时会增加桶数，或者重哈希来确保每个桶不会存太多节点导致性能下降。
## 文件说明

- hashmap.h：包含了不完整的 HashMap 类定义，以及每个函数的注释。我们需要后续修改一部分，并添加一些函数。
- hashmap.cpp：文件则是同名头文件的具体实现。
- hashmap_iterator.h：包含了完整的迭代器声明和实现（HashMapIterator），注意区别 hashmap.h 中的 iterator 和 const_iterator。
- build.sh：包含了编译指令。
- main.cpp：包含了 hashmap 的操作样例代码。
- tests.cpp 和 test_settings.cpp 包含大量测试代码。
# 实现
## const 检查
首先我们需要进行 const 关键字的检查。分别要去 hashmap 的头文件和 cpp 文件中重载常方法、以及补齐该有的 const 限定。在 main.cpp 文件中，给 student_helper 函数参数加上适当的 const 限定。
hashmap.h 和 hashmap.cpp 中需要重载常方法版本，来支持常对象的使用。例如 end、at、find。
另外有一些成员方法，本身应该被设定为常方法。例如 contains、size、empty、load_factor、bucket_count。
给出重载函数的实现：
```cpp
template <typename K, typename M, typename H>
const M& HashMap<K, M, H>::at(const K& key) const {
    return static_cast<const M&>(const_cast<HashMap<K, M, H>*>(this)->at(key));
}


template <typename K, typename M, typename H>
typename HashMap<K, M, H>::const_iterator HashMap<K, M, H>::find(const K& key) const {
    return static_cast<const_iterator>((const_cast<HashMap<K, M, H>*>(this)->find(key)));
}

template <typename K, typename M, typename H>
typename HashMap<K, M, H>::const_iterator HashMap<K, M, H>::end() const {
    return static_cast<const_iterator>(const_cast<HashMap<K, M, H>*>(this)->end());
}

/*
    虽然 [] 运算符和 at 功能类似，但是函数体内存在对 insert 这个非常方法的调用
    因此不能重载常方法版本
template <typename K, typename M, typename H>
M& HashMap<K, M, H>::operator[](const K& key) {
   return insert({key, {}}).first->second;
}
*/
```
这里使用到了一个 static-const cast 的技巧，先使用 const_cast 转型 this 指针使得常方法可以调用非常方法，最后再将结果转换为常量，降低了代码重复率。
测试方法是执行 build.sh，若存在 const 检查未到位，则会抛出编译错误，例如如下：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722183978973-e8fb3c1c-6453-4687-ad70-7020f501492f.png#averageHue=%2322201f&clientId=ud089ab67-ed95-4&from=paste&height=193&id=u834ad639&originHeight=290&originWidth=1384&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=52703&status=done&style=none&taskId=u91445824-34a3-48f8-9465-b0132934542&title=&width=922.6666666666666)
但如果所有常量都设置到位了，就不会产生编译错误。最后我们的测试结果是：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722183874362-98fd55bb-54e0-45ff-bafa-82ebb8d94de1.png#averageHue=%232c2a27&clientId=ud089ab67-ed95-4&from=paste&height=44&id=u33ff9d72&originHeight=66&originWidth=827&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=14556&status=done&style=none&taskId=ub3276c76-8a7a-425a-bf3b-190083eec3e&title=&width=551.3333333333334)
OK，说明我们的 const 检查没有问题。
## 拷贝和移动语义
一个良构的 STL 容器必须写好特殊的成员函数，例如以下几类：

- 默认构造（已实现）
- 析构（已实现）
- 拷贝构造
- 拷贝赋值
- 移动构造
- 移动赋值

因此，接下来我们要做的就是加上拷贝和移动语义，避免出现内存泄露的情况。分为以下几点：

- 需要拷贝和移动 _buckets_array，首先我们可以创建一个 vector<node*>，然后可以使用公有成员进行元素拷贝。
- clear 和 insert 会修改 _size 成员，记住不需要自己手动修改。
- 注意自赋值的情况。
- 测试用例中存在链式赋值的情况，注意处理顺序是从右向左，只要移动和拷贝语义没问题就不会出错。
- 不要忘记使用迭代器和 insert 成员函数。

给出我的实现版本：
```cpp
/*
    拷贝构造
    通过迭代器进行节点拷贝
    注意深浅拷贝，不要和 insert 进行重复增长
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(const HashMap<K, M, H>& h) :
    _size(0),
    _hash_function(h._hash_function),
    _buckets_array(h.bucket_count(), nullptr) {
    //iterator dst = this->begin();
    for(const_iterator it = h.begin();it != h.end();++it) {
        this->insert(*it);
    } 
}

/*
    拷贝赋值
    注意深浅拷贝和自赋值问题
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>& HashMap<K, M, H>::operator=(const HashMap<K, M, H>& h) {
    if(this != &h) { // 并非自赋值
        this->clear(); // 释放掉原来的资源
        for(auto[key, value]: h) {
            this->insert(std::make_pair(key, value));
        }
    }
    return *this;
}

/*
    移动构造
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(HashMap<K, M, H>&& h) noexcept:
    _size(h._size),
    _hash_function(h._hash_function),
    _buckets_array(h._buckets_array) {
    //h._buckets_array = nullptr;
}

/*
    移动赋值
    注意自赋值问题
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>& HashMap<K, M, H>::operator=(HashMap<K, M, H>&& h) noexcept {
    if(&h != this) {
        this->clear();
        _size = h.size();
        _hash_function = h._hash_function;
        _buckets_array = h._buckets_array;
        //h._buckets_array = nullptr;
    }
    return *this;
}

```
测试结果如下：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722245901640-ff5d3af7-5aee-41b2-b7db-79d9a0bfdf63.png#averageHue=%2321201f&clientId=u97f19e84-6c49-4&from=paste&height=145&id=ufbcbeeb5&originHeight=218&originWidth=521&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=20215&status=done&style=none&taskId=ue0d4ddd7-e6e2-48fa-9f81-36d56d9f526&title=&width=347.3333333333333)
这是怎么回事？经过我的搜索，浮点数例外可能是在除余运算中遇到了不合法的模数，让我们调试一下看看：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722246142763-fffe6edc-4193-4671-9d65-cfc3bad48d98.png#averageHue=%23665e2e&clientId=u97f19e84-6c49-4&from=paste&height=444&id=u95cb0359&originHeight=666&originWidth=1664&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=181774&status=done&style=none&taskId=uae8a3c2a-258f-4c46-8ef8-30a98207ed1&title=&width=1109.3333333333333)
果然是是这样，可以发现在进行除余运算时，bucket 的数量变成了 0，因此无法做除余运算。而测试数据属于 4F 组，那么看来是我们的移动语义写出了问题。
修改后的移动语义代码如下：
```cpp
/*
    移动构造
    不能直接用 std::move 给 h._buckets_array 偷过来
    会出现移动失效
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>::HashMap(HashMap<K, M, H>&& h) noexcept:
    _size(std::move(h._size)),
    _hash_function(std::move(h._hash_function)),
    _buckets_array(h.bucket_count(), nullptr) {
    for(size_t i = 0;i < _buckets_array.size();++i) {
        _buckets_array[i] = std::move(h._buckets_array[i]);
        h._buckets_array[i] = nullptr;
    }
    h._size = 0;
}

/*
    移动赋值
    注意自赋值问题
    同样不能直接用 std::move 偷 vector<node*>
*/
template <typename K, typename M, typename H>
HashMap<K, M, H>& HashMap<K, M, H>::operator=(HashMap<K, M, H>&& h) noexcept {
    if(&h != this) {
        this->clear();
        _size = std::move(h.size());
        _hash_function = std::move(h._hash_function);
        _buckets_array.resize(h.bucket_count(), nullptr);
        for(size_t i = 0;i < h.bucket_count();++i) {
            _buckets_array[i] = std::move(h._buckets_array[i]);
            h._buckets_array[i] = nullptr;
        }
        //h._size = 0;
    }
    return *this;
}
```
再次测试：
![image.png](https://cdn.nlark.com/yuque/0/2024/png/43291115/1722248392020-553993e7-6563-442a-95a7-3616b5afba43.png#averageHue=%23232120&clientId=u97f19e84-6c49-4&from=paste&height=479&id=ubdd41533&originHeight=718&originWidth=490&originalType=binary&ratio=1.5&rotation=0&showTitle=false&size=63470&status=done&style=none&taskId=u9a09a1d3-148e-49c9-abc6-3a7cee2b6c5&title=&width=326.6666666666667)
全部通过！同时也验证了移动语义在拷贝量大时性能更优。
值得一提的是，在测试文件 tests.cpp 的第 1620 行上下存在如下代码：
```cpp
HashMap<std::string, int> copy = map1;
HashMap<std::string, int> temp, new_assign;
temp = std::move(map1);
new_assign = std::move(temp);
VERIFY_TRUE(check_map_equal(new_assign, temp), __LINE__);
```
个人认为这个测试并不正确，对 new_assign 经过移动赋值之后，被移动 temp 应该被置空，buckets 内不含键值对，所以 size 应为 0。但是 check 的逻辑是检查 size 是否相同，那么被操作的 temp 和 new_assign 必然不会相同。
根据上下文来看，个人认为 check 的第二操作数应该是 copy 这个拷贝构造的值；或者将 VERIFY 修改为 FALSE——可惜测试文件中并没有给出。
