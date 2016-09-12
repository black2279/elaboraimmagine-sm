
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static void CaricaBmp(const char *Nome, unsigned char *header, unsigned int &dim_head_bmp, unsigned char * &image, unsigned int & sx, unsigned int & sy)
{
	FILE *fHan = fopen(Nome, "rb"); //Apertura del file in lettura
	if(fHan == NULL) {
		printf("File non disponibile o inesistente\n");
		exit(1);
	}
    fseek(fHan,14,0); //Salta Header BMP
    fread(&dim_head_bmp,sizeof(int), 1, fHan); //Lettura dimensione header
    fread(&sx,sizeof(int), 1, fHan); // lettura 18-esimo byte che contiene la larghezza [4byte]
    fread(&sy,sizeof(int), 1, fHan); // lettura 22-esimo byte che contiene l' altezza   [4byte]
    dim_head_bmp += 14; //Somma alla dimensione dell'header dell' immagine quella del formato
    rewind(fHan); // Resetta il puntatore al file
	fread(header, dim_head_bmp, 1, fHan); // Caricamento da file dell'header
	int dim = sx*sy*3; //Dimensione senza padding
	if((sx*3)%4 != 0){
        dim += ((4 - (sx*3)%4) * sy); // Aggiunta del padding se la larghezza non è divisibile per 4
	}
	image = new unsigned char [dim]; //Creazione spazio nello heap per l' Immagine Sorgente
	fread(image, dim, 1, fHan); //Caricamento dei dati dell'Immagine Sorgente da file
	fclose(fHan); //Chiusura del file
}

static void SalvaBmp(const char *Nome, unsigned char * header, const unsigned int dim_head_bmp, unsigned char *DaDove, int x, int y)
{
	FILE *fHan = fopen(Nome, "wb"); //Apertura del file in scrittura
	if(fHan == NULL) {
		printf("Impossibile creare o recuperare il file\n");
		exit(1);
	}

    memcpy(header+18, &x, sizeof(unsigned int)); //modifica larghezza
    memcpy(header+22, &y, sizeof(unsigned int)); //modifica altezza
    fwrite(header, dim_head_bmp, 1, fHan); //Scrittura del nuovo header
    int dim = x*y*3; //Dimensione senza padding
	if((x*3)%4 != 0){
     dim += ((4 - (x*3)%4) * y); // Aggiunta del padding se la larghezza non è divisibile per 4
	}
	fwrite(DaDove, dim, 1, fHan); // Trasferimento dell' Immagine Destinazione su file
	fclose(fHan); // Chiusura del file
}

void RimuoviPadding(unsigned char * source, int width, int height, int padding, unsigned char * dest){
    int offset = 0;
    for(int y=0; y<height; y++ ){
        for(int x=0; x<width*3; x++ ){
            dest[x + y*width*3] = source[x+offset + y*width*3];
        }
        offset += padding;
    }
}

void AggiungiPadding(unsigned char * source, int width, int height, int padding, unsigned char * dest){
    int offset = 0;
    for(int y=0; y<height; y++ ){
        for(int x=0; x<width*3; x++ ){
            dest[x+offset+ y*width*3] = source[x + y*width*3];
        }
        if(y>0){
            for(int p=0; p<padding; p++){
                dest[p + offset + y*width*3] = 0;
            }
        }
        offset += padding;
    }
}

