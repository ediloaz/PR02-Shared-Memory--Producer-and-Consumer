#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <stdbool.h>  
#include <glib/gtypes.h>

#define NAMEMAX 100 		//tamaño máximo del numbre del buffer
#define LOGMAX 164
#define ENTRYMAX 164
#define AUX "\auxiliar"

/*

-> Para compilar con la interfaz:
gcc CreadorDummy.c -o creador -lpthread -lrt -lm -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic 

*/


// Para usar la interfaz
const bool USAR_INTERFAZ = true;


// Funciones de interfaz llamadas antes de su creación
void RefrescarInterfaz();
void EscribirEnBitacora();
void RenderizarCantidadConsumidoresActivos();
void RenderizarCantidadProductoresActivos();
void ActualizarIndices();
void IniciarInterfaz();
void MensajeInicialBitacora();


//LECTURA DE PARÁMETROS
char nombreBuffer[NAMEMAX] = "/nombreBuffer";
int buff_size = 20;
int paramIndex = 1;


struct auxiliar_t{
    int index_lectura;		//Índice de lectura
    int index_escritura;	//Índice de escritura
    int max_buffer;		//Tamaño máximo de capacidad del buffer
    int flag_productor; //Flag del productor

    sem_t SEM_CONSUMIDORES; 	//semáforo de total de consumidores vivos
    sem_t SEM_PRODUCTORES; 	//semáforo de total de productores vivos
    sem_t SEM_FINALIZADOR;  //semáforo del finalizador

    sem_t SEM_LLENO;	     	//semáforo de buffer lleno
    sem_t SEM_VACIO;		//semáforo de buffer vacío
    sem_t SEM_BUFFER;		//semáforo de acceso a buffer
    sem_t SEM_BITACORA; //semáforo de acceso a la bitácora

    int PRODUCTORES;		//total de productores vivos
    int CONSUMIDORES;		//total de consumidores vivos

    char mensaje_log[LOGMAX];
} ;


struct auxiliar_t* auxptr;

void sig_handler(int signum){
    printf("\n sig_handler \n "); 
    if(signum == SIGUSR1){
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("Recibí la señal de creacion de consumidor. Ahora hay: %d vivos\n",auxptr->CONSUMIDORES );
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) RenderizarCantidadConsumidoresActivos(auxptr->CONSUMIDORES);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("LOG: %s\n", auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) EscribirEnBitacora(auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        sem_post(&auxptr->SEM_BITACORA);
        printf("\n\t> Línea %d \n ",  __LINE__); 
    }
}

void sig_handler_P(int signum){
    printf("\n sig_handler_P \n "); 
    if(signum == SIGUSR2){
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("Recibí la señal de creacion de productor. Ahora hay: %d vivos\n",auxptr->PRODUCTORES);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) RenderizarCantidadProductoresActivos(auxptr->PRODUCTORES);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("LOG: %s\n", auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) EscribirEnBitacora(auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        sem_post(&auxptr->SEM_BITACORA);
        printf("\n\t> Línea %d \n ",  __LINE__); 
    }
}

void sig_handlerLog(int signum){
    printf("\n sig_handlerLog \n "); 

    printf("\n\t> Línea %d \n ",  __LINE__); 
    printf("LOG: %s\n", auxptr->mensaje_log);
    printf("\n\t> Línea %d \n ",  __LINE__); 
    if (USAR_INTERFAZ) EscribirEnBitacora(auxptr->mensaje_log);
    printf("\n\t> Línea %d \n ",  __LINE__); 
    sem_post(&auxptr->SEM_BITACORA);
    printf("\n\t> Línea %d \n ",  __LINE__); 
}


void sig_handlerBuff(int signum){
    printf("\n sig_handlerBuff \n");
    if(signum == SIGALRM){
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("Buffer leido: INDEX_LECTURA: %d, INDEX_ESCRITURA: %d",auxptr->index_lectura, auxptr->index_escritura );
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) ActualizarIndices();
        printf("\n\t> Línea %d \n ",  __LINE__); 
        printf("LOG: %s\n", auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        if (USAR_INTERFAZ) EscribirEnBitacora(auxptr->mensaje_log);
        printf("\n\t> Línea %d \n ",  __LINE__); 
        sem_post(&auxptr->SEM_BITACORA);
        printf("\n\t> Línea %d \n ",  __LINE__); 
    }
}



