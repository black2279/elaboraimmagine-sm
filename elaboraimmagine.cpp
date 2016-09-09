/*
 * Operatori a convoluzione
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#define R 56
#define G 48
#define B 40
#define R2 32
#define G2 24
#define B2 16


static void CaricaBmp(const char *Nome, unsigned char *header, unsigned int &dim_head_bmp, unsigned char * &image, unsigned int & sx, unsigned int & sy)
{
	FILE *fHan = fopen(Nome, "rb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}
    fseek(fHan,14,0);
    fread(&dim_head_bmp,sizeof(int), 1, fHan);
    fread(&sx,sizeof(int), 1, fHan); // lettura 18-esimo byte che contiene la larghezza [4byte]
    fread(&sy,sizeof(int), 1, fHan); // lettura 22-esimo byte che contiene l' altezza   [4byte]
    dim_head_bmp += 14;
    rewind(fHan);
	fread(header, dim_head_bmp, 1, fHan);
	image = new unsigned char [sx*sy*3];
	fread(image, (sx * sy)*3, 1, fHan);
	fclose(fHan);
}

static void SalvaBmp(const char *Nome, unsigned char * header, const unsigned int dim_head_bmp, unsigned char *DaDove, int x, int y)
{
	FILE *fHan = fopen(Nome, "wb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}

    memcpy(header+18, &x, sizeof(unsigned int)); //modifica larghezza
    memcpy(header+22, &y, sizeof(unsigned int)); //modifica altezza
    fwrite(header, dim_head_bmp, 1, fHan);
	fwrite(DaDove, (x * y)*3, 1, fHan);
	fclose(fHan);
}

float Bilineare(unsigned char * source, unsigned int sx, unsigned int sy, float x, float y, int n_canali) {

	float v1, v2, v3, v4;
	int X = (int) x;
	int Y = (int) y;

	x -= X;
	y -= Y;

	if(X < 0) X = 0;
	if(X >= ((sx) - 1)) X = (sx) - 1;
	if(Y < 0) Y = 0;
	if(Y >= sy - 1) Y = sy - 1;

    //Ogni coordinata viene moltiplicate per il numero di canali presenti
    v1 = source[X*n_canali + (sx * Y*n_canali) ];
	v2 = source[(X+1)*n_canali + (sx * Y*n_canali)];
	v3 = source[(X)*n_canali + (sx * ((Y+1)*n_canali))];
	v4 = source[(X+1)*n_canali + (sx * ((Y+1)*n_canali))];

	return( ( v1 * (1 - x) * (1 - y)) +
			( v2 * x * (1 - y)) +
			( v3 * (1 - x) * y) +
			( v4 * x * y) );

}

void Ridimensiona(unsigned char * source, unsigned int sx, unsigned int sy, unsigned char * dest, unsigned int dx, unsigned int dy){

   float scalex = ((float)sx) / ((float)dx); //Fattore di scala per la larghezza
   float scaley = ((float)sy) / ((float)dy); //Fattore di scale per l'altezza
   float u, v; //Coordinate da leggere in riferimento all'immagine sorgente

   printf("Fattore di Scala per X: %f\nFattore di Scala per Y: %f\n",scalex,scaley);

/*  Lettura Immagine
            B  G  R
            0  1  2
        0 | FF AF AA
        1 | FF FF FF
        2 | AA FF AA
        ..| .. .. ..
    dx*dy | FF FF FF

*/
   for(int c=0; c<3; c++){ // Canali
        for(int y = 0;y < dy;y++){ //Altezza
            for(int x = 0;x < dx;x++) { // Larghezza
                u = x * scalex; //Fattore di scala per coordinata X
                v = y * scaley; //Fattore di scala per coordinata Y
                float int_canale = Bilineare((source+c),sx,sy,u,v,3); //Ricampionamento con
                                                                    //interpolazione bilineare
                if ( int_canale  > 255 ) int_canale  = 255; //Clamping se maggiore di 255
                if ( int_canale  < 0 ) int_canale  = 0; //Clamping se minore di 0
                dest[c + x*3 +y*dx*3 ] = int_canale; //Scrittura del valore di intensità del canale
                                                      //sull'immagine di destinazione
                }
            }
        }
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

int main(int argc, char *argv[])
{
	unsigned char header[54]; // Header
	unsigned int dim_head_bmp=0; //Dimensione Header
	unsigned int sx=0; //Larghezza Immagine Sorgente
	unsigned int sy=0; //Altezza Immagine Sorgente
    unsigned int dx = 600; /*atoi(argv[3]);*/ //Larghezza Immagine Destinazione
    unsigned int dy = 788; /*atoi(argv[4]);*/ //Altezza Immagine Destinazione
    unsigned char *ImmagineS=0; //Puntatore a Immagine Sorgente
    unsigned char *ImmagineD = new unsigned char[dx*dy*3]; //Creazione spazio per Immagine di Destinazione

	CaricaBmp("acdc_red_ltoh.bmp", header, dim_head_bmp, ImmagineS, sx, sy); //Caricamento Immagine sorgente

    printf("Dimensione immagine: L %d x H %d\n", sx, sy);

	//Convoluzione();

	Ridimensiona(ImmagineS, sx, sy, ImmagineD, dx, dy); //Ridimensionamento Immagine

	SalvaBmp("output.bmp", header, dim_head_bmp, ImmagineD, dx, dy); //Serializzazione Immagine

	//Deallocazione memoria
	delete ImmagineS;
	delete ImmagineD;
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