float Bilineare(unsigned char * source, unsigned int sx, unsigned int sy, float x, float y, int n_canali) {

	float v1, v2, v3, v4;
	int X = (int) x;
	int Y = (int) y;

	x -= X;
	y -= Y;

	if(X < 0) X = 0;
	if(X >= (sx - 1)) X = sx - 1;
	if(Y < 0) Y = 0;
	if(Y >= sy - 3) Y = sy - 3;

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

#define OFS ((KD - 1) / 2)

int *Kernel;
int Scala = 1;
int KD;
char* Filtri[4] = {"sharpen", "blur", "bordi", "bassorilievo"};

int Sharpen[5 * 5] = {
	 0, 0, 0, 0, 0,
     0, 0,-1, 0, 0,
     0,-1, 5,-1, 0,
     0, 0,-1, 0, 0,
     0, 0, 0, 0, 0
};

int Blur[5 * 5] = {
	 0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0
};

int Bordi[3 * 3] = {
    0, 1, 0,
    1,-4, 1,
    0, 1, 0
};

int BassoRilievo[3 * 3] = {
    -2,-1, 0,
    -1, 1, 1,
     0, 1, 2
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

int Pixel(unsigned char *source,unsigned int sx, unsigned int sy, int x, int y){

	int u, v;
	int a;
	int p = 0;

	for(v = -OFS;v <= OFS;v++) {
		if((y + v) < 0 || (y + v) >= sy) continue;
		for(u = -OFS;u <= OFS;u++) {
			if((x + u)*3 < 0 || (x + u)*3 >= sx*3) continue;

			a = source[(x + u)*3 + ((y + v) * sx*3)];
			p += (a * Kernel[u + OFS + ((v + OFS) * KD)]);
		}
	}

	p /= Scala;

	if(p < 0) p = 0;
	if(p > 255) p = 255;

	return(p);
}

void Convoluzione(unsigned char * source, unsigned int sx, unsigned int sy, unsigned char * dest ) {

	int x, y, c;

    for(c=0;c<3;c++){
        for(y = 0;y < sy;y++) {
            for(x = 0;x < sx;x++) {
                dest[c + x*3 + y*sx*3] = Pixel(source+c, sx, sy, x, y);
            }
        }
    }

}

static void SelezioneFiltro(char * selezione){

    if(strcmp(selezione,Filtri[0]) == 0){
        KD = 5;
        Kernel = Sharpen;
    }else if(strcmp(selezione,Filtri[1]) == 0){
        KD = 5;
        Kernel = Blur;
        Scala = 9;
    }else if(strcmp(selezione,Filtri[2]) == 0){
        KD = 3;
        Kernel = Bordi;
    }else if(strcmp(selezione,Filtri[3]) == 0){
        KD = 3;
        Kernel = BassoRilievo;
    }else{
        printf("Filtro non disponibile");
        exit(1);
    }

}

void ControlloDimensioni(const char *in_dx, unsigned int &out_dx, const char *in_dy, unsigned int &out_dy){

    if(atoi(in_dx) > 0){
        out_dx = atoi(in_dx);
    }else{
        printf("Larghezza non valida\n");
        exit(1);
    }
    if(atoi(in_dy) > 0){
        out_dy = atoi(in_dy);
    }else{
        printf("Altezza non valida\n");
        exit(1);
    }

}

int main(int argc, char *argv[])
{
	unsigned char header[54]; // Header
	unsigned int dim_head_bmp=0; //Dimensione Header
    unsigned int paddingS = 0;
    unsigned int paddingD = 0;
	unsigned int sx=0; //Larghezza Immagine Sorgente
	unsigned int sy=0; //Altezza Immagine Sorgente
    unsigned int dx=0; //Larghezza Immagine Destinazione
    unsigned int dy=0; //Altezza Immagine Destinazione

    printf("\n");
    ControlloDimensioni( argv[3], dx, argv[4], dy);

    unsigned char *ImmagineS = NULL; //Puntatore a Immagine Sorgente
    unsigned char *ImmagineD = new unsigned char[dx*dy*3]; //Creazione spazio per Immagine di Destinazione

	CaricaBmp( argv[1], header, dim_head_bmp, ImmagineS, sx, sy); //Caricamento Immagine sorgente

    printf("Dimensione immagine in input: L %d x H %d\n", sx, sy);
    printf("Dimensione immagine prevista per l'output: L %d x H %d\n", dx, dy);

    unsigned char *ImmagineF = new unsigned char [sx*sy*3];

    SelezioneFiltro( argv[2] );

    printf("Filtro Applicato: %s\n\n", argv[2]);

    unsigned char *ImmagineSNP = NULL;
    if((sx*3)%4 != 0){
      paddingS = 4 - (sx*3)%4;
      ImmagineSNP = new unsigned char [sx*sy*3];
      RimuoviPadding(ImmagineS, sx, sy, paddingS, ImmagineSNP);
      Convoluzione(ImmagineSNP, sx, sy, ImmagineF);
    }else{
      Convoluzione(ImmagineS, sx, sy, ImmagineF);
    }

    unsigned char *ImmagineDP = NULL;
	if((dx*3)%4 != 0){
        paddingD = 4 - ((dx*3)%4);
        ImmagineDP = new unsigned char [(dx*dy*3) + (paddingD*dy)];
	}

    if( (sx != dx) || (sy != dy) ){

        printf("Ridimensionamento...\n");
        Ridimensiona(ImmagineF, sx, sy, ImmagineD, dx, dy); //Ridimensionamento Immagine

        if(paddingD != 0){
            AggiungiPadding(ImmagineD, dx, dy, paddingD, ImmagineDP);
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineDP, dx, dy); //Serializzazione Immagine
         }else{
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineD, dx, dy); //Serializzazione Immagine
         }
        printf("Immagine ridimensionata\n");

    }else if( (sx == dx) && (sy == dy) ){

         if(paddingD != 0){
            AggiungiPadding(ImmagineF, dx, dy, paddingD, ImmagineDP);
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineDP, dx, dy); //Serializzazione Immagine
         }else{
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineF, dx, dy); //Serializzazione Immagine
         }

    }
    printf("Immagine processata\n");

	//Deallocazione memoria
	delete[] ImmagineS;
	delete[] ImmagineD;
	delete[] ImmagineF;
	if (ImmagineSNP){
    delete[] ImmagineSNP;
	}
	if (ImmagineDP){
    delete[] ImmagineDP;
	}

	return 0;

}

