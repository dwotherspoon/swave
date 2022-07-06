#include <stdio.h>
#include <loader.h>

int main(int argc, char *argv[]) {

    loader_load_file(argv[1]);
    return 0;
}
