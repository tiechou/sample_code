// Translated from an .exp source file.
// Demonstrate the foreach loop. 
#include <iostream> 
using namespace std; 

int main() { 
    int nums[] = { 1, 6, 19, 4, -10, 88 }; 
    int min; 

    // Find the minimum value. 
    min = nums[0]; 
    int _ia = 0;
    for(int x = nums[0];
        _ia < ((sizeof nums)/(sizeof nums[0]));
        _ia++, x = nums[_ia]) 
        if(min > x) min = x; 

    cout << "Minimum is " << min << endl; 

    return 0; 
}