void Algoritmo(){

    pid_t pid = getpid();
    printf("Mi PID es: %d\n", pid);
    if (USAR_INTERFAZ) MensajeInicialBitacora();

    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler_P);
    signal(SIGVTALRM, sig_handlerLog);
    signal(SIGALRM, sig_handlerBuff);


    struct buffer_t{
         char b[buff_size][ENTRYMAX];
    };

    struct buffer_t buffer;

    char (*bufptr)[ENTRYMAX];


    //DECLARA MEMORIA COMPARTIDA
    int len = sizeof(struct auxiliar_t);
    int fd = shm_open(AUX, O_RDWR | O_CREAT, 0666);
    int fd2 = shm_open(nombreBuffer, O_RDWR | O_CREAT, 0666);
    if(fd < 0 || fd2 < 0){
        printf("Error de shm_open\n");
        exit(1);
    }
    if(ftruncate(fd, len) == -1 || ftruncate(fd2, ENTRYMAX*buff_size) == -1) {
    	printf("Error de ftruncate\n");
        exit(1);
    }
    printf("Largo: %d\n", len);
    auxptr = mmap(0,len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    bufptr = mmap(0,ENTRYMAX*buff_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd2,0);

    if(auxptr == MAP_FAILED){
    	printf("Error al crear la memoria compartida (mmap)\n");
        exit(1);
    }


    //INICIALIZA VALROES DE STRUCT

    sem_init(&auxptr->SEM_CONSUMIDORES, 1, 1);
    sem_init(&auxptr->SEM_PRODUCTORES, 1, 1);
    sem_init(&auxptr->SEM_FINALIZADOR, 1, 0);
    sem_init(&auxptr->SEM_BITACORA, 1, 1);


    sem_init(&auxptr->SEM_LLENO, 1, 0);
    sem_init(&auxptr->SEM_VACIO, 1, buff_size-1);

    sem_init(&auxptr->SEM_BUFFER, 1, 1);

    auxptr->index_lectura = auxptr->index_escritura = 0;
    auxptr->PRODUCTORES = auxptr->CONSUMIDORES = 0;
    auxptr->max_buffer = buff_size;
    auxptr->flag_productor = 1;


    //printf("Largo: %ld\n", sizeof(bufptr));
    printf("Memoria compartida creada correctamente\n");
    //Ciclo infinito para mantenerlo vivo

    if (!USAR_INTERFAZ){
        for(int i=1;;i++){
        sleep(1);
        }
    }
}


int main(int argc, char** argv){
    printf("\n\t> Línea %d \n ",  __LINE__);
    // Lee los parámetros y guarda en variables globales
    while(paramIndex < argc)
    {
    	char* param = argv[paramIndex];
    	paramIndex++;

    	if(param[0] == '-'){
    	    switch(param[1]){
    	    	case 'n':
    	    	strcpy(nombreBuffer, argv[paramIndex]);
    	    	printf("Nombre del buffer: %s\n", nombreBuffer);
    	    	paramIndex++;
    	    	break;
    	    case 'm':
    	    	buff_size = atoi(argv[paramIndex]);
    	    	printf("Tamaño del buffer: %d\n", buff_size);
    	    	paramIndex++;
    	    	break;
    	    default:
    	    	printf("Error: Parámetro no reconocido\n");
    	    	return 1;
    	    }
    	}
    }
    printf("\n\t> Línea %d \n ",  __LINE__);

    // La Interfaz tiene un botón para llamar a la función Algoritmo()
    USAR_INTERFAZ ? IniciarInterfaz(argc, argv) : Algoritmo();

    return 0;
}












/*


██╗  ███╗░░██╗  ████████╗  ███████╗  ██████╗░  ███████╗  ░█████╗░  ███████╗
██║  ████╗░██║  ╚══██╔══╝  ██╔════╝  ██╔══██╗  ██╔════╝  ██╔══██╗  ╚════██║
██║  ██╔██╗██║  ░░░██║░░░  █████╗░░  ██████╔╝  █████╗░░  ███████║  ░░███╔═╝
██║  ██║╚████║  ░░░██║░░░  ██╔══╝░░  ██╔══██╗  ██╔══╝░░  ██╔══██║  ██╔══╝░░
██║  ██║░╚███║  ░░░██║░░░  ███████╗  ██║░░██║  ██║░░░░░  ██║░░██║  ███████╗
╚═╝  ╚═╝░░╚══╝  ░░░╚═╝░░░  ╚══════╝  ╚═╝░░╚═╝  ╚═╝░░░░░  ╚═╝░░╚═╝  ╚══════╝

Para generar textos grandes: https://fsymbols.com/generators/tarty/

*/


