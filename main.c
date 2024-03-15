#include "controller.h"
#include <stdlib.h>
#include <time.h>

//#define _FORTIFY_SOURCE
//#define __USE_FORTIFY_LEVEL 3


int main(int argc, char* argv[]) {
    srand(time(0));
    return controller_run(&argc, &argv);
}