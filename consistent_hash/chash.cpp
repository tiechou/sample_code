#include "error.h"
#include "chash.h"
#ifdef TESTCASE
int main(int argc, char **argv)
{
    std::string c = "/etc/server.list";
    chash::hash h(c);
    for(int i=0;i<10000;i++) {
        std::string ip; uint32 port;
        std::string key = "a" + lbase::String::toString(i);
        h.location(key, ip, port);
        std::cout << ip << ":" <<port<< std::endl;
    }
    return 0;
}

#else
namespace chash
{
    const uint32 hash::_MAGIC_ = (4294705152); //((1<<32)-1) ^ ((1<<18)-1)
    const uint32 hash::_VNODE_SIZE_ = (262144); //(1<<18)
    const uint32 hash::_VNODE_NUM_ = (16384);  //(1<<14)
    const uint32 hash::_NODE2VNODE_ = 64;
    //
    chash::ERROR_CODE hash::location(std::string &key, std::string &ip, uint32 &port)
    {
        if(!_OK) return chash::ERR_CONF;
        //set file
        if(!lbase::file::exist(_path)) {
            return chash::ERR_CONF;
        }
        uint32 h = lbase::lcode::crc32(key);
        uint32 vnode = (h & _MAGIC_);
        //std::cout << vnode << std::endl;
        std::string node =  _vnodelist[vnode];
        if(node == "") return chash::ERR_CONF;
        std::vector<std::string> v;
        lbase::String::splitString(node, ‘:’, v);
        if(v.size() != 2) return chash::ERR_CONF;
        ip = v[0];
        port = atoi(v[1].c_str());
        return chash::ERROK;
    }
    void hash::dump()
    {
        for(uint32 vnode =0; vnode<=_MAGIC_; vnode += _VNODE_SIZE_)
        {
            std::cout << vnode << "=>" << _vnodelist[vnode] << std::endl;
            if(vnode == _MAGIC_) break;
        }
    }
    chash::ERROR_CODE hash::flush()
    {
        uint32 first_vnode = 0;
        for(uint32 vnode=0; vnode<=_MAGIC_; vnode += _VNODE_SIZE_)
        {
            if(_vnodelist[vnode] != "")
            {
                first_vnode = vnode;
                break;
            }
            if(vnode == _MAGIC_) break;
        }
        //std::cout << first_vnode << std::endl;
        if( _vnodelist[first_vnode] == "") return chash::ERR_CONF;
        uint32 tmp = first_vnode;
        for(uint32 vnode=first_vnode+_VNODE_SIZE_; vnode<=_MAGIC_; vnode += _VNODE_SIZE_)
        {
            if(vnode == first_vnode) break;
            if(_vnodelist[vnode] != "" )
            {
                tmp = vnode;
                continue;
            }
            _vnodelist[vnode] = _vnodelist[tmp];
        }
        return chash::ERROK;
    }
    chash::ERROR_CODE hash::addServer(std::string &node)
    {
        chash::ERROR_CODE rc;
        rc= addServerLL(node);
        if(rc != chash::ERROK)
            return rc;
        //
        return flush();
    }
    chash::ERROR_CODE hash::removeServer(std::string &node)
        {
            chash::ERROR_CODE rc;
            rc= removeServerLL(node);
            if(rc != chash::ERROK)
                return rc;
            //
            return flush();
        }
    chash::ERROR_CODE hash::addServerLL(std::string &node)
    {
        if(node == "") return chash::ERR_NOTOK;
        if(_nodelist[node] == true) return chash::ERR_EXIST;
        //add node
        _nodelist[node] = true;
        //add vnode
        for(int i=0; i<_NODE2VNODE_; i++)
        {
            uint32 h = lbase::lcode::crc32(node + lbase::String::toString(i));
            uint32 vnode = (h & _MAGIC_);
            //std::cout << vnode << std::endl;
            _vnodelist[vnode] = node;
        }
        return chash::ERROK;
    }
    chash::ERROR_CODE hash::removeServerLL(std::string &node)
        {
            if(node == "" ) return chash::ERR_NOTOK;
            if(_nodelist[node] != true) return chash::ERR_NOTEXIST;
            //delete vnode
            for(int i=0; i<_NODE2VNODE_; i++)
            {
                uint32 h = lbase::lcode::crc32(node + lbase::String::toString(i));
                uint32 vnode = (h & _MAGIC_);
                //std::cout << vnode << std::endl;
                _vnodelist[vnode] = "";
            }
            //clean
            for(uint32 vnode=0; vnode<= _MAGIC_; vnode += _VNODE_SIZE_)
            {
                if(_vnodelist[vnode] == node) _vnodelist[vnode] = "";
                if(vnode == _MAGIC_) break;
            }
            //delete node
            _nodelist[node] = false;
            return chash::ERROK;
        }
    //client class
    hash::hash(std::string &confpath) : _path(confpath),_OK(false)
    {
        lbase::String s;
        if(false == s.load_file(_path))
        {
            _OK = false;
            return;
        }
        std::vector<std::string> v;
        std::vector<std::string> nodelist;
        lbase::String::splitString(s,'\n', v);
        for(int i=0;i<v.size();i++)
        {
            if(v[i].size() <= 0) continue;
            if(v[i][0] == '#') continue;
            nodelist.push_back(v[i]);
        }
        if(nodelist.size() <= 0)
        {
            _OK = false;
            return;
        }
        for(int i=0;i<nodelist.size();i++)
        {
            //std::cout << nodelist[i] << std::endl;
            addServerLL(nodelist[i]);
        }
        //must check all vnode
        flush();
        //dump();
        _OK = true;
    }
    hash::~hash()
    {
        //
    }
} //namespace
#endif
