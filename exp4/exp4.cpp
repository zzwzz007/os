#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

char ori_cwd[128];

void printdir(char *dir, int depth);
void printinfo(struct stat s);

int main(int argc, char **argv)
{
    if (getcwd(ori_cwd, 128) == NULL)
    {
        perror("getcwd error!\n");
        exit(-1);
    }
    if (argc == 1)
        printdir(ori_cwd, 0);
    else
    {
        for (int i = 1; i < argc; i++)
        {
            printdir(argv[i], 0);
        }
    }
    return 0;
}

void printinfo(struct stat s)
{
    // print type
    switch (s.st_mode & S_IFMT)
    {
    case S_IFBLK:
        printf("b");
        break;
    case S_IFCHR:
        printf("c");
        break;
    case S_IFDIR:
        printf("d");
        break;
    case S_IFIFO:
        printf("p");
        break;
    case S_IFLNK:
        printf("l");
        break;
    case S_IFREG:
        printf("-");
        break;
    case S_IFSOCK:
        printf("s");
        break;
    default:
        break;
    }
    //print permission
    for (int i = 8; i >= 0; i--)
    {
        if (s.st_mode & (1 << i))
        {
            switch (i % 3)
            {
            case 0:
                printf("x");
                break;
            case 1:
                printf("w");
                break;
            case 2:
                printf("r");
                break;
            }
        }
        else
        {
            printf("-");
        }
    }
    //print size
    printf(" %ld ", s.st_size);
    //print time
    char *time = ctime(&s.st_atime);
    time[strlen(time) - 1] = '\0';
    printf("%s ", time);
}

void printdir(char *dir, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char cur_cwd[128];
    if ((dp = opendir(dir)) == NULL)
    {
        perror("opendir error!\n");
        return;
    }
    chdir(dir);
    //print cur dir
    if (getcwd(cur_cwd, 128) == NULL)
    {
        perror("getcwd error!\n");
        exit(-1);
    }
    printf("\n%s:\n", cur_cwd);

    while ((entry = readdir(dp)) != NULL)
    {
        //get info
        if (entry->d_name[0] == '.')
            continue;
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            //if".." or "."
            if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
                //skip
                continue;
            //print info
            printf("%d ", depth);
            printinfo(statbuf);
            printf("%s\n", entry->d_name);
            //get next
            printdir(entry->d_name, depth + 1);
            printf("\n");
        }
        else
        {
            fprintf(stdout, "%d ", depth);
            printinfo(statbuf);
            fprintf(stdout, "%s\n", entry->d_name);
        }
    }
    chdir("..");
    closedir(dp);
}