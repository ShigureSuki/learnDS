#ifndef SELF_VECTOR_H
#define SELF_VECTOR_H

#include <self_construct.h> //默认在gcc的文件夹中，可以直接使用。如果不行，可以改为self_construct.h自行实现。
/*
 * 本头文件实现STL式的线性表，采用现代C++语法。
 * Author: Sam X 
 * Date: from Sep 18th 2017 to ...
 * 为课程设计以及自我水平而coding.
 */
//这里采用SGI STL内置的空间配置器alloc
//如课程设计要求可自行实现之
template <class T, class Alloc=alloc>
class myVector
{
public:
    //初始定义，线性表
    typedef T           value_type;
    typedef value_type* pointer;
    typedef value_type* iterator;
    typedef value_type& reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
protected:
//simple_alloc也是SGI STL的空间配置器
//如课程设计要求可自行实现之
    typedef simple_alloc<value_type,Alloc> data_allocator;
    iterator start;
    iterator finish;
    iterator end_of_storage;

    void insert_aux(iterator position, const T& x);
    void deallocate()
    {
        if(start)
            data_allocator::deallocate(start, end_of_storage-start);
    }

    void fill_initialize(size_type n, const T& value)
    {
        start=allocate_and_fill(n,value);
        finish=start+n;
        end_of_storage=finish;
    }
public:
    iterator begin() { return start; }
    iterator end() { return finish; }//[a,b)
    size_type size() const { return size_type(end()-begin()); }
    size_type capacity() const { return size_type(end_of_storage-begin()); }
    bool empty() const { return begin()==end(); }
    reference operator[] (size_type n) { return *(begin()+n); }

    //构造函数
    vector(): start(0),finish(0), end_of_storage(0) {}
    vector(size_type n, const T& value) { fill_initialize(n,value); }
    vector(int n,const T& value) { fill_initialize(n,value); }
    vector(long n,const T& value) { fill_initialize(n,value); }
    explicit vector(size_type) { fill_initialize(n,T()); }//抑制了构造函数定义的隐式的转换，见C++ Primer Ver5 p265.

    ~vector()
    {
        destroy(start,finish); //见self/stl_construct.h
        deallocate(); //vector的一个member function。
    }

    reference front() { return *begin(); }
    reference back() { return *(end()-1); } //这里采用了和stl同样的处理方式，end并非最后一个元素，以便使用泛型算法。
    void push_back(const T& x)
    {
        if(finish!=end_of_storage)
        {
            construct(finish,x);//见self/stl_construct.h
            ++finish;
        }
        else insert_aux(end(),x);
    }
};

#endif // !SELF_VECTOR_H