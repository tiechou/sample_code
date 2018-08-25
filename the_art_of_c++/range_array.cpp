listing 1
// A custom container that implements a 
// range-selectable array. 
// 
// Call this file ra.h 
// 
#include <iostream> 
#include <iterator> 
#include <algorithm> 
#include <cstdlib> 
#include <stdexcept> 

using namespace std; 

// An exception class for RangeArray. 
class RAExc { 
    string err; 
    public: 

    RAExc(string e) { 
        err = e; 
    } 

    string geterr() { return err; } 
}; 

// A range-selectable array container. 
template<class T, class Allocator = allocator<T> > 
class RangeArray { 
    T *arrayptr; // pointer to array that underlies the container 

    unsigned len;   // holds length of the container 
    int upperbound; // lower bound 
    int lowerbound; // upper bound 

    Allocator a; // allocator 
    public: 
    // Required typedefs for container. 
    typedef T value_type; 
    typedef Allocator allocator_type; 
    typedef typename Allocator::reference reference; 
    typedef typename Allocator::const_reference const_reference; 
    typedef typename Allocator::size_type size_type; 
    typedef typename Allocator::difference_type difference_type; 
    typedef typename Allocator::pointer pointer; 
    typedef typename Allocator::const_pointer const_pointer; 

    // Forward iterators. 
    typedef T * iterator; 
    typedef const T * const_iterator; 

    // Note: This container does not support reverse 
    // iterators, but you can add them if you like.  


    // *****  Constructors and Destructor ***** 

    // Default constructor. 
    RangeArray() 
    { 
        upperbound = lowerbound = 0; 
        len = 0; 
        arrayptr = a.allocate(0); 
    } 

    // Construct an array of the specified range 
    // with each element having the specified initial value. 
    RangeArray(int low, int high, const T &t); 

    // Construct zero-based array of num elements 
    // with the value t. This constructor is required 
    // for STL compatibility. 
    RangeArray(int num, const T &t=T()); 

    // Construct from range of iterators. 
    RangeArray(iterator start, iterator stop); 

    // Copy constructor. 
    RangeArray(const RangeArray &o); 

    // Destructor. 
    ~RangeArray(); 


    // *****  Operator Functions ***** 

    // Return reference to specified element. 
    T &operator[](int i) 
    { 
        return arrayptr[i - lowerbound]; 
    } 

    // Return const references to specified element. 
    const T &operator[](int i) const 
    { 
        return arrayptr[i - lowerbound]; 
    } 

    // Assign one container to another. 
    RangeArray &operator=(const RangeArray &o); 


    // *****  Insert Functions ***** 

    // Insert val at p. 
    iterator insert(iterator p, const T &val); 

    // Insert num copies of val at p. 
    void insert(iterator p, int num, const T &val) 
    { 
        for(; num>0; num--) p = insert(p, val) + 1; 
    } 

    // Insert range specified by start and stop at p. 
    void insert(iterator p, iterator start, iterator stop) 
    { 
        while(start != stop) { 
            p = insert(p, *start) + 1; 
            start++; 
        } 
    } 


    // *****  Erase Functions ***** 

    // Erase element at p. 
    iterator erase(iterator p); 

    // Erase specified range. 
    iterator erase(iterator start, iterator stop) 
    { 
        iterator p = end(); 

        for(int i=stop-start; i > 0; i--) 
            p = erase(start); 

        return p; 
    } 


    // *****  Push and Pop Functions *****  

    // Add element to end. 
    void push_back(const T &val) 
    { 
        insert(end(), val); 
    } 

    // Remove element from end. 
    void pop_back() 
    { 
        erase(end()-1); 
    } 

    // Add element to front. 
    void push_front(const T &val) 
    { 
        insert(begin(), val); 
    } 

    // Remove element from front. 
    void pop_front() 
    { 
        erase(begin()); 
    } 


    // *****  front() and back() functions ***** 

    // Return reference to first element. 
    T &front() 
    { 
        return arrayptr[0]; 
    } 

    // Return const reference to first element. 
    const T &front() const 
    { 
        return arrayptr[0]; 
    } 

