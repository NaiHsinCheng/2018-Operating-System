#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define SIZE 100

#define color "\x1b[;36;1;1m"
#define rst "\x1b[0;m"

int main(int argc, const char *argv[])
{

    int fd;
    int choose;
    printf("1    : empty file\nelse : exist file\n");
    printf("=> ");
    scanf("%d", &choose);
    getchar();
    if(choose == 1)
        fd = open("text", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    else
        fd = open("text", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if(fd == -1)
    {
        printf("file open error!\n");
        exit(EXIT_FAILURE);
    }

    struct stat text_info = {0};
    fstat( fd, &text_info );
    int text_len = (int)text_info.st_size;

    char *map = (char *)mmap(0, getpagesize(), PROT_WRITE, MAP_SHARED, fd, 0);

    if(map == MAP_FAILED)
    {
        close(fd);
        printf("mapping error!\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        char s[SIZE] = {0};

		printf("==============================================\n");
        printf("input string (input q to exit)\n");
        printf("=> ");
        fgets(s, SIZE, stdin);
        int len = strlen(s);

        if(len == 2 && s[0] == 'q')
            break;
        printf("String : "color"%s"rst,s);

        lseek(fd, text_len + len, SEEK_SET);    // find text_len+len from head
        write(fd, "", 1);          // write \n per line
        lseek(fd, 0, SEEK_SET);      // move to head

        for(int i = 0; i < len; ++i)
        {
            map[text_len + i] = s[i];
        }

        text_len += len;
    }

    close(fd);

    return 0;
}
