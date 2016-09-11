# elaboraimmagine-sm
Progetto didattico per il corso di Sistemi Multimediali tenuto dal prof. Paolo Cattani

Sviluppo di una applicazione, lanciata da linea di comando, in grado di elaborare un’immagine a
colori in formato BMP RGB non compresso (Versione 3, .bmp) e di salvare il risultato,
eventualmente riscalato a nuove dimensioni.  

L’applicazione viene lanciata da linea di comando con la seguente sintassi:  
`>elaboraimmagine <fileinput> <filtro> <dimensioni_x> <dimensioni_y> <fileoutput>`

dove:  
`<fileinput>` e’ il file .bmp di input  
`<filtro>` nome del filtro che si vuole applicare   
`<dimensioni_x>` nuova dimensione X del file di output (eventualmente riscalata)  
`<dimensioni_y>` nuova dimensione Y del file di output (eventualmente riscalata)  
`<fileoutput>` nome del file di output  

##Filtri disponibili
Per il parametro `<filtro>` sono disponibili i seguenti valori per identificare i vari filtri:

`sharpen` Filtro che aumenta il contrasto  
`blur` Filtro che applica una sfocatura all'immagine  
`bordi` Filtro che evidenzia i bordi dell'immagine  
`bassorilievo` Filtro che genera un bassorilievo dell'immagine  
