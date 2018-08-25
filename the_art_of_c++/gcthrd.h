// A garbage collector that runs as a back ground task. 

#include <iostream>   
#include <list>   
#include <typeinfo>   
#include <cstdlib> 
#include <windows.h> 
#include <process.h> 

using namespace std;   

// To watch the action of the garbage collector, define DISPLAY. 
// #define DISPLAY   

// Exception thrown when an attempt is made to    
// use an Iter that exceeds the range of the    
// underlying object.   
//   
class OutOfRangeExc {   
    // Add functionality if needed by your application.   
};   

// Exception thrown when a time-out occurs  
// when waiting for access to hMutex.  
//    
class TimeOutExc {    
    // Add functionality if needed by your application.    
};    

// An iterator-like class for cycling through arrays   
// that are pointed to by GCPtrs. Iter pointers   
// ** do not ** participate in or affect garbage   
// collection.  Thus, an Iter pointing to   
// some object does not prevent that object   
// from being recycled.   
//   
template <class T> class Iter {   
    T *ptr;   // current pointer value   
    T *end;   // points to element one past end   
    T *begin; // points to start of allocated array   
    unsigned length; // length of sequence   
    public:   

    Iter() {    
        ptr = end = begin = NULL;   
        length = 0;   
    }   

    Iter(T *p, T *first, T *last) {   
        ptr =  p;   
        end = last;   
        begin = first;   
        length = last - first;   
    }   

    // Return length of sequence to which this   
    // Iter points.   
    unsigned size() { return length; }   

    // Return value pointed to by ptr.   
    // Do not allow out-of-bounds access. 
    T &operator*() {    
        if( (ptr >= end) || (ptr < begin) )   
            throw OutOfRangeExc();   
        return *ptr;   
    }   

    // Return address contained in ptr.   
    // Do not allow out-of-bounds access. 
    T *operator->() {    
        if( (ptr >= end) || (ptr < begin) )   
            throw OutOfRangeExc();   
        return ptr;   
    }     

    // Prefix ++.   
    Iter operator++() {   
        ptr++;   
        return *this;   
    }   

    // Prefix --.   
    Iter operator--() {   
        ptr--;   
        return *this;   
    }   

    // Postfix ++.   
    Iter operator++(int notused) {   
        T *tmp = ptr;   

        ptr++;   
        return Iter<T>(tmp, begin, end);   
    }   

    // Postfix --.   
    Iter operator--(int notused) {   
        T *tmp = ptr;   

        ptr--;   
        return Iter<T>(tmp, begin, end);   
    }   

    // Return a reference to the object at the   
    // specified index. Do not allow out-of-bounds 
    // access. 
    T &operator[](int i) {   
        if( (i < 0) || (i >= (end-begin)) )   
            throw OutOfRangeExc();   
        return ptr[i];   
    }   

    // Define the relational operators.   
    bool operator==(Iter op2) {   
        return ptr == op2.ptr;   
    }   

    bool operator!=(Iter op2) {   
        return ptr != op2.ptr;   
    }   

    bool operator<(Iter op2) {   
        return ptr < op2.ptr;   
    }   

    bool operator<=(Iter op2) {   
        return ptr <= op2.ptr;   
    }   

    bool operator>(Iter op2) {   
        return ptr > op2.ptr;   
    }   

    bool operator>=(Iter op2) {   
        return ptr >= op2.ptr;   
    }   

    // Subtract an integer from an Iter.   
    Iter operator-(int n) {   
        ptr -= n;   
        return *this;   
    }   

    // Add an integer to an Iter.   
    Iter operator+(int n) {   
        ptr += n;   
        return *this;   
    }   

    // Return number of elements between two Iters.   
    int operator-(Iter<T> &itr2) {   
        return ptr - itr2.ptr;   
    }   
};   


