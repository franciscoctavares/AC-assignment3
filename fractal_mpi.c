#include "mpi.h"

#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include "fractalfuncs.h"
#include <string.h>

#ifdef _OPENMP
    #include <omp.h>
#else
    #define omp_get_thread_num() 0
#endif

void Generate(struct IMG * img, int rows, int cols, int first_color, int last_color) {
    int color;
    int scrsizex, scrsizey;
    scrsizex = cols;
    scrsizey = rows;
    char filename[80];
    struct IMG* local_img;

    local_img = (struct IMG *)malloc(sizeof(struct IMG));
    
    local_img->pixels = (PIXEL *)malloc(cols*rows*sizeof(PIXEL));
    local_img->cols = cols;
    local_img->rows = rows;

    //printf("Hello there\n");

    for(int color = first_color; color < last_color; color++) {
        //printf("color = %d\n", color);
        #pragma omp parallel for collapse(2)
        for(int j = 0; j < scrsizey; j++) {
            for(int i = 0; i < scrsizex; i++) {
                //printf("%d %d\n", j, i);
                julia(local_img, i, j, color);
            }
        }
        sprintf(filename, "imgs/normal/julia_%04d.pgm", color);
        //printf("color = %d\n", color);
        saveimg(local_img, filename);
    }

    free(local_img->pixels);
    free(local_img);
}

int main(int argc, char** argv) {
    clock_t t1,t2,t3;
    int resx,resy;
    struct IMG * img;
    int nepocs=0;
    float alpha=0;

	int rank, size = 4;

    if (argc==1){
	resx=640;
	resy=480;
    } else if ((argc==3)||(argc==5)){
	resx=atoi(argv[1]);
	resy=atoi(argv[2]);
	if(argc==5){
	    nepocs=atoi(argv[3]);
	    alpha=atof(argv[4]);
	    if (alpha<0.0 || alpha>1.0){
		printf("Alpha tem de estar entre 0 e 1\n");
		exit(1);
	    }
	}
    } else {
	printf("Erro no número de argumentos\n");
	printf("Se não usar argumentos a imagem de saida terá dimensões 640x480\n");
	printf("Senão devera especificar o numero de colunas seguido do numero de linhas\n");
	printf("Adicionalmente poderá especificar o numero de epocas de difusao e o factor de difusao,\\ caso contrario serao considerados como 0.");
	printf("\nExemplo: %s 320 240 \n",argv[0]);
	printf("\nExemplo: %s 320 240 100 0.5\n",argv[0]);
	exit(1);
    }

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int n_pro = initer / size;
    Generate(img, resy, resx,  n_pro*rank, n_pro*(rank + 1));

    difuse(img, nepocs, alpha, rank, size);

	MPI_Finalize();
	return 0;
}