    // Return reference to last element. 
    T &back() 
    { 
        return arrayptr[len-1]; 
    } 

    // Return const reference to last element. 
    const T &back() const 
    { 
        return arrayptr[len-1]; 
    } 


    // *****  Iterator Functions ***** 

    // Return iterator to first element. 
    iterator begin() 
    { 
        return &arrayptr[0]; 
    } 

    // Return iterator to last element. 
    iterator end() 
    { 
        return &arrayptr[upperbound - lowerbound]; 
    } 

    // Return const iterator to first element. 
    const_iterator begin() const 
    { 
        return &arrayptr[0]; 
    } 

    // Return const iterator to last element. 
    const_iterator end() const 
    { 
        return &arrayptr[upperbound - lowerbound]; 
    } 


    // *****  Misc. Functions ***** 

    // The at() function performs a range check. 
    // Return a reference to the specified element. 
    T &at(int i) 
    { 
        if(i < lowerbound || i >= upperbound) 
            throw out_of_range("Index Out of Range"); 

        return arrayptr[i - lowerbound]; 
    } 

    // Return a const reference to the specified element. 
    const T &at(int i) const 
    { 
        if(i < lowerbound || i >= upperbound) 
            throw out_of_range("Index Out of Range"); 

        return arrayptr[i - lowerbound]; 
    }     

    // Return the size of the container. 
    size_type size() const 
    {  
        return end() - begin(); 
    } 

    // Return the maximum size of a RangeArray. 
    size_type max_size() 
    { 
        return a.max_size(); 
    } 

    // Return true if container is empty. 
    bool empty() 
    {  
        return size() == 0; 
    } 

    // Exchange the values of two containers. 
    void swap(RangeArray &b) 
    { 
        RangeArray<T> tmp; 

        tmp = *this; 
        *this = b; 
        b = tmp; 
    } 

    // Remove and destroy all elements. 
    void clear() 
    { 
        erase(begin(), end()); 
    } 

    // ***** Non-STL functions ***** 

    // Return endpoints. 
    int getlowerbound() 
    { 
        return lowerbound; 
    } 

    int getupperbound() 
    { 
        return upperbound; 
    } 

}; 


// ***** Implementations of non-inline functions ***** 

// Construct an array of the specified range 
// with each element having the specified initial value. 
    template <class T, class A>  
RangeArray<T, A>::RangeArray(int low, int high, 
        const T &t) 
{ 
    if(high <= low) throw RAExc("Invalid Range"); 

    high++;  

    // Save endpoints. 
    upperbound = high; 
    lowerbound = low; 

    // Allocate memory for the container. 
    arrayptr = a.allocate(high - low); 

    // Save the length of the container. 
    len = high - low; 

    // Construct the elements. 
    for(size_type i=0; i < size(); i++) 
        a.construct(&arrayptr[i], t); 
} 

// Construct zero-based array of num elements 
// with the value t. This constructor is required 
// for STL compatibility. 
template <class T, class A>  
RangeArray<T, A>::RangeArray(int num, const T &t) { 

    // Save endpoints. 
    upperbound = num; 
    lowerbound = 0; 

    // Allocate memory for the container. 
    arrayptr = a.allocate(num); 

    // Save the length of the container. 
    len = num; 

    // Construct the elements. 
    for(size_type i=0; i < size(); i++) 
        a.construct(&arrayptr[i], t); 

} 

// Construct zero-based array from range of iterators. 
// This constructor is required for STL compatibility. 
    template <class T, class A>  
RangeArray<T, A>::RangeArray(iterator start, 
        iterator stop) 
{ 
    // Allocate sufficient memory. 
    arrayptr = a.allocate(stop - start); 

    upperbound = stop - start; 
    lowerbound = 0; 

    len = stop - start; 

    // Construct the elements using those 
    // specified by the range of iterators. 
    for(size_type i=0; i < size(); i++) 
        a.construct(&arrayptr[i], *start++); 
} 

// Copy constructor. 
    template <class T, class A>  
