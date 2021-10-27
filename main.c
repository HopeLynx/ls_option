//gcc main.c -o myLS
// ./myLS -a .

//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>
//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
// new
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <pwd.h>
#include <grp.h>

#define NULL ((void *)0)

const char *getUserName(int id) {
    uid_t uid = id;
    struct passwd *pw = getpwuid(uid);
    if (pw) return pw->pw_name;
    return "";
}

const char *getGroupName(int id) {
    gid_t gid = id;
    struct group *gr = getgrgid(gid);
    if (gr) return gr->gr_name;
    return "";
}

char* statModeToStr(int stat_mode){
    if (stat_mode & S_IFREG)return "file";
    if (stat_mode & S_IFDIR)return "directory";
    //printf("%d",stat_mode);
    return "UNKNOWN";
}

void PrintFileInfo(const char* name,int op_l){
    //printf("!%s!",name);
    struct stat s;
    if( stat(name,&s) == 0 )
    {
        if(!op_l){
            printf("%s ",name);
        } else {
            printf("%s mode:%s nlink:%ld size:%ld uid:%s gid:%s \n",
                   name,statModeToStr(s.st_mode),s.st_nlink,s.st_size,
                   getUserName(s.st_uid),getGroupName(s.st_gid));
        }

    } else {
        perror("stat in print failed");
    }
}

void _ls(const char *dir,int op_a,int op_l,int op_R)
{
    struct stat s;
    if( stat(dir,&s) == 0 )
    {
        if( s.st_mode & S_IFDIR )
        {
            //it's a directory
            //Here we will list the directory
            struct dirent *d;
            DIR *dh = opendir(dir);
            if (!dh){
                if (errno == ENOENT){
                    //If the directory is not found
                    perror("Directory doesn't exist");
                } else {
                    //If the directory is not readable then throw error and exit
                    perror("Unable to read directory");
                }
                exit(EXIT_FAILURE);
            }
            //While the next entry is not readable we will print directory files
            while ((d = readdir(dh)) != NULL){
                //If hidden files are found we continue
                if (!op_a && d->d_name[0] == '.')
                    continue;
                PrintFileInfo(d->d_name,op_l);
            }
            if(!op_l)
                printf("\n");
            
            if (op_R){
                //recursive
                // opening the same folder to check all the dirs
                DIR *dh = opendir(dir);
                if (!dh){
                    (errno == ENOENT) ? perror("Directory doesn't exist"):perror("Unable to read directory");
                    exit(EXIT_FAILURE);
                }
                while ((d = readdir(dh)) != NULL){
                    if (d->d_type & DT_DIR ){
                        if ((!op_a && d->d_name[0] == '.') || !strcmp(d->d_name,"..") || !strcmp(d->d_name,"."))
                            continue;
                        printf("./%s:\n",d->d_name);
                        char strrr[512]  = {};
                        strcpy(strrr,dir);strcat(strrr,"/");strcat(strrr,d->d_name);
                        _ls(d->d_name,op_a,op_l,op_R);
                    }
                }
            }
        }
        else if( s.st_mode & S_IFREG ) {
            //it's a file
            PrintFileInfo(dir,op_l);
            printf("\n");
        }
        else {
            //something else
            perror("wtf is this path?");
        }
    }
    else {
        //error
        perror("stats unavailable");
    }
}

int main(int argc, const char *argv[])
{
    //printf("%d\n",argc);
    int op_a = 0, op_l = 0, op_R = 0, path_index = 0;
    if (argc > 1) {
        for (int i=1; i <= argc-1; i++)
            //Checking if option is passed
            if (argv[i][0] == '-'){
                //Options supporting: a, l, R
                char *p = (char*)(argv[i] + 1);
                if(*p == 'a') op_a = 1;
                else if(*p == 'l') op_l = 1;
                else if (*p == 'R') op_R = 1;
                else{
                    perror("Option not available");
                    exit(EXIT_FAILURE);
                }
            } else {
                path_index = i;
            }
    }
    path_index ? _ls(argv[path_index],op_a,op_l,op_R) : _ls(".",op_a,op_l,op_R) ;
    return 0;
}
