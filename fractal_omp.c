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

void Generate(struct IMG * img) {
    int color;
    int scrsizex, scrsizey;
    scrsizex = img->cols;
    scrsizey = img->rows;
    char filename[80];
    struct IMG* local_img = img;
    int thread_id;
    
    //#pragma omp parallel
    //{
        thread_id = omp_get_thread_num();
        for(int color = 0; color < initer; color++) {
            //#pragma omp for
            #pragma omp parallel for collapse(2)
            for(int j = 0; j < scrsizey; j++) {
                for(int i = 0; i < scrsizex; i++) {
                    julia(img, i, j, color);
                }
            }
            sprintf(filename, "imgs/normal/julia_%04d.pgm", color);
            saveimg(img, filename);
        }
    //}
}

void difuse(struct IMG * imgin, int nepocs, float alpha){
    struct IMG * temp,*imgnew;
    int i;
    char filename[80];
    
    imgnew = (struct IMG *) malloc(sizeof(struct IMG));
    imgnew->rows = imgin->rows;
    imgnew->cols = imgin->cols;
    imgnew->pixels = (PIXEL *)malloc(imgnew->cols*imgnew->rows*sizeof(PIXEL));

    float alpha_avg = alpha / 8;
    for(i = 1; i <= nepocs; i++) {
        int nrows = imgnew->rows;
        int ncols = imgnew->cols;
        //#pragma omp parallel for collapse(2)
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
    }
}
    
int main(int argc, char ** argv){
    clock_t t1,t2,t3,t4;
    int resx,resy;
    struct IMG * img;
    int nepocs=0;
    float alpha=0;

	
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
    img=(struct IMG *)malloc(sizeof(struct IMG));
    
    img->pixels=(PIXEL *)malloc(resx*resy*sizeof(PIXEL));
    img->cols=resx;
    img->rows=resy;
    
	
    t1=clock();
    Generate(img);
    t2=clock();
    printf("Julia Fractal gerado em %6.3f secs.\n",(((double)(t2-t1))/CLOCKS_PER_SEC));
    //	mandel(img,resx,resy);
    //saveimg(img, "julia.pgm");
    //difuse(img, 50, 0.5);
    t3=clock();
    //printf("Julia Fractal com difusão gerado em %6.3f secs.\n",(((double)(t3-t2))/CLOCKS_PER_SEC));
    //printf("Tempo total: %6.3f secs.\n", (((double)(t3-t1))/CLOCKS_PER_SEC));
    
    if(nepocs>0)
	difuse(img,nepocs,alpha);

    t4 = clock();
    printf("Difusão gerada em %6.3f secs.\n",(((double)(t4-t3))/CLOCKS_PER_SEC));
}
										  
