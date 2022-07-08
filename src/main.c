#include <stdio.h>
#include <director/director.h>

int main(int argc, char *argv[]) {
    director_t *movie = director_load_file(argv[1]);
    return 0;
}
