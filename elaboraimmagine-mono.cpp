/*
 * Operatori a convoluzione
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static void CaricaBmp(const char *Nome, unsigned char *header, unsigned int &dim_head_bmp, unsigned char *img, unsigned int & sx, unsigned int & sy)
{
	FILE *fHan = fopen(Nome, "rb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}
    fseek(fHan,14,0);
    int temp;
    fread(&temp,sizeof(int), 1, fHan);
    fread(&sx,sizeof(int), 1, fHan); // lettura 18-esimo byte che contiene la larghezza [4byte]
    fread(&sy,sizeof(int), 1, fHan); // lettura 22-esimo byte che contiene l' altezza   [4byte]
    //dim_head_bmp += 14;
    rewind(fHan);
	fread(header, dim_head_bmp, 1, fHan);
	printf("%d x %d\n",sx,sy);
	fread(img, (sx * sy), 1, fHan);
	fclose(fHan);
}

static void SalvaBmp(const char *Nome, unsigned char * header, const unsigned int dim_head_bmp, unsigned char *DaDove, int x, int y)
{
	FILE *fHan = fopen(Nome, "wb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}
	// modificare l'
    memcpy(header+18, &x, sizeof(unsigned int)); //modifica larghezza
    memcpy(header+22, &y, sizeof(unsigned int)); //modifica altezza
    fwrite(header, dim_head_bmp, 1, fHan);
	fwrite(DaDove, x * y, 1, fHan);
	fclose(fHan);
}

// kernel

#define KD	3
#define OFS	((KD - 1) / 2)


int Kernel[KD * KD] = {
	0,	-2,	0,
	-2,	8,	-2,
	0,	-2,	0
};

/*
int Kernel[KD * KD] = {
	0,	0,	1,	0,	0,
	0,	1,	2,	1,	0,
	1,	2,	3,	2,	1,
	0,	1,	2,	1,	0,
	0,	0,	1,	0,	0
};
*/

//int Scala = 1;

// operatori

//int Pixel(int x, int y);
//void Convoluzione();
void Ridimensiona(unsigned char * source,unsigned int sx, unsigned int sy, unsigned char * dest,unsigned int dx, unsigned int dy);
int LeggiInterpolato(unsigned char * source, unsigned int sx, unsigned int sy, float x, float y);

int main(int argc, char *argv[])
{
	unsigned char ImmagineS1[480000];
	unsigned char header[1078];
	unsigned int dim_head_bmp=1078;
	unsigned int sx=0;
	unsigned int sy=0;
    unsigned int dx = 100; /*atoi(argv[3]);*/
    unsigned int dy = 100; /*atoi(argv[4]);*/
    unsigned char ImmagineD[dx*dy];
	CaricaBmp("img400x300.bmp", header, dim_head_bmp, ImmagineS1, sx, sy);
	//Convoluzione();
    printf("X %d Y %d",dx,dy);
	Ridimensiona(ImmagineS1,sx,sy,ImmagineD,dx,dy);

	SalvaBmp("output.bmp", header, dim_head_bmp, ImmagineD, dx, dy);

	return 0;

}

// Implementazione
/*
int Pixel(int x, int y)
{
	int u, v;
	int a;
	int p = 0;

	for(v = -OFS;v <= OFS;v++) {
		if(y + v < 0 || y + v >= SY) continue;
		for(u = -OFS;u <= OFS;u++) {
			if(x + u < 0 || x + u >= SX) continue;

			a = ImmagineS1[x + u + ((y + v) * SX)];
			p += (a * Kernel[u + OFS + ((v + OFS) * KD)]);
		}
	}

	p /= Scala;

	if(p < 0) p = 0;
	if(p > 255) p = 255;

	return(p);
}

void Convoluzione()
{
	int x, y;

	for(y = 0;y < SY;y++) {
		for(x = 0;x < SX;x++) {
			ImmagineD[x + (y * DX)] = Pixel(x, y);
		}
	}

}*/

void Ridimensiona(unsigned char * source, unsigned int sx, unsigned int sy, unsigned char * dest, unsigned int dx, unsigned int dy){
   float scalex =  (float)sx / (float)dx;
   float scaley = (float)sy / (float)dy;
   int l;
   float u, v;
   printf("\nscX %f scY %f\n",scalex,scaley);
        for(int y = 0;y < dy;y++) {
            for(int x = 0;x < dx;x++) {
                u = scalex * x;
                v = scaley * y;
                l = LeggiInterpolato(source,sx,sy,u, v);
                if(l > 255) l = 255;
                if(l < 0) l = 0;
                dest[ x + (y * dx)] = l;
            }
        }
	//Moto probabilmente serve un for per scorrere i canali
    printf("Ridimensionato\n");
}

/*void Ridimensiona(unsigned char * source, unsigned int sx, unsigned int sy, unsigned char * dest, unsigned int dx, unsigned int dy){
   int scalex =  sx / dx;
   int scaley = sy / dy;
   printf("\nscX %d scY %d\n",scalex,scaley);

    for(int c = 0; c<3; c++){
    for(int y = 0;y < sy;y++) {
		for(int x = 0;x < sx;x++) {
		    if(x%scalex == 0 && y%scaley == 0){
            //printf("X %d, Y %d\n", x/scalex , y/scaley);
            if (x/scalex <dx && y/scaley <dy){
			dest[(x/scalex) + ((y/scaley) * dx) + (dx*dy*c)] = source[x + (y * sx) + (sx*sy*c)];
            }
		    }
		}
	}
    }
	//Moto probabilmente serve un for per scorrere i canali
    printf("Ridimensionato\n");
}*/






int LeggiInterpolato(unsigned char * source, unsigned int sx, unsigned int sy, float x, float y)
{
	float v1, v2, v3, v4;
	int X = (int) x;
	int Y = (int) y;

	x -= X;
	y -= Y;

	if(X < 0) X = 0;
	if(X >= sx - 1) X = sx - 1;
	if(Y < 0) Y = 0;
	if(Y >= sy - 1) Y = sy - 1;

	v1 = source[X + (sx * Y)];
	v2 = source[X + 1 + (sx * Y)];
	v3 = source[X + (sx * (Y + 1))];
	v4 = source[X + 1 + (sx * (Y + 1))];

	return( (v1 * (1 - x) * (1 - y)) +
			(v2 * x * (1 - y)) +
			(v3 * (1 - x) * y) +
			(v4 * x * y) );
}
