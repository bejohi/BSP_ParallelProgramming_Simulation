#include "task2.h"


#define DEBUG 1
#define DEEP_DEBUG 1
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


    if(DEBUG) printf("...distribution for processorId=%d\n",processorId);

    int start = (int) (n / numberOfProcessors * processorId);
    int end = (int) (n / numberOfProcessors * (processorId + 1));
    int nrows = end - start;

    // Matrix init
    double* pointerA = (double*) malloc(sizeof(double)*nrows*nrows);
    double* pointerB = (double*) malloc(sizeof(double)*nrows*nrows);
    double* pointerC = (double*) malloc(sizeof(double)*nrows*nrows);
    double** matrixA = (double**) malloc(sizeof(double*) * nrows);
    double** matrixB = (double**) malloc(sizeof(double*) * nrows);
    double** matrixC = (double**) malloc(sizeof(double*) * nrows);
    double* iRow = (double*) malloc(sizeof(double)*n);
    double* jColum = (double*) malloc(sizeof(double)*n);

    if(DEBUG) printf("...allocation done for processorId=%d\n",processorId);

    for(int i = 0; i < nrows; i++){
        matrixA[i] = pointerA + i*nrows;
        matrixB[i] = pointerB + i*nrows;
        matrixC[i] = pointerC + i*nrows;
    }

    if(DEBUG) printf("...SUB allocation done for  processorId=%d\n",processorId);

    srand((unsigned int) (time(NULL) * processorId));
    for(int i = 0; i < nrows; i++){
        for(int y = 0; y < nrows; y++){
            matrixA[i][y] = (double)rand()/(double)(RAND_MAX);
            matrixB[i][y] = (double)rand()/(double)(RAND_MAX);
            matrixC[i][y] = 0;
        }
    }

    if(DEBUG) printf("...fill done for  processorId=%d\n",processorId);

    bsp_push_reg(pointerA,n*nrows*sizeof(double));
    bsp_push_reg(pointerB,n*nrows*sizeof(double));
    bsp_push_reg(pointerC,n*nrows*sizeof(double));
    bsp_sync();

    int s = (int) sqrt(numberOfProcessors);
    int iProcessor = iToCheck / nrows * s + jToCheck / nrows;
    int iRemote = iToCheck % nrows;


    // Collect the i-th row and j-colum
    
    bsp_sync();

    for(int localP = 0; localP < s;localP++){
        bsp_get(localP+s*iToCheck/nrows,pointerA,iRemote*sizeof(double)*nrows,iRow,nrows*sizeof(double));
    }

    for(int localP = 0; localP < s;localP++){
        for(int localN = 0; localN < nrows;localN++){
            bsp_get(localP*s+jToCheck/nrows,pointerB,(localN*nrows+jToCheck)*sizeof(double),jColum+localP*nrows+localN,sizeof(double));
            bsp_sync();
        }
    }
    
    if(DEBUG) printf("...Matrix init done for processorId=%d\n",processorId);

    // Algorithm begin
    bsp_sync();



     // TODO: what is in case numberOfProcessors = 1? Is this realy correct?
    double timeStart= bsp_time();

    // TODO: Maybe skip step 0 of matrix multiplication.

    for(int iteration = 0; iteration < s; iteration++){

        for (int i=0;i<nrows;i++) {
            for (int k=0;k<nrows;k++) {
                for (int j=0;j<nrows;j++) {
                    matrixC[i][j] += matrixA[i][k]* matrixB[k][j];

                }
            }
        }
        

        int downId = ((processorId + s) % numberOfProcessors);
        int rightId;
        if(processorId / s == s-1){
            rightId = processorId - s + 1;
            if(DEEP_DEBUG) printf("special case with rightId for processorId=%d\n",processorId);
        } else {
            rightId = processorId + 1;
        }

        if(DEEP_DEBUG) printf("iteration %d start for processorId=%d, downId=%d, rightId=%d\n",iteration, processorId,downId,rightId);

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
            sequ_result += iRow[x] * jColum[x];
        }
        bsp_get(iProcessor,pointerC, ((iToCheck % nrows) * nrows + jToCheck) * sizeof(double),&result,sizeof(double)); // pid,source,offset,destination,size
        
    }

    bsp_sync();
    if(processorId == 0){
        if(result != sequ_result){
            printf("CHECK FAILED!\n");
            if(DEBUG) printf("Parallel result for (%d,%d)=%lf\n",iToCheck,jToCheck,result);
            if(DEBUG) printf("Sequential result for (%d,%d)=%lf\n",iToCheck,jToCheck,sequ_result);
        } else {
            printf("Check okay.\n");
            if(DEBUG) printf("Parallel result for (%d,%d)=%lf\n",iToCheck,jToCheck,result);
            if(DEBUG) printf("Sequential result for (%d,%d)=%lf\n",iToCheck,jToCheck,sequ_result);
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