//Inicialización de los componentes en Interfaz
GtkBuilder *builder;
GtkWidget *window;
GtkWidget *g_lbl_nombreBuffer;
GtkWidget *g_lbl_cantidadProductoresActivos;
GtkWidget *g_lbl_cantidadConsumidoresActivos;
GtkWidget *g_txt_bitacora;
GtkTextBuffer *g_buffer_bitacora;
GtkWidget *g_lbl_bufferFin;     // Renderizará el tamaño del buffer
GtkWidget *g_lbl_leyendo;       // Etiqueta con el puntero de "Leyendo"
GtkWidget *g_lbl_escribiendo;   // Etiqueta con el puntero de "Escribiendo"
GtkWidget *g_contenedorBuffer;
GtkWidget *g_progressBarLector1;
GtkWidget *g_progressBarLector2;
GtkWidget *g_progressBarEscritor1;
GtkWidget *g_progressBarEscritor2;
GtkWidget *g_lbl_pid;
// testing
GtkWidget *g_entry_escritura;
GtkWidget *g_entry_lectura;

// Constantes para usar en bitácora
const int bitacora_lineasInicialesEscritas = 0;
const int bitacora_lineasUsadasPorEscritura = 3;
const int buffer_porcentajeParaCambiarEtiqueta = 30;

// int buff_size = 9500;       // POR MIENTRAS ES QUEMADO, SE DEBE LEER DE LOS PARÁMETROS





/* 
    FUNCIONES ÚTILES
*/
void PrintInt(int value){
    char stringValue[100];
    sprintf(stringValue, "%d", value);
    printf("%s", stringValue);
}
void PrintFloat(float value){
    char stringValue[100];
    sprintf(stringValue, "%f", value);
    printf("%s", stringValue);
}

// Devuelve el ancho de cualquier widget que se le pase como parámetro
int getAnchoWidget(GtkWidget * widget){
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget, alloc);
    int width = alloc->width;
    g_free(alloc);
    return width;
}




/*
    BUFFER
*/

bool _cambioEtiquetaDeLado(GtkWidget * widget, int indiceActual){
    if ((int)(indiceActual*100/buff_size) > buffer_porcentajeParaCambiarEtiqueta){
        return true;
    }else{
        return false;
    }
}

// Calcula la cantidad de llenado del buffer, es un porcentaje del 0.0 al 1.0
float _calcularLlenadoBufferGraficoEnPorcentaje(int indiceActual){
    printf("\n indiceActual: %d", indiceActual);
    printf("\n buff_size: %d", buff_size);
    if (indiceActual==0){
        indiceActual = 1;
        printf("\n indiceActual modif: %d", indiceActual);
    }else{
        printf("\n all good");
    }
    int porcentaje = 100 * indiceActual / buff_size;
    
    printf("\n porcentaje: "); 
    PrintInt(porcentaje);
    
    return 0.5;
}
// Calcula la posición de las etiquetas de "Leyendo" y "Escribiendo".
int _calcularPosicionEtiquetaEnPX(GtkWidget * widget, int indiceActual){
    // int anchoTotal = getAnchoWidget(g_contenedorBuffer);
    // int pos = (int)(anchoTotal * indiceActual / buff_size);
    // if (_cambioEtiquetaDeLado(widget, indiceActual)){
    //     pos = pos - getAnchoWidget(widget);
    // }
    // return pos;
    return indiceActual;
}

void _ActualizarBuffersGraficosAuxiliares(){
    float porcentajeLector1 = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(g_progressBarLector1));
    float porcentajeEscritor1 = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(g_progressBarEscritor1));

    if (porcentajeLector1 > porcentajeEscritor1){
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progressBarLector1), 0.0);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progressBarLector2), porcentajeLector1);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progressBarEscritor2), 1.0);
    }
}