void difuse(struct IMG * imgin, int nepocs, float alpha, int rank, int size){
    int rows_per_proc, extra_rows, start_row, end_row, nrows, ncols;
    struct IMG *local_img, *local_img_new;
    float alpha_avg = alpha / 8;
    MPI_Status status;

    nrows = imgin->rows;
    ncols = imgin->cols;

    // Calculate rows per process
    rows_per_proc = nrows / size;
    extra_rows = nrows % size;

    // Assign start and end rows for each process
    start_row = rank * rows_per_proc + (rank < extra_rows ? rank : extra_rows);
    end_row = start_row + rows_per_proc + (rank < extra_rows ? 1 : 0);
    
    // Allocate memory for local chunks
    local_img = allocate_image_chunk(imgin, start_row, end_row, ncols); // Include boundary rows
    local_img_new = allocate_image_chunk(imgin, start_row, end_row, ncols);

    //float alpha_avg = alpha / 8;
    for(int i = 1; i <= nepocs; i++) {
        //int nrows = imgnew->rows;
        //int ncols = imgnew->cols;

        // Exchange boundary rows with neighbors
        if (rank > 0) {
            MPI_Send(local_img->pixels + ncols, ncols, MPI_PIXEL, rank - 1, 0, MPI_COMM_WORLD);
            MPI_Recv(local_img->pixels, ncols, MPI_PIXEL, rank - 1, 0, MPI_COMM_WORLD, &status);
        }
        if (rank < size - 1) {
            MPI_Send(local_img->pixels + (end_row - start_row - 1) * ncols, ncols, MPI_PIXEL, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(local_img->pixels + (end_row - start_row) * ncols, ncols, MPI_PIXEL, rank + 1, 0, MPI_COMM_WORLD, &status);
        }

        #pragma omp parallel for collapse(2)
        for(int y = 1; y < imgnew->rows - 1; y++) {
            for(int x = 1; x < imgnew->cols - 1; x++) {
                // red
                imgnew->pixels[y*ncols+x].r = (1 - alpha)*imgin->pixels[y*ncols+x].r + alpha_avg*(imgin->pixels[(y-1)*ncols+(x-1)].r
                                            + imgin->pixels[(y-1)*ncols+(x-1)].r + imgin->pixels[(y-1)*ncols+(x)].r + imgin->pixels[(y-1)*ncols+(x+1)].r
                                            + imgin->pixels[(y)*ncols+(x-1)].r + imgin->pixels[(y)*ncols+(x+1)].r + imgin->pixels[(y+1)*ncols+(x-1)].r
                                            + imgin->pixels[(y+1)*ncols+(x)].r + imgin->pixels[(y+1)*ncols+(x+1)].r);
                // green
                imgnew->pixels[y*ncols+x].g = (1 - alpha)*imgin->pixels[y*ncols+x].g + (alpha/8)*(imgin->pixels[(y-1)*ncols+(x-1)].g
                                            + imgin->pixels[(y-1)*ncols+(x-1)].g + imgin->pixels[(y-1)*ncols+(x)].g + imgin->pixels[(y-1)*ncols+(x+1)].g
                                            + imgin->pixels[(y)*ncols+(x-1)].g + imgin->pixels[(y)*ncols+(x+1)].g + imgin->pixels[(y+1)*ncols+(x-1)].g
                                            + imgin->pixels[(y+1)*ncols+(x)].g + imgin->pixels[(y+1)*ncols+(x+1)].g);
                // blue
                imgnew->pixels[y*ncols+x].b = (1 - alpha)*imgin->pixels[y*ncols+x].b + (alpha/8)*(imgin->pixels[(y-1)*ncols+(x-1)].b
                                            + imgin->pixels[(y-1)*ncols+(x-1)].b + imgin->pixels[(y-1)*ncols+(x)].b + imgin->pixels[(y-1)*ncols+(x+1)].b
                                            + imgin->pixels[(y)*ncols+(x-1)].b + imgin->pixels[(y)*ncols+(x+1)].b + imgin->pixels[(y+1)*ncols+(x-1)].b
                                            + imgin->pixels[(y+1)*ncols+(x)].b + imgin->pixels[(y+1)*ncols+(x+1)].b);
                        
            }
        }
            
        sprintf(filename, "imgs/difusion/julia_%04d.pgm", i);
        saveimg(imgnew, filename);
        temp = imgin;
        imgin = imgnew;
        imgnew = temp;

        // Gather results at root
        MPI_Gather(local_img->pixels + ncols, (end_row - start_row - 2) * ncols, MPI_PIXEL,
                   imgin->pixels, (end_row - start_row - 2) * ncols, MPI_PIXEL,
                   0, MPI_COMM_WORLD);


        // Root saves image
        if (rank == 0) {
            char filename[80];
            sprintf(filename, "imgs/difusion/julia_%04d.pgm", epoch + 1);
            saveimg(imgin, filename);
        }

        // Broadcast updated image to all processes for next iteration
        MPI_Broadcast(imgin->pixels, nrows * ncols, MPI_PIXEL, 0, MPI_COMM_WORLD);

    }

    free_image_chunk(local_img);
    free_image_chunk(local_img_new);

}
