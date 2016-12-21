
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <string.h>

using std::string;

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
            dest[x + y*width*3] = source[x+offset + y*width*3]; // Copia nel buffer di destinazione dei
                                                                // byte significativi
        }
        offset += padding; // Contatore dei byte del padding per ogni riga di pixel
    }
}

void AggiungiPadding(unsigned char * source, int width, int height, int padding, unsigned char * dest){
    int offset = 0;
    for(int y=0; y<height; y++ ){
        for(int x=0; x<width*3; x++ ){
            dest[x+offset+ y*width*3] = source[x + y*width*3]; // Copia del buffer di ingresso
                                                              //in quello di destinazione tenendo conto del padding
        }
        if(y>0){            //Aggiunta del padding partendo dalla riga successiva alla prima
            for(int p=0; p<padding; p++){
                dest[p + offset + y*width*3] = 0;
            }
        }
        offset += padding;
    }
}

float Bilineare(unsigned char * source, unsigned int sx, unsigned int sy, float x, float y, int n_canali) {

	float v1, v2, v3, v4;
	int X = (int) x; // Parte intera di x
	int Y = (int) y; // Parte intera di y

	x -= X; // Parte decimale di x
	y -= Y; // Parte decimale di y

    //Correzione delle coordinate se superano i limiti prestabiliti
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
                dest[c + x*3 +y*dx*3 ] = (char) int_canale; //Scrittura del valore di intensità del canale
                                                            //sull'immagine di destinazione
                }
            }
        }
}

#define OFS ((KD - 1) / 2)

int *Kernel;
int Scala = 1; // Valore del divisore che compone il rapporto che viene moltiplicato per la matrice kernel
int KD; // Dimensione della matrice kernel
char * Filtri[] = {"sharpen", "blur", "bordi", "bassorilievo"};
char * Comandi[] = {"brightness", "gamma"};

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

