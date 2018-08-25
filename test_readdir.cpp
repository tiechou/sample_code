#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

int main(int argc, char** argv)
{
    if (argc != 2) {
        return -1;
    }
    DIR* dir = 0;
    struct dirent file;
    struct dirent* retptr = 0;

	char path_buf[PATH_MAX+1];
	int len = 0;
	
	if(argv[1] && argv[1][0]){
		dir = opendir(argv[1]);
		if(!dir){
			printf("opendir argv[1] error, return!");
			return -1;
		}
		
		while(readdir_r(dir, &file, &retptr)==0 && retptr){
			len = strlen(file.d_name);
			if((file.d_type!=DT_REG && file.d_type!=DT_LNK) || len<4 || strstr(file.d_name, ".so")==0){
				continue;
			}
			snprintf(path_buf, PATH_MAX+1, "%s/%s", argv[1], file.d_name);
            printf("path_buf:%s\n", path_buf);
		}
		
		closedir(dir);
	}
    return 0;
}
