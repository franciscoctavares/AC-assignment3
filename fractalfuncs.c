//  These are the global variables.  That means they can be changed from
//  any of my functions.  This is usfull for my options function.  That
//  means that these variables are 'user' variables and can be changed
//  and the changes will be stored for the displaying part of the program
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include "fractalfuncs.h"



    
void putpixel(struct IMG * img,int x, int y, int color){
    img->pixels[y*img->cols+x].r=(color*1111)&0xff;
    img->pixels[y*img->cols+x].g=((color*1111)>>4) & 0xff;
    img->pixels[y*img->cols+x].b=((color*1111)>>8) & 0xff;
}

//	***************  JULIA FUNCTION ******************
/*  First, the pixel must be converted to its actual plane value (x,y).
    Then it is passed through the function the number of times in the
    gloabal variable initer(ations).  If the break condition is reached,
    which has been mathmaticaly determined so that if the value of Z is
    larger than that, the function will continue for ever to infinity.
    Depending on the number of times the function was initerated before the
    break value was reached, the color displayed on the screen changes. If
    the break value is never reached, the point is considered to be part of
    the set. The color for that is black (0).
    For the julia set, the Z is the point under consideration.  The
    constant can be changed by the user.
*/

void julia(struct IMG * img, int xpt, int ypt, int col)
{
    int resx=img->cols;
    int resy=img->rows;
    float pixcorx=(Maxx-Minx)/resx;
    float pixcory=(Maxy-Miny)/resy;
    
    long double x=xpt*pixcorx+Minx;
    long double y=Maxy-ypt*pixcory;		//converting from pixels to points
    long double xnew=0;
    long double ynew=0;
    int k;
	
    for(k=0;k<=col;k++)  // Each pixel loop
    {
	//The Julia Function Z=Z*Z+c (of complex numbers) into x and y parts
	xnew=x*x-y*y + conx;
	ynew=2*x*y   + cony;
	x=xnew;
	y=ynew;
	if ( (x*x+y*y)>4 ) break;  // Break condition Meaning the loop will go
	// on to a value of infinity.
    }                      // End each pixel loop

    int color = k;
    // if (color>15) color=color%15;
    if (k>=col)
	putpixel(img,xpt,ypt,0);
    else
	putpixel(img,xpt,ypt,color);		
}


void saveimg(struct IMG * img, char * fname){
	FILE * fp;
	int color,i,j;
	int rx,ry;

	rx=img->cols;
	ry=img->rows;
	fp=fopen(fname,"w");
    if(fp == NULL) {
        printf("Could not open file!");
        return;
    }
	/* header for PPM output */
	fprintf(fp,"P6\n# CREATOR: AC Course, DEEC-UC\n");
	fprintf(fp,"%d %d\n255\n",rx,ry);

	
	for (i=0;i<ry;i++){
	    for (j=0;j<rx;j++){
		
		int red=img->pixels[i*rx+j].r;
		int green=img->pixels[i*rx+j].g;
		int blue=img->pixels[i*rx+j].b;
		
		fputc((char)(red & 0x00ff),fp);
		fputc((char)(green & 0x00ff),fp);
		fputc((char)((blue & 0x00ff) ),fp);
	    }
	}
	fclose(fp);
}