// Llamar esta función para actualizar (1) interfaz el gráfica del buffer y (2) posición del puntero de la etiqueta LECTURA
void ActualizarIndiceLectura(int indiceLectura){
    // Actualizar el gráfico de la Interfaz
    printf("__a");
    float porcentaje = _calcularLlenadoBufferGraficoEnPorcentaje(indiceLectura);
    printf("b");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progressBarLector1), porcentaje);
    printf("c");
    _ActualizarBuffersGraficosAuxiliares();
    printf("d");

    // Actualizar la etiqueta
    int pos = _calcularPosicionEtiquetaEnPX(g_lbl_leyendo, indiceLectura);
    printf("e");
    gtk_widget_set_margin_start(g_lbl_leyendo, pos);
    printf("f");
    char stringLabel[40];
    printf("g");
    sprintf(stringLabel, _cambioEtiquetaDeLado(g_lbl_leyendo, indiceLectura) ? " Leyendo (pos: %d) ▲" : "▲ Leyendo (pos: %d)", indiceLectura);
    printf("h");
    gtk_label_set_text(GTK_LABEL(g_lbl_leyendo), stringLabel);
    printf("i__");
}

// Llamar esta función para actualizar (1) interfaz el gráfica del buffer y (2) posición del puntero de la etiqueta ESCRITURA
void ActualizarIndiceEscritura(int indiceEscritura){
    printf("a");
    // Actualizar el gráfico de la Interfaz
    float porcentaje = _calcularLlenadoBufferGraficoEnPorcentaje(indiceEscritura);
    printf("b");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progressBarEscritor1), porcentaje);
    printf("c");
    _ActualizarBuffersGraficosAuxiliares();
    printf("d");

    // Actualizar la etiqueta
    int pos = _calcularPosicionEtiquetaEnPX(g_lbl_escribiendo, indiceEscritura);
    printf("e");
    gtk_widget_set_margin_start(g_lbl_escribiendo, pos);
    printf("f");
    char stringLabel[40];
    printf("g");
    sprintf(stringLabel, _cambioEtiquetaDeLado(g_lbl_escribiendo, indiceEscritura) ? " Escribiendo (pos: %d) ▼" : "▼ Escribiendo (pos: %d)", indiceEscritura);
    printf("h");
    gtk_label_set_text(GTK_LABEL(g_lbl_escribiendo), stringLabel);
    printf("i");
}

void ActualizarIndices(){
    printf("00");
    int indiceEscritura = auxptr->index_escritura;
    int indiceLectura = auxptr->index_lectura;
    printf("01");
    ActualizarIndiceEscritura(indiceEscritura);
    printf("02");
    ActualizarIndiceLectura(indiceLectura);
    printf("03");
    // RefrescarInterfaz();
    printf("04");
}


void InicializarBuffer(){
    char stringValue[20];   // stringValue Debe ser lo bastante grande
    sprintf(stringValue, "%i", buff_size);
    gtk_label_set_text(GTK_LABEL(g_lbl_bufferFin), stringValue);
}

void TestearEscribirBuffer(){
    // temporal, solo para testear
    const gchar * indiceString = gtk_entry_get_text(GTK_ENTRY(g_entry_escritura));
    int indice = atoi(indiceString);

    // así se debería llamar
    ActualizarIndiceEscritura(indice);
}

void TestearLeerBuffer(){
    // temporal, solo para testear
    const gchar * indiceString = gtk_entry_get_text(GTK_ENTRY(g_entry_lectura));
    int indice = atoi(indiceString);

    // así se debería llamar
    ActualizarIndiceLectura(indice);
}




/*
    BITÁCORA
*/
void EscribirEnBitacora(char *texto){
    printf("\n 1");
    // RefrescarInterfaz();
    // printf("\n 2");
    // gint lineaSiguiente = gtk_text_buffer_get_line_count (g_buffer_bitacora)/bitacora_lineasUsadasPorEscritura - bitacora_lineasInicialesEscritas;
    // printf("\n 3");
    // char strLineaSiguiente[6];
    // printf("\n 4");
    // sprintf(strLineaSiguiente, "%d", lineaSiguiente);
    // printf("\n 5");
    // gchar *textoParaEscribir = g_strjoin("", " > [", strLineaSiguiente, "]:\n   ", texto, "\n\n",  NULL);
    // printf("\n 6");
    // gtk_text_buffer_insert_at_cursor(g_buffer_bitacora, textoParaEscribir, -1);
    // printf("\n 7");
    // RefrescarInterfaz();
    printf("\n 8");
}

void MensajeInicialBitacora(){
    char stringValue[100];
    sprintf(stringValue, "Mensaje inicial de Bitácora: El PID es  %d \n", getpid());
    if (USAR_INTERFAZ) EscribirEnBitacora(stringValue);
}

void IniciarBitacora(){
    gchar *text = "\n";
    gtk_text_buffer_set_text (g_buffer_bitacora, text, -1);
}