int Pixel(unsigned char *source,unsigned int sx, unsigned int sy, int x, int y){

	int u, v;
	int a;
	int p = 0;

	for(v = -OFS;v <= OFS;v++) {
		if((y + v) < 0 || (y + v) >= sy) continue; // Salta un iterazione se la condizione si verifica
		for(u = -OFS;u <= OFS;u++) {
			if((x + u)*3 < 0 || (x + u)*3 >= sx*3) continue; // Salta un iterazione se la condizione si verifica
			a = source[(x + u)*3 + ((y + v) * sx*3)]; // Recupera il valore del canale del pixel
			p += (a * Kernel[u + OFS + ((v + OFS) * KD)]); // Il valore ottenuto in precedenza viene
                                                           // moltiplicato per il componente
                                                           // della matrice kernel corrispondente.
                                                           // Esso viene sommato n volte dove n è il numero
                                                           // di componenti letti della matrice kernel


		}
	}

	p /= Scala; // Rapporto tra il valore del canale del pixel ottenuto ed un valore di scala

	if(p < 0) p = 0; // Clamping se il valore risulta minore di 0
	if(p > 255) p = 255; // Clamping se il valore risulta maggiore di 255

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

void Brightness(float valore, unsigned char * source,  unsigned int sx, unsigned int sy, unsigned char * dest){

    for (int c = 0; c<3; c++){
        for(int y = 0;y < sy;y++) {
            for(int x = 0;x < sx;x++) {
                float v = source[x*3 + c + (y * sx*3)];
                v += v * valore; // Aumento/Diminuzione della percentuale di luminosità
                if(v > 255) v = 255;
                if(v < 0) v = 0;
                dest[x*3 + c + (y * sx*3)] = (char) v;
            }
        }
    }
}

void Gamma(float esponente, unsigned char * source, unsigned int sx, unsigned int sy, unsigned char * dest) {

        for(int c = 0;c < 3;c++) {
            for(int y = 0;y < sy;y++) {
                for(int x = 0;x < sx;x++) {
                    float v = source[x*3 + c + (y * sx*3)];
                    v = pow(v / 255.0f, 1.0f/esponente);
                    v *= 255.0f;
                    if(v > 255) v = 255;
                    if(v < 0) v = 0;
                    dest[x*3 + c  + (y * sx*3)] = (char) v;
                }
            }
        }

}

static bool SelezioneFiltro(char * selezione){

    if(strcmp(selezione,Filtri[0]) == 0){
        KD = 5;
        Kernel = Sharpen;
    }else if(strcmp(selezione,Filtri[1]) == 0){
        KD = 5;
        Kernel = Blur;
        Scala = 9; // Questo parametro serve per bilanciare l'effetto blur
    }else if(strcmp(selezione,Filtri[2]) == 0){
        KD = 3;
        Kernel = Bordi;
    }else if(strcmp(selezione,Filtri[3]) == 0){
        KD = 3;
        Kernel = BassoRilievo;
    }else{
        return false;
    }
    return true;
}

void ControlloDimensioni(const char *in_dx, unsigned int &out_dx, const char *in_dy, unsigned int &out_dy){

	if(atoi(in_dx) > 0){
        out_dx = atoi(in_dx); // Conversione da stringa a intero
    }else{
        printf("Larghezza non valida\n");
        exit(1);
    }
    if(atoi(in_dy) > 0){
        out_dy = atoi(in_dy); // Conversione da stringa a intero
    }else{
        printf("Altezza non valida\n");
        exit(1);
    }

}

bool is_number(const std::string s)
{
    std::string::const_iterator it = s.begin() + 1;
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end() && ( isdigit(*(s.begin())) || *(s.begin()) == '-' );
}

bool is_float(const std::string s)
{
    std::string::const_iterator it = s.begin();
    int dot = 0;
    while (it != s.end() && (isdigit(*it) || *it=='.') && dot <= 1 ){
        if( *it == '.'){
        dot++;
        }
        ++it ;
    }
    return !s.empty() && it == s.end() && dot <= 1;
}

void print_help(){
	printf(
	"elaboraimmagine fileinput filtro/comando dimensioni_x dimensioni_y fileoutput\n"
    "\n"  
	"Filtri disponibili\n"
	"\n"
	"sharpen        Filtro che aumenta il contrasto\n"
	"blur           Filtro che applica una sfocatura all'immagine\n"  
	"bordi          Filtro che evidenzia i bordi dell'immagine\n"  
	"bassorilievo   Filtro che genera un bassorilievo dell'immagine\n"
	"\n"
	"Comandi disponibili\n"
	"\n"
	"brightness     Permette di aumentare o di diminuire la luminosità\n"
	"               di una certa percentuale. Essa può essere inserita a runtime\n"
	"gamma          Permette di corregge la gamma dei colori dell'immagine.\n"
	"               Il valore di correzione può essere specificato a runtime\n"
	"\n"
	"Comandi speciali\n"
	"\n"
	"nofilter       Permette di non applicare nessun filtro/comando\n"
	);
}

int main(int argc, char *argv[])
{
    unsigned char header[54]; // Header
    unsigned int dim_head_bmp=0; //Dimensione Header
    unsigned int paddingS = 0; // Padding dell'Immagine Sorgente
    unsigned int paddingD = 0; // Padding dell'Immagine Destinazione
    unsigned int sx=0; //Larghezza Immagine Sorgente
    unsigned int sy=0; //Altezza Immagine Sorgente
    unsigned int dx=0; //Larghezza Immagine Destinazione
    unsigned int dy=0; //Altezza Immagine Destinazione

    printf("\n");

    if(argv[1] == NULL || strcmp(argv[1],"help")==0 || strcmp(argv[1],"h")==0){ // Controllo comando senza parametri
        print_help();
        exit(1);
    }
	
	if(argc > 4)
        ControlloDimensioni( argv[3], dx, argv[4], dy);
	else{
        printf("Dimensioni non inserite\n");
		exit(1);
	}

    unsigned char *ImmagineS = NULL; //Puntatore a Immagine Sorgente
    unsigned char *ImmagineD = new unsigned char[dx*dy*3]; //Creazione spazio per Immagine di Destinazione

    CaricaBmp( argv[1], header, dim_head_bmp, ImmagineS, sx, sy); //Caricamento Immagine sorgente

    printf("Dimensione immagine in input: L %d x H %d\n", sx, sy);
    printf("Dimensione immagine prevista per l'output: L %d x H %d\n", dx, dy);

    unsigned char *ImmagineF = new unsigned char [sx*sy*3]; // Creazione Buffer per Immagine Filtrata

    unsigned char *ImmagineSTE = ImmagineS;
    if((sx*3)%4 != 0){
      paddingS = 4 - (sx*3)%4; // Calcolo del padding per l'immagine sorgente
      unsigned char *ImmagineSNP = new unsigned char [sx*sy*3]; // Buffer per immagine senza padding
      RimuoviPadding(ImmagineS, sx, sy, paddingS, ImmagineSNP); // Rimozione del padding
      ImmagineSTE = ImmagineSNP;
    }

    if ( strcmp(argv[2],"nofilter") != 0){

        if (SelezioneFiltro( argv[2] )){
            printf("Filtro Applicato: %s\n\n", argv[2]);
            Convoluzione(ImmagineSTE, sx, sy, ImmagineF);
        }else if ( strcmp(argv[2], Comandi[0]) == 0 ){
            char val[100];
            do{
                printf("Inserisci il fattore di luminosita (%%): "); // Controllo del valore inserito
                scanf("%s", &val[0]);
            }while( !is_number(val));
            int valore = atoi(val);
            printf("Valore inserito: %d\n", valore);
            float brightness = ((float) valore) / 100.0f;
            Brightness(brightness, ImmagineSTE, sx, sy, ImmagineF);
            printf("Comando utlizzato: %s\n", Comandi[0]);
        }else if ( strcmp(argv[2], Comandi[1]) == 0 ){
            char gam[100];
            do{
                printf("Inserisci il fattore di gamma: ");
                scanf("%s", &gam[0]);
            }while( !is_float(gam) || atof(gam) < 0.0f ); // Controllo del valore inserito
            float gamma = atof(gam);
            printf("Valore inserito: %f\n", gamma);
            Gamma(gamma, ImmagineSTE, sx, sy, ImmagineF);
            printf("Comando utlizzato: %s\n", Comandi[1]);
        }else{
            printf("Comando/Filtro non disponibile\n");
            exit(1);
        }

    }else{
        printf("Nessun Filtro Selezionato\n");
        ImmagineF = ImmagineSTE;
    }

    unsigned char *ImmagineDP = NULL;
	if((dx*3)%4 != 0){
        paddingD = 4 - ((dx*3)%4);  // Calcolo del padding per l'immagine destinazione
        ImmagineDP = new unsigned char [(dx*dy*3) + (paddingD*dy)]; // Buffer per immagine destinazione
                                                                    // con padding
	}

    if( (sx != dx) || (sy != dy) ){ // Controllo che valuta se le dimansioni di ingresso
                                    // sono diverse da quelle di destinazione

        printf("Ridimensionamento...\n");
        Ridimensiona(ImmagineF, sx, sy, ImmagineD, dx, dy); //Ridimensionamento Immagine

        if(paddingD != 0){
            AggiungiPadding(ImmagineD, dx, dy, paddingD, ImmagineDP); // Aggiunta del padding per l'immagine di
                                                                      // destinazione
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineDP, dx, dy); //Serializzazione Immagine
         }else{
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineD, dx, dy); //Serializzazione Immagine
         }
        printf("Immagine ridimensionata\n");

    }else if( (sx == dx) && (sy == dy) ){

         if(paddingD != 0){
            AggiungiPadding(ImmagineF, dx, dy, paddingD, ImmagineDP); // Aggiunta del padding per l'immagine di
                                                                      // destinazione
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineDP, dx, dy); //Serializzazione Immagine
         }else{
            SalvaBmp( argv[5], header, dim_head_bmp, ImmagineF, dx, dy); //Serializzazione Immagine
         }

    }
    printf("Immagine processata\n");

	//Deallocazione memoria
	if(ImmagineSTE){
	 delete [] ImmagineSTE;
	}
	delete[] ImmagineD;
	if(ImmagineSTE != ImmagineF){
	delete[] ImmagineF;
	}
	if (ImmagineDP){
        delete[] ImmagineDP;
	}

	return 0;

}
