#include <stdio.h>
#include <director/director.h>
#include <renderer/renderer.h>

int main(int argc, char *argv[]) {
    director_t *movie = director_load_file(argv[1]);
    if (movie != NULL) {
        renderer_test();
    }
    return 0;
}
