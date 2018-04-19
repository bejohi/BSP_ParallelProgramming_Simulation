#include "task1.h"


/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

static long globalN;

void initMatrix(double** matrix){
    matrix = (double**) malloc(sizeof(double*) * globalN);
    if(!matrix){
        bsp_abort("Not enough memory for allocating the matrix available\n");
    }
    for(long i = 0; i < globalN; i++){
        matrix[i] = (double*) malloc(sizeof(double) * globalN);
        if(!matrix[i]){
            bsp_abort("Not enough memory for allocating the matrix available\n");
        }
    }
}


/**
 * The BSP Main method to run on multiple machines.
 */
void bspEntrance(){

    bsp_begin(numberOfProcessors);
    long p= bsp_nprocs();
    long s= bsp_pid();

    bsp_push_reg(&globalN,sizeof(long));
    bsp_sync();

    bsp_get(0,&globalN,0,&globalN,sizeof(long));
    bsp_sync();
    bsp_pop_reg(&globalN);



    double** matrix;
    initMatrix(matrix);

    bsp_end();
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 100;

    bspEntrance();

    exit(EXIT_SUCCESS);
}