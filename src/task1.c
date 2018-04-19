#include "task1.h"


/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

/**
 * The BSP Main method to run on multiple machines.
 */
void bspEntrance(){
    bsp_begin(numberOfProcessors);
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 100;

    bspEntrance();

    exit(EXIT_SUCCESS);
}