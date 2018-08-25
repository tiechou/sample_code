#include <memory> 
#include <iostream> 
#include <assert.h> 

using namespace std; 
int main() 
{ 
    auto_ptr<int> p1(new int(1)); 
    auto_ptr<int> p2; 
    int * raw_p3; 

    cout << "p1\t" << p1.get() << endl; 
    cout << "*p1\t" << *p1 << endl; 
    cout << "p2\t" << p2.get() << endl; 

    p2 = p1; 
    assert(p1.get() == NULL && *p2 == 1); 
    cout << p2.get() << endl; 

    p2.reset(new int(2)); 
    cout << p2.get() << endl; 
    assert(*p2 == 2); 

    *p2 = 10; 
    assert(*p2 == 10); 
    raw_p3 = p2.release(); 
    assert(p2.get() == NULL && *raw_p3 == 10); 
    cout << raw_p3 << endl; 
    delete raw_p3; 

    return 0; 
}
