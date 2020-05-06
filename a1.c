#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PATH_LEN 258


void listElem(char *dirName, char *curr_path, int recursive, long size_filter, int is_filtered_size, int is_filtered_write)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];
    dir = opendir(dirName);
    if (dir == 0)
    {
        printf ("ERROR\nInvalid directory path");
        exit(4);
    }
    while ((dirEntry=readdir(dir)) != 0)
    {
        snprintf(name, MAX_PATH_LEN, "%s/%s",dirName,dirEntry->d_name);
        int is_printed = 1;
        {
            if(strcmp(dirEntry->d_name,".") == 0 || strcmp(dirEntry->d_name,"..") == 0)
                continue;
            lstat (name, &inode);
            if(inode.st_size + 1 > size_filter && is_filtered_size)
                is_printed = 0;
            if(is_filtered_size && (S_ISDIR(inode.st_mode) || S_ISDIR(inode.st_mode)))
                is_printed = 0;
            if(is_filtered_write && access(name, W_OK) == -1)
            {
                    is_printed = 0;
            }
            if(is_printed)
            printf("%s/%s\n", curr_path, dirEntry->d_name);
            if(recursive && S_ISDIR(inode.st_mode) && !strchr(dirEntry->d_name,'.'))
            {
                char nextDir[]="";
                char new_path[MAX_PATH_LEN];
                strcpy(new_path, curr_path);
                strcat(new_path, "/");
                strcat(new_path, dirEntry->d_name);
                strcat(nextDir, dirName);
                strcat(nextDir,"/");
                strcat(nextDir, dirEntry->d_name);
                listElem(nextDir, new_path, 1, size_filter, is_filtered_size,is_filtered_write);
            }
        }
    }
    closedir(dir);
}