// This class defines an element that is stored   
// in the garbage collection information list.    
//   
template <class T> class GCInfo {   
    public:   
        unsigned refcount; // current reference count 

        T *memPtr; // pointer to allocated memory   

        /* isArray is true if memPtr points   
           to an allocated array. It is false   
           otherwise. */   
        bool isArray; // true if pointing to array   

        /* If memPtr is pointing to an allocated   
           array, then arraySize contains its size */   
        unsigned arraySize; // size of array   

        // Here, mPtr points to the allocated memory. 
        // If this is an array, then size specifies 
        // the size of the array.   
        GCInfo(T *mPtr, unsigned size=0) {   
            refcount = 1; 
            memPtr = mPtr;   
            if(size != 0)   
                isArray = true;   
            else   
                isArray = false;   

            arraySize = size;   
        }   
};   

// Overloading operator== allows GCInfos to be compared.   
// This is needed by the STL list class.    
template <class T> bool operator==(const GCInfo<T> &ob1,   
        const GCInfo<T> &ob2) {   
    return (ob1.memPtr == ob2.memPtr);   
}   


// GCPtr implements a pointer type that uses   
// garbage collection to release unused memory.   
// A GCPtr must only be used to point to memory   
// that was dynamically allocated using new.    
// When used to refer to an allocated array,   
// specify the array size.   
//   
template <class T, int size=0> class GCPtr {   

    // gclist maintains the garbage collection list.   
    static list<GCInfo<T> > gclist;   

    // addr points to the allocated memory to which   
    // this GCPtr pointer currently points.   
    T *addr;   

    /* isArray is true if this GCPtr points   
       to an allocated array. It is false   
       otherwise. */   
    bool isArray; // true if pointing to array   

    // If this GCPtr is pointing to an allocated   
    // array, then arraySize contains its size.   
    unsigned arraySize; // size of the array   

    // These support multithreading.  
    unsigned tid; // thread id  
    static HANDLE hThrd;  // thread handle  
    static HANDLE hMutex; // handle of mutex  

    static int instCount; // counter of GCPtr objects  

    // Return an interator to pointer info in gclist.   
    typename list<GCInfo<T> >::iterator findPtrInfo(T *ptr); 

    public:   

    // Define an iterator type for GCPtr<T>.   
    typedef Iter<T> GCiterator;   

    // Construct both initialized and uninitialized objects.   
    GCPtr(T *t=NULL) { 

        // When first object is created, create the mutex  
        // and register shutdown().  
        if(hMutex==0) {  
            hMutex = CreateMutex(NULL, 0, NULL);   
            atexit(shutdown);  
        }  

        //获取互斥体，为了阻止两个线程在同一时刻同时访问gclist·
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p; 

        p = findPtrInfo(t); 

        // If t is already in gclist, then 
        // increment its reference count.  
        // Otherwise, add it to the list. 
        if(p != gclist.end())  
            p->refcount++; // increment ref count 
        else { 
            // Create and store this entry.   
            GCInfo<T> gcObj(t, size);   
            gclist.push_front(gcObj);   
        } 

        addr = t;   
        arraySize = size;   
        if(size > 0) isArray = true;   
        else isArray = false;   

        // Increment instance counter for each new object.    
        instCount++;  

        // If the garbage collection thread is not 
        // currently running, start it running. 
        if(hThrd==0) {  
            hThrd = (HANDLE) _beginthreadex(NULL, 0, gc,  
                    (void *) 0, 0, (unsigned *) &tid);  

            // For some applications, it will be better  
            // to lower the priority of the garbage collector  
            // as shown here:  
            //  
            // SetThreadPriority(hThrd,  
            //                   THREAD_PRIORITY_BELOW_NORMAL);  
        }  

        ReleaseMutex(hMutex);  
    } 

    // Copy constructor.   
    GCPtr(const GCPtr &ob) {   
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p; 

        p = findPtrInfo(ob.addr);  
        p->refcount++; // increment ref count 

        addr = ob.addr;   
        arraySize = ob.arraySize;   
        if(arraySize > 0) isArray = true;   
        else isArray = false;   

        instCount++; // increase instance count for copy 

        ReleaseMutex(hMutex); 
    }   

    // Destructor for GCPTr. 
    ~GCPtr();   

    // Collect garbage.  Returns true if at least   
    // one object was freed.   
    static bool collect();  

    // Overload assignment of pointer to GCPtr.   
    T *operator=(T *t);  

    // Overload assignment of GCPtr to GCPtr.   
    GCPtr &operator=(GCPtr &rv);  

    // Return a reference to the object pointed   
    // to by this GCPtr.   
    T &operator*() {   
        return *addr;   
    }   

    // Return the address being pointed to.   
    T *operator->() { return addr; }     

    // Return a reference to the object at the   
    // index specified by i.   
    T &operator[](int i) {   
        return addr[i];   
    }   

    // Conversion function to T *.   
    operator T *() { return addr; }   

    // Return an Iter to the start of the allocated memory.   
    Iter<T> begin() {   
        int size;   

        if(isArray) size = arraySize;   
        else size = 1;   

        return Iter<T>(addr, addr, addr + size);   
    }       

    // Return an Iter to one past the end of an allocated array. 
    Iter<T> end() {   
        int size;   

        if(isArray) size = arraySize;   
        else size = 1;   

        return Iter<T>(addr + size, addr, addr + size);   
    }   

    // Return the size of gclist for this type   
    // of GCPtr.   
    static int gclistSize() {  
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        unsigned sz = gclist.size();  

        ReleaseMutex(hMutex);  
        return sz;  
    }    

    // A utility function that displays gclist.   
    static void showlist();  

    // The following functions support multithreading.  
    //  
    // Returns true if the collector is still in use.  
    static bool isRunning() { return instCount > 0; }  

    // Clear gclist when program exits.  
    static void shutdown();  

    // Entry point for gabarge collector thread.  
    static unsigned __stdcall gc(void * param);  
};   