void TestearEscribirEnBitacora(){
    EscribirEnBitacora("Ésto es un ejemplo del mensaje a imprimir en la bitácora");
}


/*
    CANTIDAD DE PRODUCTORES Y CONSUMIDORES
*/
// Función auxiliar
void _RenderizarCantidadProductoresOConsumidoresActivos(int cantidad, bool esProductor){
    char stringValue[200];   // stringValue Debe ser lo bastante grande
    sprintf(stringValue, "%i", cantidad);
    gtk_label_set_text(GTK_LABEL(esProductor ? g_lbl_cantidadProductoresActivos : g_lbl_cantidadConsumidoresActivos ), stringValue);
}

// Llamar esta función para RENDERIZAR/PINTAR el número de PRODUCTORES en la pantalla
void RenderizarCantidadProductoresActivos(int cantidad)
{
    _RenderizarCantidadProductoresOConsumidoresActivos(cantidad, true);
    RefrescarInterfaz();
}

// Llamar esta función para RENDERIZAR/PINTAR el número de CONSUMIDORES en la pantalla
void RenderizarCantidadConsumidoresActivos(int cantidad)
{
    _RenderizarCantidadProductoresOConsumidoresActivos(cantidad, false);
    RefrescarInterfaz();
}



/*
    NOMBRE DEL BUFFER
*/
void DefinirNombreBuffer(){
    gtk_label_set_text(GTK_LABEL(g_lbl_nombreBuffer ), nombreBuffer);
    char stringValue[20];
    sprintf(stringValue, "PID: %d", getpid());
    gtk_label_set_text(GTK_LABEL(g_lbl_pid ), stringValue);
}


// Revisa si algún evento está pendiente de actualizar y lo actualiza.
// Éste se usa para actualizar cambios en el UI e invocar timeouts en interfaz.
// Mientras o luego de hacer algún cambio de la interfaz (como el set_text).
void RefrescarInterfaz(){
    // while (gtk_events_pending ())
    // g_main_context_pending(NULL) and g_main_context_iteration(NULL,FALSE)
    while ( g_main_context_pending(NULL) )
        gtk_main_iteration ();
}

// Creación de la interfaz
void IniciarInterfaz(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "interface.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "interface"));
    gtk_builder_connect_signals(builder, NULL);

    // Estilos
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Instancia de los componentes en interfaz que se ocupan manejar con código
    g_lbl_nombreBuffer = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_nombreBuffer"));
    g_lbl_pid = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_pid"));
    g_lbl_cantidadProductoresActivos = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_cantidadProductoresActivos"));
    g_lbl_cantidadConsumidoresActivos = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_cantidadConsumidoresActivos"));
    g_lbl_bufferFin = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_bufferFin"));
    g_lbl_leyendo = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_leyendo"));
    g_lbl_escribiendo = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_escribiendo"));
    g_txt_bitacora = GTK_WIDGET(gtk_builder_get_object(builder, "txt_bitacora"));
    g_buffer_bitacora = gtk_text_view_get_buffer( GTK_TEXT_VIEW(g_txt_bitacora) );
    g_contenedorBuffer = GTK_WIDGET(gtk_builder_get_object(builder, "contenedorBufferEscritor1"));
    g_progressBarLector1 = GTK_WIDGET(gtk_builder_get_object(builder, "progressBarLector1"));
    g_progressBarLector2 = GTK_WIDGET(gtk_builder_get_object(builder, "progressBarLector2"));
    g_progressBarEscritor1 = GTK_WIDGET(gtk_builder_get_object(builder, "progressBarEscritor1"));
    g_progressBarEscritor2 = GTK_WIDGET(gtk_builder_get_object(builder, "progressBarEscritor2"));
    g_entry_escritura = GTK_WIDGET(gtk_builder_get_object(builder, "entry_escritura"));
    g_entry_lectura = GTK_WIDGET(gtk_builder_get_object(builder, "entry_lectura"));
    

    // Ejemplo de Renderizar/Pintar en interfaz
    InicializarBuffer();
    DefinirNombreBuffer();
    RenderizarCantidadProductoresActivos(0);
    RenderizarCantidadConsumidoresActivos(0);
    IniciarBitacora();
    

    g_object_unref(builder);
    gtk_widget_show(window);
    gtk_main();
}


// Se llama cuando la interfaz es cerrada
void on_window_main_destroy()
{
    gtk_main_quit();
    exit(0);
}

