void parse(char *dirName, char *curr_path, int findAll)
{
    int validFind = 1;
    int fileDesc;
    char buff[10];
    char text[1000000];
    char faults[4][15];
    int curr_fault = 0;
    int limit = 0;
    fileDesc = open(curr_path, O_RDONLY);
    if (fileDesc == -1)
    {
        printf ("ERROR\nInvalid file path\n");
        exit(3);
    }
    ssize_t reader = read(fileDesc, buff, 0);
    int curr_pos = 0;
    if(reader == -1)
        printf("ERROR\nSpecified path is not a file\n");
    else
    {
        while((reader = read(fileDesc, buff, 1)) != 0 && curr_pos < 1000000)
        {
            text[curr_pos] = buff[0];
            curr_pos++;
        }
    }
    char magic[5];
    int i = 0;
    while(i < 4)
    {
    magic[i] = text[i];
    i++;
    }
    limit = i;
    if(magic[0] != '8' || magic[1] !='O' || (magic[2]!='q' && magic[3]!='Q'))
            strcpy(faults[curr_fault ++], "magic");


    while(i < 6)
    {
    i++;
    }

    limit = i + 2;
    char version = text[i ++];
    if(version < 68 || version > 121)
        strcpy(faults[curr_fault ++], "version");


    int no_sect = text[i ++] & 0x00FF;
    if(no_sect < 3 || no_sect > 10)
        {strcpy(faults[curr_fault ++], "sect_nr");
        goto ERROR;
        }
    char sect_names[100][100];
    char sect_types[100];
    char sect_sizes[100][100];
    char sect_offsets[100][100];
    if(no_sect >= 3 || no_sect <= 10)
    for(int curr_sect = 0; curr_sect < no_sect; curr_sect ++)
    {
        int j = 0;
        char sect_name[40];
        for(j = 0; j < 19; j ++)
            sect_name[j] = text[j + limit];
        int sect_type = text[j + limit];
        sect_types[curr_sect] = sect_type;
        if(sect_type != 24 && sect_type != 46 && sect_type != 31 && sect_type != 43 && sect_type != 67 && sect_type != 40 && sect_type != 46)
            {
            strcpy(faults[curr_fault ++], "sect_types");
            break;
            }
        limit += j + 1;
        char sect_offset[40] = "";
        char sect_size[40] = "";
        j = 0;
        while(j < 3)
        {
            sect_offset[j] = text[j + limit];
            j++;
        }
        j = 0;
        limit += 4;
        while(j < 3)
        {
            sect_size[j] = text[j + limit];
            j++;
        }
        limit += 4;
        strcpy(sect_sizes[curr_sect], sect_size);
        strncpy(sect_names[curr_sect], sect_name, 19);
        strcpy(sect_offsets[curr_sect], sect_offset);
    }
    ERROR:
    if(curr_fault != 0)
        {
            validFind = 0;
            if(findAll == 0)
            {
            printf("ERROR\n");
            printf("wrong ");
            printf("%s", faults[0]);
            if(curr_fault > 1)
            printf("|%s", faults[1]);
            if(curr_fault > 2)
            printf("|%s", faults[2]);
            if(curr_fault > 3)
            printf("|%s", faults[3]);
            }
        }
    else {
    if(findAll == 0)
    {
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", no_sect);}
    for(int p = 0; p < no_sect; p ++)
    {
    char trans[4];
    trans[0] = (sect_sizes[p][3]);
    trans[1] = (sect_sizes[p][2]);
    trans[2] = (sect_sizes[p][1]);
    trans[3] = (sect_sizes[p][0]);
    int x = trans[3] & 0x00FF;
    int y = trans[2] & 0x00FF;
    int z = trans[1] & 0x00FF & 0x0000;
    int w = trans[0] & 0x00FF & 0x0000;
    long size = x + 16 * 16 * y + 16 * 16 * 16 *16 * z + 16 * 16 * 16 * 16 * 16 * 16 * w;
    if(strstr(curr_path,".LoS"))
        validFind = 0;
    if(size > 1197)
    {
        validFind = 0;
    }
    if(size == 0l)
    printf("ASDF");
    trans[0] = (sect_offsets[p][3]);
    trans[1] = (sect_offsets[p][2]);
    trans[2] = (sect_offsets[p][1]);
    trans[3] = (sect_offsets[p][0]);
    x = trans[3] & 0x00FF;
    y = trans[2] & 0x00FF;
    z = trans[1] & 0x00FF;
    w = trans[0] & 0x00FF;
    char name[19];
    for(int c = 0; c < 19; c ++)
        name[c] = sect_names[p][c];
    name[19] = '\0';
    if(findAll == 0)
    printf("section%d: %s %d %ld\n", p + 1, name, sect_types[p], size);
    }
    }
        if(findAll == 1 && validFind == 1)
    {
        printf("%s\n", curr_path);
    }
  
}
void extract(char *dirName, char *curr_path, int t_sect, int t_line)
{
    t_sect --;
    int fileDesc;
    char buff[10];
    char text[1000000];
    char faults[10][150];
    int curr_fault = 0;
    int limit = 0;
    fileDesc = open(curr_path, O_RDONLY);
    if (fileDesc == -1)
    {
        printf ("ERROR\ninvalid file\n");
        exit(3);
    }
    ssize_t reader = read(fileDesc, buff, 0);
    int curr_pos = 0;
    if(reader == -1)
        printf("ERROR\nSpecified path is not a file\n");
    else
    {
        while((reader = read(fileDesc, buff, 1)) != 0 && curr_pos < 1000000)
        {
            text[curr_pos] = buff[0];
            curr_pos++;
        }
    }
    char magic[5] = "";
    int i = 0;
    while(i < 4)
    {
    magic[i] = text[i];
    i++;
    }
    limit = i;
    if(strcmp(magic, "8OqQ") != 0)
            strcpy(faults[curr_fault ++], "magic");


    while(i < 6)
    {
    i++;
    }
    limit = i + 2;
    char version = text[i ++];
    if(version < 68 || version > 121)
        strcpy(faults[curr_fault ++], "version");


    int no_sect = text[i ++] & 0x00FF;
    if(no_sect < 3 || no_sect > 10)
        {strcpy(faults[curr_fault ++], "sect_nr");
        goto ERROR;
        }
    char sect_names[100][300];
    char sect_sizes[100][300];
    char sect_offsets[100][300];
    if(no_sect >= 3 || no_sect <= 10)
    for(int curr_sect = 0; curr_sect < no_sect; curr_sect ++)
    {
        int j = 0;
        char sect_name[30];
        for(j = 0; j < 19; j ++)
            sect_name[j] = text[j + limit];
        int sect_type = text[j + limit];
        if(sect_type != 24 && sect_type != 46 && sect_type != 31 && sect_type != 43 && sect_type != 67 && sect_type != 40 && sect_type != 46)
            {
            strcpy(faults[curr_fault ++], "sect_types");
            break;
            }
        limit += j + 1;
        char sect_offset[40]="";
        char sect_size[40]="";
        j = 0;
        while(j < 3)
        {
            sect_offset[j] = text[j + limit];
            j++;
        }
        j = 0;
        limit += 4;
        while(j < 3)
        {
            sect_size[j] = text[j + limit];
            j++;
        }
        limit += 4;
        strcpy(sect_sizes[curr_sect], sect_size);
        strncpy(sect_names[curr_sect], sect_name, 19);
        strcpy(sect_offsets[curr_sect], sect_offset);
    }
    ERROR:
    if(curr_fault != 0)
        {
            printf("ERROR\n");
            printf("wrong ");
            printf("%s", faults[0]);
            if(curr_fault > 1)
            printf("|%s", faults[1]);
            if(curr_fault > 2)
            printf("|%s", faults[2]);
            if(curr_fault > 3)
            printf("|%s", faults[3]);
        }
    else {
    printf("SUCCESS\n");
    char trans[5];
    trans[0] = (sect_offsets[t_sect][3]);
    trans[1] = (sect_offsets[t_sect][2]);
    trans[2] = (sect_offsets[t_sect][1]);
    trans[3] = (sect_offsets[t_sect][0]);
    int x = trans[3] & 0x00FF;
    int y = trans[2] & 0x00FF;
    int z = trans[1] & 0x00FF;
    int w = trans[0] & 0x00FF;

    long offset = x + 16 * 16 * y + 16 * 16 * 16 *16 * z + 16 * 16 * 16 * 16 * 16 * 16 * w;

    trans[0] = (sect_sizes[t_sect][3]);
    trans[1] = (sect_sizes[t_sect][2]);
    trans[2] = (sect_sizes[t_sect][1]);
    trans[3] = (sect_sizes[t_sect][0]);
    x = trans[3] & 0x00FF;
    y = trans[2] & 0x00FF;
    z = trans[1] & 0x00FF;
    w = trans[0] & 0x00FF;

    long size = x + 16 * 16 * y + 16 * 16 * 16 *16 * z + 16 * 16 * 16 * 16 * 16 * 16 * w;

    int line_offsets[1000];
    int curr_line = 1;
    line_offsets[0] = offset - 1;
    lseek(fileDesc, offset, SEEK_SET);
    char buff[1];
    int curr_size = 0;
    while(curr_size < size)
    {
        read(fileDesc, buff, 1);
        if(buff[0] == 0x0A)
        {
        line_offsets[curr_line] = curr_size + offset;
        curr_line ++;
        }
        curr_size ++;
    }
    int d_line = curr_line - t_line;
    if(d_line == -1)
    d_line ++;
    offset = line_offsets[d_line] + 1;
    lseek(fileDesc, offset, SEEK_SET);
    int curr_place = offset;
    int boundary = 0;
    if(t_line == 1)
        boundary = offset + size;
    else boundary = line_offsets[curr_line - t_line + 1];
    while(curr_place < boundary)
    {
        reader=read(fileDesc, buff, 1);
        if(reader == 0 || buff[0] == 0x0A || buff[0] == 0x00)
        break;
        printf("%c", buff[0]);
        curr_place ++;
    }
    }

}
void findAll(char *dirName, char *curr_path)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if (dir == 0) {
        printf("Error opening directory\n");
        exit(4);
    }

    while ((dirEntry=readdir(dir)) != 0) {
        snprintf(name, MAX_PATH_LEN, "%s/%s",dirName,dirEntry->d_name);
        lstat (name, &inode);
        if(strcmp(dirEntry->d_name,".") == 0 || strcmp(dirEntry->d_name,"..") == 0)
                continue;
            if (S_ISREG(inode.st_mode))
            {
                char nextDir[]="";
                char new_path[MAX_PATH_LEN];
                strcpy(new_path, curr_path);
                strcat(new_path, "/");
                strcat(new_path, dirEntry->d_name);
                strcat(nextDir, dirName);
                strcat(nextDir,"/");
                strcat(nextDir, dirEntry->d_name);
                parse(nextDir, new_path, 1);
            }
            else if (S_ISDIR(inode.st_mode))
            {
                char nextDir[]="";
                char new_path[MAX_PATH_LEN];
                strcpy(new_path, curr_path);
                strcat(new_path, "/");
                strcat(new_path, dirEntry->d_name);
                strcat(nextDir, dirName);
                strcat(nextDir,"/");
                strcat(nextDir, dirEntry->d_name);
                findAll(nextDir, new_path);
            }
    }

    closedir(dir);
}
int main(int argc, char **argv)
{
    if(argc == 1)
    {
        printf("Arguments needed!\n");
        return 1;
    }
    if(strcmp(argv[1], "variant") == 0)
        {
        printf("77649\n");
        exit(0);
        }
    if(strcmp(argv[1], "list") == 0)
    {
        char cwd[MAX_PATH_LEN];
        getcwd(cwd,sizeof(cwd));
        strcat(cwd, "/");
        int found_path = 0;
        int is_filtered_size = 0;
        int list_recursive = 0;
        int is_filtered_write = 0;
        char size_filter[100] = "-1";
        char curr_path[MAX_PATH_LEN];
        for(int i = 2; i < argc; i ++)
        {
            if(strstr(argv[i], "path="))
            {
                found_path = 1;
                strcat(cwd, argv[i] + 5);
                strcpy(curr_path, argv[i] + 5);
            }
            if(strcmp(argv[i], "recursive") == 0)
            {
                list_recursive = 1;
            }
            if(strstr(argv[i], "size_smaller="))
            {
                strcpy(size_filter, argv[i] + 13);
                is_filtered_size = 1;
            }
            if(strstr(argv[i], "has_perm_write"))
            {
                is_filtered_write = 1;
            }
        }
        if(found_path)
        {
            printf("SUCCESS\n");
            listElem(cwd, curr_path, list_recursive, atol(size_filter), is_filtered_size, is_filtered_write);
        }
        else printf("Usage: list [recursive] <filtering_options> path=<dir_path>\n");
    }
    else if(strcmp(argv[1], "parse") == 0 || strcmp(argv[2], "parse") == 0)
    {
        char cwd[MAX_PATH_LEN];
        getcwd(cwd,sizeof(cwd));
        strcat(cwd, "/");
        int found_path = 0;
        char curr_path[MAX_PATH_LEN];
        for(int i = 1; i < argc; i ++)
        {
            if(strstr(argv[i], "path="))
            {
                found_path = 1;
                strcat(cwd, argv[i] + 5);
                strcpy(curr_path, argv[i] + 5);
            }
        }
        if(found_path)
        {
            parse(cwd, curr_path, 0);
        }
        else printf("Usage: parse path=<dir_path>\n");
    }
    else if(strcmp(argv[1], "extract") == 0 || strcmp(argv[2], "extract") == 0)
    {
        char cwd[MAX_PATH_LEN];
        getcwd(cwd,sizeof(cwd));
        strcat(cwd, "/");
        int found_path = 0;
        int found_section = 0;
        char sect[10];
        char line[10];
        int found_line = 0;
        char curr_path[MAX_PATH_LEN];
        for(int i = 1; i < argc; i ++)
        {
            if(strstr(argv[i], "path="))
            {
                found_path = 1;
                strcat(cwd, argv[i] + 5);
                strcpy(curr_path, argv[i] + 5);
            }
            if(strstr(argv[i], "section="))
            {
                found_section = 1;
                strcpy(sect, argv[i] + 8);
            }
            if(strstr(argv[i], "line="))
            {
                found_line = 1;
                strcpy(line, argv[i] + 5);
            }
        }
        if(found_path && found_section && found_line)
        {
            extract(cwd, curr_path, atoi(sect), atoi(line));
        }
        else printf("Usage: extract path=<dir_path> section=<sect_nr> line=<line_nr>\n");
    }
    else if(strcmp(argv[1], "findall") == 0)
    {
        char cwd[MAX_PATH_LEN];
        getcwd(cwd,sizeof(cwd));
        strcat(cwd, "/");
        int found_path = 0;
        char print_dir[MAX_PATH_LEN];
        char curr_path[MAX_PATH_LEN];
        for(int i = 2; i < argc; i ++)
        {
            if(strstr(argv[i], "path="))
            {
                found_path = 1;
                strcat(cwd, argv[i] + 5);
                strcpy(curr_path, argv[i] + 5);
                strcpy(print_dir, argv[i] + 5);
            }
        }
        if(found_path)
        {
        printf("SUCCESS\n");
            findAll(cwd, curr_path);
        }
    }
    return 0;
}