// Create storage for the static variables.  
template <class T, int size>     
list<GCInfo<T> > GCPtr<T, size>::gclist;   

template <class T, int size>  
int GCPtr<T, size>::instCount = 0;  

template <class T, int size>  
HANDLE GCPtr<T, size>::hMutex = 0;  

template <class T, int size>  
HANDLE GCPtr<T, size>::hThrd = 0;  

// Destructor for GCPtr. 
template <class T, int size>  
    GCPtr<T, size>::~GCPtr() {   
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p;   

        p = findPtrInfo(addr); 
        if(p->refcount) p->refcount--; // decrement ref count 

        // Decrement instance counter for each object  
        // that is destroyed.  
        instCount--; 

        ReleaseMutex(hMutex); 
    }   

// Collect garbage.  Returns true if at least   
// one object was freed.   
template <class T, int size>  
    bool GCPtr<T, size>::collect() {    
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        bool memfreed = false;   

        list<GCInfo<T> >::iterator p;   
        do {   

            // Scan gclist looking for unreferenced pointers.   
            for(p = gclist.begin(); p != gclist.end(); p++) {   
                // If in-use, skip. 
                if(p->refcount > 0) continue;   

                memfreed = true;   

                // Remove unused entry from gclist. 
                gclist.remove(*p);   

                // Free memory unless the GCPtr is null. 
                if(p->memPtr) { 
                    if(p->isArray) {   
                        delete[] p->memPtr; // delete array   
                    }   
                    else {   
                        delete p->memPtr; // delete single element   
                    }   
                } 

                // Restart the search. 
                break; 
            } 

        } while(p != gclist.end());   

        ReleaseMutex(hMutex); 

        return memfreed;   
    }   

// Overload assignment of pointer to GCPtr.   
template <class T, int size>  
    T * GCPtr<T, size>::operator=(T *t) {   
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p;   

        // First, decrement the reference count 
        // for the memory currently being pointed to. 
        p = findPtrInfo(addr); 
        p->refcount--;  

        // Next, if the new address is already 
        // existent in the system, increment its 
        // count.  Otherwise, create a new entry 
        // for gclist. 
        p = findPtrInfo(t); 
        if(p != gclist.end())  
            p->refcount++;  
        else { 
            // Create and store this entry.   
            GCInfo<T> gcObj(t, size);   
            gclist.push_front(gcObj);   
        } 

        addr = t; // store the address.   

        ReleaseMutex(hMutex); 

        return t;   
    }   

// Overload assignment of GCPtr to GCPtr.   
template <class T, int size>  
    GCPtr<T, size> & GCPtr<T, size>::operator=(GCPtr &rv) {   
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p;   

        // First, decrement the reference count 
        // for the memory currently being pointed to. 
        p = findPtrInfo(addr); 
        p->refcount--; 

        // Next, increment the reference count of 
        // of the new object. 
        p = findPtrInfo(rv.addr); 
        p->refcount++; // increment ref count 

        addr = rv.addr;// store the address.   

        ReleaseMutex(hMutex); 

        return rv;   
    }   

