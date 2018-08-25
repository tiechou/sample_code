#include<iostream>
using std::cout;

template <typename T>
class Negate
{
    public:
        T operator()(T n){return -n;}
    
};

template <typename T>
class GenericNegate
{
    public:
         //operator()(T t)const{return -t;}
         T fan(T t){return -t;}
    
};

template <typename T > T Callback(T n, Negate& neg)
{
    //调用重载的操作符“()” 
    //T val = neg(n);
    T val = neg(-n);
    return val;
}
int main(int argc,char** argv)
{
    Negate neg;
    cout<<Callback(23,neg );
    //cout<<Callback(23,Negate() );
    GenericNegate<double> negate;

    cout<< negate.fan(5.3333); // double

}
