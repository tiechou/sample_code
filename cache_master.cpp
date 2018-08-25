#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;
using namespace __gnu_cxx;

typedef struct CmdParam{
    char file_name[256];
    char dir_name[256]; 
    int  type;
}CmdParam;

typedef struct CacheStat{
    size_t pageCount;
    size_t inCache;
}CacheStat;

bool clearByName(const char* file_name){
    assert(file_name != NULL);

    struct stat st;
    int fd = 0;
    if(stat(file_name, &st)<0){
        goto ERROR;
    }

    fd = open(file_name, O_RDONLY);
    if(fd<0){
        goto ERROR;
    }

    if(posix_fadvise(fd, 0, st.st_size, POSIX_FADV_DONTNEED) != 0){
        goto ERROR;
    }

    fprintf(stdout, "Release:%s\n", file_name);
    return true;

ERROR:
    fprintf(stdout, "File:%s %s\n", file_name, strerror(errno));
    return false;
}

void usage(char *bin_name){
    fprintf(stderr, "Usage:%s [Option] [File] ...\n", bin_name);
    fprintf(stderr, "-c Clear Page Cache.\n");
    fprintf(stderr, "-s Stat of Page Cache.\n");
    fprintf(stderr, "-f Operation on a file.\n");
    fprintf(stderr, "-d Operation on a directory.\n");
    fprintf(stderr, "-c is mutual with -s; -f is mutual with -d\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "%s -c -f file_name | Clear cache of a file\n", bin_name);
    fprintf(stderr, "%s -c -d dir_name | Clear cache of a direcroty\n", bin_name);
    fprintf(stderr, "%s -s -f file_name | Stat cache of a file\n", bin_name);
    fprintf(stderr, "%s -d -d dir_name | Stat cache of a direcroty\n", bin_name);
    fprintf(stderr, "\n");
}

void parseArgs(int argc, char *argv[], CmdParam *pCmd){
    int i = 0;
    int iLoop=0;
    bool isClear = false;
    bool isStat  = false;
    int type = 0;
    memset(pCmd, 0, sizeof(CmdParam));
    while ((i = getopt(argc, argv, "d:f:csh")) != EOF){
        switch(i){
            case 'c':
                isClear = true;
                break;
            case 'd':
                strcpy(pCmd->dir_name, optarg);
                break;
            case 'f':
                strcpy(pCmd->file_name, optarg);
                break;
            case 's':
                isStat = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
        ++iLoop;
    }

    int flen = strlen(pCmd->file_name);
    int dlen = strlen(pCmd->dir_name);

    if(isStat == isClear){
        fprintf(stderr, "Clear is mutual with stat!\n");
        usage(argv[0]);
        return;
    }

    if((dlen==0 && flen==0) || (dlen!=0 && flen!=0)){
        fprintf(stderr, "Parameter Error\n");
        usage(argv[0]);
        return;
    }

    if(isClear){
        if(flen>0){
            type = 1;
        }else{
            type = 2;
        }
    }else{
        if(flen>0){
            type = 3;
        }else{
            type = 4;
        }
    }

    pCmd->type = type;
}

bool clearByDirpath(const char *dir_name){
    DIR *dir;
    struct dirent *dp;
    char path[256];

    if((dir = opendir(dir_name)) == NULL){
        goto ERROR;
    }

    while((dp = readdir(dir)) != NULL){
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
            memset(path, 0, sizeof(path));
            strcat(path, dir_name);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(dp->d_type == DT_REG){
                clearByName(path);
            }else if(dp->d_type == DT_DIR){
                clearByDirpath(path);
            }else{
                fprintf(stdout, "%s:%c type unsupported!\n", dp->d_name, dp->d_type);
            }
        }
    }

    closedir(dir);
    return true;
ERROR:
    fprintf(stdout, "DIR:%s %s\n", dir_name, strerror(errno));
    closedir(dir);
    return false;
}

bool statByName(const char* file_name, CacheStat& stat_cache){
    int fd;
    int pageIndex;
    int pageCount;
    int inCache = 0;
    void *pbase = NULL;
    char *vec = NULL;
    int pagesize = getpagesize();
    struct stat st;

    fd = open(file_name, O_RDONLY);
    if(fd<0){
        goto ERROR;
    }

    if(stat(file_name, &st)<0){
        goto ERROR;
    }

    pbase = mmap((void *)0, st.st_size, PROT_NONE, MAP_SHARED, fd, 0);
    if(pbase == MAP_FAILED){
        goto ERROR;
    }

    pageCount = (st.st_size+pagesize-1)/pagesize;
    vec = (char*)calloc(1, pageCount);
    if(mincore(pbase, st.st_size, (unsigned char *)vec) != 0){
        goto ERROR; 
    }

    for(pageIndex=0; pageIndex<pageCount; pageIndex++){
        if(vec[pageIndex]&1 != 0){
            ++inCache;
        } 
    }

    stat_cache.pageCount += pageCount;
    stat_cache.inCache   += inCache;
    
    fprintf(stdout, "Stat:%s size:%ldM cached:%dM\n", 
            file_name,
            st.st_size/1024/1024, 
            inCache*(pagesize/1024)/1024);

    free(vec);
    munmap(pbase, st.st_size);
    close(fd);
    return true;

ERROR:
    fprintf(stdout, "File:%s %s\n", file_name, strerror(errno));
    if(vec)     free(vec);
    if(pbase)   munmap(pbase, st.st_size);
    if(fd>0)    close(fd);
    return false;
}

bool statByDirpath(const char *dir_name, CacheStat& stat_cache){
    DIR *dir;
    struct dirent *dp;
    char path[256];

    if((dir = opendir(dir_name)) == NULL){
        goto ERROR;
    }

    while((dp = readdir(dir)) != NULL){
        if(dp->d_name[0] != '.'){
            memset(path, 0, sizeof(path));
            strcat(path, dir_name);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(dp->d_type == DT_REG){
                statByName(path, stat_cache);
            }else if(dp->d_type == DT_DIR){
                statByDirpath(path, stat_cache);
            }else{
                fprintf(stdout, "%s:%c type unsupported!\n", dp->d_name, dp->d_type);
            }
        }
    }

    closedir(dir);
    return true;
ERROR:
    fprintf(stdout, "DIR:%s %s\n", dir_name, strerror(errno));
    closedir(dir);
    return false;
}

int main(int argc, char *argv[]){
    if(argc<=2){
        usage(argv[0]);
        exit(-1);
    } 

    int pagesize = getpagesize();
    CmdParam cmd;
    CacheStat stat;
    memset(&stat, 0, sizeof(stat));
    parseArgs(argc, argv, &cmd);

    if(cmd.type == 1){
        clearByName(cmd.file_name);
    }else if(cmd.type == 2){
        clearByDirpath(cmd.dir_name);
    }else if(cmd.type == 3){
        statByName(cmd.file_name, stat);
    }else if(cmd.type == 4){
        statByDirpath(cmd.dir_name, stat);
        fprintf(stdout, "\nTotal Cache of Directory:%s size:%luM cached:%luM\n", 
                cmd.dir_name,
                stat.pageCount*(pagesize/1024)/1024, 
                stat.inCache*(pagesize/1024)/1024);
    }
    return 0;
}
