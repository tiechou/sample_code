
/*********************************************************************
 * $Author: cuilun $
 *
 * $LastChangedBy: cuilun $
 *
 * $Revision: 2577 $
 *
 * $LastChangedDate: 2011-03-09 09:50:55 +0800 (Wed, 09 Mar 2011) $
 *
 * $Id: ksb_stopword.h 2577 2011-03-09 01:50:55Z cuilun $
 *
 * $Brief: ini file parser $
 ********************************************************************/

#ifndef SRC_KSB_STOPWORD_H_
#define SRC_KSB_STOPWORD_H_

#include <stdint.h>

typedef struct stopword_map_s {
    uint64_t map[1024];
} stopword_map_t;

int ksb_load_stopword(const char *stopword_dict, stopword_map_t* stopword);

inline int
ksb_is_stopword(int word, stopword_map_t* stopword) {
    if (word >= 65536 || word < 0) {
        return 0;
    }
    int pos = word / 64;
    int bit = word % 64;
    return (stopword->map[pos] & (1 << bit)) ? 1 : 0;
}

#endif                          //SRC_KSB_STOPWORD_H_
