#include <stdio.h>
#include <cgi_flash.h>

int main (int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s <filename> <device>\n", argv[0]);
        return -1;
    }
    const char* error = erase_flash(argv[2]);
    if (error) {
        printf("%s\n", error);
        return -1;
    }
    if ((error = write_flash(argv[1], argv[2]))) {
        printf("%s\n", error);
        return -1;
    }
    printf("Flashed %s to %s\n", argv[1], argv[2]);
    return 0;
}

