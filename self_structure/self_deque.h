#ifndef SELF_DEQUE_H_INCLUDED
#define SELF_DEQUE_H_INCLUDED

/* mydeque
 * written by Zn, 2017.
 * INCOMPLETE @ 8th,Nov
 */

// n!=0 means that buffer size is defined by the user.
// Or else we use the default.
inline size_t __deque_buf_size(size_t n,size_t sz)
{
    return n!=0?n:(sz<512?size_t(512/sz):size_t(1));
}


template <class ElemType, class Ref, class Ptr, size_t BufSiz>
struct __mydeque_iterator //complicated iterator, not inheriting from normal iterator.
{
    typedef __mydeque_iterator<ElemType, ElemType&, ElemType*, BufSiz>             iterator;
    typedef __mydeque_iterator<ElemType, const ElemType&, const ElemType*, BufSiz> const_iterator;
    static size_t buffer_size() { return __deque_buf_size(BufSiz,sizeof(ElemType)); }

    //Five necessary iterator types.
    typedef random_access_iterator_tag iterator_category;//(1)
    typedef ElemType                   value_type;//(2)
    typedef Ptr                        pointer;//(3)
    typedef Ref                        reference;//(4)
    typedef size_t                     size_type;
    typedef ptrdiff_t                  difference_type;//(5)
    typedef ElemType**                 map_pointer;

    typedef __deque_iterator self;

    //cur: current element of the buffer that the iterator points to.
    //first: head~
    //last: last~(including the spare space)
    ElemType* cur,first,last;
    map_pointer node;

    // skip a buffer.
    void set_node(map_pointer new_node)
    {
        node=new_node;
        first=*new_node;
        last=first+difference_type(buffer_size());
    }

    // operators overloading.
    reference operator*() const { return *cur; } ///???
    pointer operator->() const  { return &(operator*()); } ///???
    difference_type operator-(const self& x) const // not difficult: considering the concrete structure of the iterator.
    {
        return difference_type(buffer_size())*(node-x.node-1)+(cur-first)+(x.last-x.cur);
    }

    //More Effective C++, item6.
    self& operator++()
    {
        ++cur;
        if(cur==last)
        {
            set_node(node+1); //switch to the next node.
            cur=first;
        }
        return *this;
    }
    self operator++(int)
    {
        self tmp=*this;
        ++*this;
        return tmp;
    }
    self& operator--()
    {
        if(cur==first)
        {
            set_node(node-1); //switch to the next node.
            cur=last;
        }
        --cur;
        return *this;
    }
    self operator--(int)
    {
        self tmp=*this;
        --*this;
        return tmp;
    }

    //random access. VERY DIFFICULT~
    self& operator+=(difference_type n)
    {
        difference_type offset=n+(cur-first);
        if(offset>=0 && offset<difference_type(buffer_size()))
            cur+=n; // in the same buffer.
        else
        {
            difference_type node_offset=offset>0?offset/difference_type(buffer_size())
                                                :-difference_type((-offset-1)/buffer_size())-1;
            set_node(node+node_offset);
            cur=first+(offset-nodeoffset*difference_type(buffer_size()));
        }
        return *this;
    }

    //More Effective C++, item22.
    self operator+(difference_type n) const
    {
        self tmp=*this; return tmp+=n;
    }
    self& operator-=(difference_type n) const { return *this+=-n; }

    //More Effective C++, item22.
    self operator-(difference_type n) const
    {
        self tmp=*this; return tmp-=n;
    }

    //random access. BETTER TO IMPLEMENT;)
    reference operator[](difference_type n) const { return *(*this+n); }

    bool operator==(const self& x) const { return cur==x.cur; }
    bool operator!=(const self& x) const { return !(*this==x); }
    bool operator<(const self& x) const { return (node==x.node)?(cur<x.cur):(node<x.node); }
};

template <class ElemType, class Alloc=alloc, size_t BufSiz=0>
class mydeque
{
public:     //Basic Types.
    typedef ElemType    value_type;
    typedef value_type* pointer;
    typedef size_t      size_type;
    //later defined.

public: //Iterators
    typedef __mydeque_iterator<ElemType, ElemType&, ElemType*, BufSiz> iterator;

protected:  //Internal typedefs
    typedef pointer* map_pointer; // pointer of pointer of ElemType;
    typedef simple_alloc<value_type, Alloc> data_allocator;
    typedef simple_alloc<pointer, Alloc> map_allocator;

protected:  //Data members
    iterator start,finish;// 1st and last node in the map
    map_pointer map; // a consecutive place where the element maintains a cursor that points to a node(buffer).
    size_type   map_size; // compatible to STL C++(size_type).
    mydeque(int n,const value_type& value) : start(), finish(), map(0), map_size(0) // protected constructor
    {  fill_initialize(n,value);/* defined later. */ }

public:     //Basic accessors
    iterator begin() { return start; }
    iterator end() { return finish; }

    reference operator[](size_type n) { return start[difference_type(n)]; }// __mydeque_iterator<>::operator[]
    reference front() { return *start; } // pay attention to the difference of begin() and front().
    reference back()
    {
        iterator tmp=finish; --tmp; return *tmp;
        // we dont have a opt-(int) :(
        // so we have to do so.
    }

    size_type size() const { return finish-start; }//__mydeque_iterator<>::operator-
    size_type max_size() const { return size_type(-1); } //unsigned int
    bool empty() const { return finish==start; }
};

template <class ElemType, class Alloc, size_t BufSiz>
    void deque<ElemType, Alloc, BufSiz>::fill_initialize(size_type n, const value_type& value)
    {
        create_map_and_nodes(n);
        map_pointer cur;
        try
        {
            for(cur=start.node;cur<finish.node;++cur) uninitialized_fill(*cur,*cur+buffer_size(),value);
            uninitialized_fill(finish.first,finish.cur,value);// the last node may have some spare space.
        }
        catch(...)
        {
            //Accomplished later.
            //commit or rollback.
        }
    }
template <class ElemType, class Alloc, size_t BufSiz>
    void deque<ElemType, Alloc, BufSiz>::create_map_and_nodes(size_type num_elements)
    {
        size_type num_nodes=num_elements/buffer_size()+1;//exact devision -> an extra node

        map_size=max(initial_map_size(),num_nodes+2);// +1+1 -> head and tail
        map=map_allocator::allocate(map_size);

        map_pointer nstart=map+(map_size-num_nodes)/2;
        map_pointer nfinish=nstart+num_nodes-1;

        map_pointer cur;
        try
        {
            //configure the buffer.
            //the sum of buffer equals the available space of the deque.
            for(cur=nstart;cur<=nfinish;++cur) *cur=allocate_nodes();
        }
        catch(...)
        {
            //Accomplished later.
            //commit or rollback.
        }

        start.set_nodes(nstart);
        start.set_nodes(nfinish);
        start.cur=start.first;
        finish.cur=finish.first+num_elements%buffer_size();
    }

//TODO: push_back ...

#endif // SELF_DEQUE_H_INCLUDED
