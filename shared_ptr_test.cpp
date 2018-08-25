#include <tr1/memory>
#include <iostream>
#include <assert.h>
using namespace std;
using namespace tr1;

void delete_array(int *arr)
{
    delete [] arr;
}

int main()
{
    shared_ptr<int> sp1(new int(1));

    cout << sp1.get() << endl;          // output:address1
    cout << *sp1 << endl;               // output:1
    cout << sp1.use_count() << endl;    // output:1
    assert(sp1.unique());

    shared_ptr<int> sp2;
    cout << sp2.get() << endl;          // output:null address

    sp2 = sp1;
    assert(!sp1.unique() && sp1.use_count() == 2 && sp1 == sp2);

    *sp2 = 100;
    assert(*sp1 == 100);

    shared_ptr<int> sp3(new int[100], delete_array);

    return 0;
}