// A utility function that displays gclist.   
template <class T, int size>  
    void GCPtr<T, size>::showlist() {   
        if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT)  
            throw TimeOutExc();  

        list<GCInfo<T> >::iterator p;   

        cout << "gclist<" << typeid(T).name() << ", "   
            << size << ">:\n";   
        cout << "memPtr      refcount    value\n";   

        if(gclist.begin() == gclist.end()) {   
            cout << "           -- Empty --\n\n";   
            return;   
        }   

        for(p = gclist.begin(); p != gclist.end(); p++) {   
            cout <<  "[" << (void *)p->memPtr << "]" 
                << "      " << p->refcount << "     ";   
            if(p->memPtr) cout << "   " << *p->memPtr;   
            else cout << "   ---";   
            cout << endl;         
        }   
        cout << endl;   

        ReleaseMutex(hMutex); 
    }  

// Find a pointer in gclist. 
template <class T, int size>  
typename list<GCInfo<T> >::iterator  
GCPtr<T, size>::findPtrInfo(T *ptr) {   

    list<GCInfo<T> >::iterator p;   

    // Find ptr in gclist.   
    for(p = gclist.begin(); p != gclist.end(); p++)  
        if(p->memPtr == ptr)  
            return p; 

    return p;   
}   

// Entry point for gabarge collector thread.  
template <class T, int size>   
unsigned __stdcall GCPtr<T, size>::gc(void * param) {  
#ifdef DISPLAY  
    cout << "Garbage collection started.\n";  
#endif  

    while(isRunning()) {  
        collect();  
    }  

    collect(); // collect garbage on way out 

    // Release and reset the thread handle so 
    // that the garbage collection thread can 
    // be restarted if necessary. 
    CloseHandle(hThrd); 
    hThrd = 0;  

#ifdef DISPLAY  
    cout << "Garbage collection terminated for "  
        << typeid(T).name() << "\n";    
#endif  

    return 0;  
}   

// Clear gclist when program exits.  
template <class T, int size>   
void GCPtr<T, size>::shutdown() {  

    if(gclistSize() == 0) return; // list is empty 

    list<GCInfo<T> >::iterator p;  

#ifdef DISPLAY      
    cout << "Before collecting for shutdown() for "  
        << typeid(T).name() << "\n";    
#endif  

    for(p = gclist.begin(); p != gclist.end(); p++) {  
        // Set all remaining reference counts to zero. 
        p->refcount = 0;  
    }  

    collect();  

#ifdef DISPLAY      
    cout << "After collecting for shutdown() for "  
        << typeid(T).name() << "\n";    
#endif  
}

listing 9
// Demonstrate the multithreaded garbage collector. 
#include <iostream> 
#include <new> 
#include "gcthrd.h" 

using namespace std; 

// A simple class for load testing GCPtr. 
class LoadTest { 
    int a, b; 
    public: 
    double n[100000]; // just to take-up memory 
    double val; 

    LoadTest() { a = b = 0; } 

    LoadTest(int x, int y) { 
        a = x; 
        b = y; 
        val = 0.0; 
    } 

    friend ostream &operator<<(ostream &strm, LoadTest &obj); 
}; 

// Create an insertor for LoadTest. 
ostream &operator<<(ostream &strm, LoadTest &obj) { 
    strm << "(" << obj.a << " " << obj.b << ")"; 
    return strm; 
} 

int main() { 
    GCPtr<LoadTest> mp; 
    int i; 

    for(i = 1; i < 2000; i++) { 
        try { 
            mp = new LoadTest(i, i); 
            if(!(i%100)) 
                cout << "gclist contains " << mp.gclistSize() 
                    << " entries.\n"; 
        } catch(bad_alloc xa) { 
            // For most users, this exception won't 
            // ever occur. 
            cout << "Last object: " << *mp << endl; 
            cout << "Length of gclist: " 
                << mp.gclistSize() << endl; 
        } 
    } 

    return 0; 
}

