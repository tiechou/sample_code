#include <lbase/lbase.h>
#include <iostream>
#include "error.h"
#ifndef _CHASH__H_
#define _CHASH__H_

namespace chash
{
    class hash {
        public:
            hash(std::string &confpath);
            ~hash();
            chash::ERROR_CODE location(std::string &key, std::string &ip, uint32 &port);
            chash::ERROR_CODE addServer(std::string &node);
            chash::ERROR_CODE removeServer(std::string &node);
            void dump();
        private:
            chash::ERROR_CODE addServerLL(std::string &node);
            chash::ERROR_CODE removeServerLL(std::string &node);
            chash::ERROR_CODE flush();
        private:
            std::string _path;
            bool _OK;
            std::map<std::string, bool> _nodelist;
            std::map<uint32, std::string> _vnodelist;
            static const uint32 _MAGIC_;
            static const uint32 _VNODE_SIZE_;
            static const uint32 _VNODE_NUM_;
            static const uint32 _NODE2VNODE_;
    };
}

#endif