RangeArray<T, A>::RangeArray(const RangeArray<T, A> &o) 
{ 
    // Allocate memory for the copy. 
    arrayptr = a.allocate(o.size()); 

    upperbound = o.upperbound; 
    lowerbound = o.lowerbound; 
    len = o.len; 

    // Make the copy. 
    for(size_type i=0; i < size(); i++) 
        a.construct(&arrayptr[i], o.arrayptr[i]); 
} 

// Destructor. 
    template <class T, class A> 
RangeArray<T, A>::~RangeArray() 
{ 
    // Call destructors for elements in the container. 
    for(size_type i=0; i < size(); i++) 
        a.destroy(&arrayptr[i]); 

    // Release memory. 
    a.deallocate(arrayptr, size()); 
} 

// Assign one container to another. 
    template <class T, class A> RangeArray<T, A> &  
RangeArray<T, A>::operator=(const RangeArray<T, A> &o) 
{ 
    // Call destructors for elements in target container. 
    for(size_type i=0; i < size(); i++) 
        a.destroy(&arrayptr[i]); 

    // Release original memory. 
    a.deallocate(arrayptr, size());  

    // Allocate memory for new size. 
    arrayptr = a.allocate(o.size()); 

    upperbound = o.upperbound; 
    lowerbound = o.lowerbound; 
    len = o.len; 

    // Make copy. 
    for(size_type i=0; i < size(); i++) 
        arrayptr[i] = o.arrayptr[i]; 

    return *this; 
} 

// Insert val at p. 
template <class T, class A> 
    typename RangeArray<T, A>::iterator  
RangeArray<T, A>::insert(iterator p, const T &val) 
{ 
    iterator q; 
    size_type i, j; 

    // Get sufficient memory. 
    T *tmp = a.allocate(size() + 1); 

    // Copy existing elements to new array, 
    // inserting new element if possible. 
    for(i=j=0; i < size(); i++, j++) { 
        if(&arrayptr[i] == p) {  
            tmp[j] = val; 
            q = &tmp[j]; 
            j++; 
        } 
        tmp[j] = arrayptr[i]; 
    } 

    // Otherwise, the new element goes on end. 
    if(p == end()) { 
        tmp[j] = val; 
        q = &tmp[j]; 
    } 

    // Adjust len and bounds. 
    len++; 
    if(p < &arrayptr[abs(lowerbound)]) 
        lowerbound--; 
    else  
        upperbound++; 

    // Call destructors for elements in old container. 
    for(size_type i=0; i < size()-1; i++) 
        a.destroy(&arrayptr[i]); 

    // Release memory for old container. 
    a.deallocate(arrayptr, size()-1); 

    arrayptr = tmp; 

    return q; 
} 

// Erase element at p. 
template <class T, class A> 
    typename RangeArray<T, A>::iterator  
RangeArray<T, A>::erase(iterator p) 
{     
    iterator q = p; 

    // Destruct element being erased.  
    if(p != end()) a.destroy(p); 

    // Adjust len and bounds. 
    len--; 
    if(p < &arrayptr[abs(lowerbound)])  
        lowerbound++; 
    else  
        upperbound--; 

    // Compact remaining elements. 
    for( ; p < end(); p++) 
        *p = *(p+1); 

    return q; 
} 


// ********  Relational Operators ************** 

    template<class T, class Allocator> 
bool operator==(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    if(a.size() != b.size()) return false; 

    return equal(a.begin(), a.end(), b.begin()); 
} 

    template<class T, class Allocator> 
bool operator!=(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    if(a.size() != b.size()) return true; 

    return !equal(a.begin(), a.end(), b.begin()); 
} 

    template<class T, class Allocator> 
bool operator<(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    return lexicographical_compare(a.begin(), a.end(), 
            b.begin(), b.end()); 
} 

    template<class T, class Allocator> 
bool operator>(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    return b < a; 
} 

    template<class T, class Allocator> 
bool operator<=(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    return !(a > b); 
} 

    template<class T, class Allocator> 
bool operator>=(const RangeArray<T, Allocator> &a, 
        const RangeArray<T, Allocator> &b) 
{ 
    return !(a < b); 
}

