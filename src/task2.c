#include "task2.h"


#define DEBUG 1
#define DEEP_DEBUG 0
#define REPORT_MODE 0

static unsigned int numberOfProcessors;
static long globalN;
static int globalI;
static int globalJ;


/**
 * This method will run on every machine.
 */
void cannonMatrixMult(){

    bsp_begin(numberOfProcessors);

    unsigned int numberOfProcessors = bsp_nprocs();
    unsigned int processorId = bsp_pid();
    long n = globalN;
    int iToCheck = globalI;
    int jToCheck = globalJ;
    int s = (int) sqrt(numberOfProcessors);
    int nrows = (int) (n / s);

    // Distribution
    bsp_push_reg(&n,sizeof(long));
    bsp_push_reg(&iToCheck,sizeof(int));
    bsp_push_reg(&jToCheck,sizeof(int));
    bsp_sync();

    bsp_get(0,&n,0,&n,sizeof(long));
    bsp_get(0,&iToCheck,0,&iToCheck,sizeof(int));
    bsp_get(0,&jToCheck,0,&jToCheck,sizeof(int));
    bsp_sync();
    bsp_pop_reg(&n);
    bsp_pop_reg(&iToCheck);
    bsp_pop_reg(&jToCheck);


    if(DEEP_DEBUG) printf("...distribution for processorId=%d\n",processorId);




    // Matrix init
    double* pointerA = (double*) malloc(sizeof(double)*nrows*nrows);
    double* pointerB = (double*) malloc(sizeof(double)*nrows*nrows);
    double* pointerC = (double*) malloc(sizeof(double)*nrows*nrows);
    double** matrixA = (double**) malloc(sizeof(double*) * nrows);
    double** matrixB = (double**) malloc(sizeof(double*) * nrows);
    double** matrixC = (double**) malloc(sizeof(double*) * nrows);
    double* iRow = (double*) malloc(sizeof(double)*n);
    double* jColum = (double*) malloc(sizeof(double)*n);

    if(DEEP_DEBUG) printf("...allocation done for processorId=%d\n",processorId);

    for(int i = 0; i < nrows; i++){
        matrixA[i] = pointerA + i*nrows;
        matrixB[i] = pointerB + i*nrows;
        matrixC[i] = pointerC + i*nrows;
    }

    if(DEEP_DEBUG) printf("...SUB allocation done for  processorId=%d\n",processorId);

    if(processorId == 0){
        if(DEBUG) printf("...nrows=%d\n",nrows);
    }

    srand((unsigned int) (time(NULL) * processorId));
    for(int i = 0; i < nrows; i++){
        for(int y = 0; y < nrows; y++){
            matrixA[i][y] = (double)rand()/(double)(RAND_MAX);
            matrixB[i][y] = (double)rand()/(double)(RAND_MAX);
            matrixC[i][y] = 0;
        }
    }

    if(DEEP_DEBUG) printf("...fill done for  processorId=%d\n",processorId);

    bsp_push_reg(pointerA,nrows*nrows*sizeof(double));
    bsp_push_reg(pointerB,nrows*nrows*sizeof(double));
    bsp_push_reg(pointerC,nrows*nrows*sizeof(double));
    bsp_sync();

    if(DEEP_DEBUG) printf("...push done for processorId=%d\n",processorId);

    int iProcessor = iToCheck / nrows * s + jToCheck / nrows;
    int iRemote = iToCheck % nrows;


    // Collect the i-th row and j-colum
    
    bsp_sync();

    for(int localP = 0; localP < s;localP++){
        bsp_get((const unsigned int) (localP + s * iToCheck / nrows), pointerA,
                iRemote * sizeof(double) * nrows, iRow + localP * nrows, nrows * sizeof(double));
        bsp_sync();
    }

    if(DEEP_DEBUG) printf("...iRow init done for processorId=%d\n",processorId);
    bsp_sync();
    for(int localP = 0; localP < s; localP++){
        for(int localN = 0; localN < nrows;localN++){
            if(DEEP_DEBUG) printf("localP=%d localN=%d for processorId=%d\n",localP,localN,processorId);
            // (localN*n+get_j)*sizeof(double)
            bsp_get((const unsigned int) (localP * s + jToCheck / nrows), pointerB,
                    (localN * nrows + jToCheck) * sizeof(double), jColum + localP * nrows + localN, sizeof(double));
            bsp_sync();
        }
    }

    if(DEEP_DEBUG) printf("...jColum init done for processorId=%d\n",processorId);
    if(DEEP_DEBUG) printf("...Matrix init done for processorId=%d\n",processorId);

    // Algorithm begin
    bsp_sync();


    double timeStart= bsp_time();

    // TODO: Build initial shift

    int ii = (processorId % s);
    int jj = (processorId / s);

    unsigned int downId = (processorId + ii * s) % numberOfProcessors;

    unsigned int rightId = processorId / s == s-1 ? processorId - s + jj : processorId + jj;

    bsp_get(downId,pointerB,0,pointerB, sizeof(double) * nrows * nrows);
    if(DEBUG) printf("...pointerB movement done downId=%d rightId=%d for processorId=%d\n",downId,rightId,processorId);

    bsp_get(rightId,pointerA,0,pointerA, sizeof(double) * nrows * nrows);

    if(DEBUG) printf("...pointerA movement done downId=%d rightId=%d for processorId=%d\n",downId,rightId,processorId);
    bsp_sync();

    for(int iteration = 0; iteration < s; iteration++){

        for (int i=0;i<nrows;i++) {
            for (int k=0;k<nrows;k++) {
                for (int j=0;j<nrows;j++) {
                    matrixC[i][j] += matrixA[i][k] * matrixB[k][j];

                }
            }
        }



        if(DEEP_DEBUG) printf("iteration %d start for processorId=%d, downId=%d, rightId=%d\n",iteration, processorId,downId,rightId);
        downId = ((processorId + s) % numberOfProcessors);
        rightId = processorId / s == s-1 ? processorId - s + 1 : processorId + 1;;
        bsp_get(downId,pointerA,0,pointerA,sizeof(double) * nrows * nrows);
        bsp_get(rightId,pointerB,0,pointerB,sizeof(double) * nrows * nrows);

        if(DEEP_DEBUG) printf("iteration %d done for processorId=%d\n",iteration, processorId);
    }
    
    bsp_sync();
    double timeEnd= bsp_time();
    if(DEBUG) printf("...calculations done for processorId=%d\n",processorId);


    bsp_sync();
    if(processorId==0){
        printf("...calculations done in %.6lf seconds\n",timeEnd-timeStart);
    }

    // Verify result
    double sequ_result = 0;
    double result = 0;

    if(processorId == 0){
        for(int x = 0; x < n; x++){
            if(DEEP_DEBUG) printf("iRow[%d]=%lf done jColum[%d]=%lf\n",x,iRow[x],x,jColum[x]);
            sequ_result += iRow[x] * jColum[x];
        }
        bsp_get(iProcessor,pointerC, ((iToCheck % nrows) * nrows + jToCheck) * sizeof(double),&result,sizeof(double));
        
    }

    bsp_sync();
    if(processorId == 0){
        if(result != sequ_result){
            printf("CHECK FAILED!\n");
            if(DEBUG) printf("Parallel result for (%d,%d)=%lf\n",iToCheck,jToCheck,result);
            if(DEBUG) printf("Sequential result for (%d,%d)=%lf\n",iToCheck,jToCheck,sequ_result);
        } else {
            printf("Check okay.\n");
        }
    }

    // Clean-Up
    free(pointerA);
    free(pointerB);
    free(pointerC);
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(iRow);
    free(jColum);

    bsp_end();
}

int main(int argc, char **argv){
    bsp_init(cannonMatrixMult, argc, argv);
    numberOfProcessors = 1;
    globalN = 1800;
    globalI = 5;
    globalJ = 5;
    printf("Please enter number of processors:");
    scanf("%d", &numberOfProcessors);
    if(!REPORT_MODE){
        printf("\nPlease enter size of matrix:");
        scanf("%ld", &globalN);
        printf("\nPlease enter i:");
        scanf("%d", &globalI);
        printf("\nPlease enter j:");
        scanf("%d", &globalJ);
        printf("\n");
    }

    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }
    printf("Start program with n=%ld,p=%d...\n",globalN,numberOfProcessors);
    cannonMatrixMult();

    exit(EXIT_SUCCESS);
}