#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

namespace __gnu_cxx
{
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& x ) const
        {
            return hash< const char* >()( x.c_str() );
        }
    };
}

#define RADIX_NUM 256

struct radix_node_t
{
    radix_node_t* p[RADIX_NUM];
    int index;
    bool is_final;

    //radix_node_t() {cout << "new a node" << endl;}
    //~radix_node_t() {cout << "destroy a node" << endl;}
};

void radix_init(radix_node_t* &radix)
{
    radix = new radix_node_t;
    memset(radix, 0, sizeof(radix_node_t));
}

void radix_insert(radix_node_t* radix, char* str, int index)
{
    char* ptr = str;
    radix_node_t* radix_iter = radix;

    while (*ptr)
    {
        //cout << *ptr << endl;

        if (radix_iter->p[*ptr] == NULL)
        {
            //
            radix_node_t* new_radix = new radix_node_t;
            memset(new_radix, 0, sizeof(radix_node_t));
            radix_iter->p[*ptr] = new_radix;
        }

        radix_iter = radix_iter->p[*ptr];

        ++ptr;
    }

    radix_iter->index=index;
    radix_iter->is_final=true;
}

inline int radix_find(radix_node_t* radix, char* str)
{
    radix_node_t* radix_iter = radix;

    char* ptr = str;

    while(*ptr)
    {
        if(radix_iter->p[*ptr] == NULL)
        {
            return -1;
        }

        radix_iter = radix_iter->p[*ptr];
        ++ptr;
    }

    if (radix_iter->is_final == true)
    {
        return radix_iter->index;
    }
}

bool radix_destroy(radix_node_t* radix)
{
    radix_node_t* radix_iter = radix;
    for(int i=0; i<RADIX_NUM; ++i)
    {
        if (radix_iter->p[i] == NULL)
        {
            continue;
        }

        //the leaf node
        if (radix_iter->p[i]->is_final == true)
        {
            delete radix_iter->p[i];
        }
        else
        {
            radix_destroy(radix_iter->p[i]);
        }
    }

    delete radix_iter;
}

#define FIND_COUNT 100000000

int main(int argc, const char *argv[])
{
    char fields[][20]={{"nid"}, {"user_id"}, {"post_fee"}, {"title"}, {"nick"}, {"price"}, {"pict_url"}};

    hash_map<string, int> hashMap;

    int begin, end;

    //head node
    radix_node_t* radix;

    for(int i=0; i<7; ++i)
    {
        hashMap[fields[i]] = i;
    }

    begin = time(NULL);
    for (int i=0; i<FIND_COUNT; ++i)
    {
        volatile int index = hashMap[fields[i % 7]];
    }
    end = time(NULL);

    cout << "hash_map time: " << end - begin << endl;

    //===================================================

    radix_init(radix);

    for(int i=0; i<7; ++i)
    {
        char* ptr = fields[i];
        radix_insert(radix, ptr, i);
    }

    char* str = "abc";

    begin = time(NULL);
    for (int i=0; i<FIND_COUNT; ++i)
    {
        char* str = fields[i % 7];
        volatile int index = radix_find(radix, str);

        //cout << index << endl;
    }
    end = time(NULL);

    cout << "radix tree time: " << end - begin << endl;

    radix_destroy(radix);

    return 0;
}
