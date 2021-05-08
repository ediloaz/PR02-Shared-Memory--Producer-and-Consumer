#include <gtk/gtk.h>
#include <stdbool.h>  
#include <glib/gtypes.h>

/* COMANDO PARA CORRER:
gcc -o gladewin interface.c -lm -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic ; ./gladewin
*/


//Inicialización de los componentes en Interfaz
GtkBuilder *builder;
GtkWidget *window;
GtkWidget *g_lbl_nombreBuffer;
GtkWidget *g_lbl_cantidadProductoresActivos;
GtkWidget *g_lbl_cantidadConsumidoresActivos;
GtkWidget *g_lbl_bitacora;
GtkWidget *g_txt_bitacora;
GtkTextBuffer *g_buffer_bitacora;
GtkWidget *g_lbl_bufferFin;     // Renderizará el tamaño del buffer
GtkWidget *g_lbl_leyendo;       // Etiqueta con el puntero de "Leyendo"
GtkWidget *g_lbl_escribiendo;   // Etiqueta con el puntero de "Escribiendo"
GtkWidget *g_contenedorBuffer;  
// testing
GtkWidget *g_entry_escritura;
GtkWidget *g_entry_lectura;

// Constantes para usar en bitácora
const int bitacora_lineasInicialesEscritas = 0;
const int bitacora_lineasUsadasPorEscritura = 3;

int tamanoTotalBuffer = 9500;       // POR MIENTRAS ES QUEMADO, SE DEBE LEER DE LOS PARÁMETROS

/* 
    FUNCIONES ÚTILES
*/
void PrintInt(int value){
    char stringValue[100];
    sprintf(stringValue, "%d", value);
    printf("%s", stringValue);
}

// Devuelve el ancho de cualquier widget que se le pase como parámetro
int getAnchoWidget(GtkWidget * widget){
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget, alloc);
    printf("widget size is currently %dx%d\n",alloc->width, alloc->height);
    int width = alloc->width;
    g_free(alloc);
    return width;
}



/*
    BUFFER
*/

// Calcula la posición de las etiquetas de "Leyendo" y "Escribiendo".
// Esta función usa las constantes del tamaño del ancho de:
//      1. La ventana
//      2. El label/etiqueta
int _calcularPosicionEtiquetaEnPX(GtkWidget * widget, int indiceActual){
    int anchoTotal = getAnchoWidget(g_contenedorBuffer);
    int pos = (int)(anchoTotal * indiceActual / tamanoTotalBuffer);
    return pos;
}

// Llamar esta función para actualizar (1) interfaz el gráfica del buffer y (2) posición del puntero de la etiqueta LECTURA
void ActualizarIndiceLectura(int indiceLectura){
    // Actualizar el gráfico de la Interfaz
    // TODO

    // Actualizar la etiqueta
    int pos = _calcularPosicionEtiquetaEnPX(g_lbl_leyendo, indiceLectura);
    gtk_widget_set_margin_start(g_lbl_leyendo, pos);
    char stringLabel[40];
    sprintf(stringLabel, "▲ Leyendo (pos: %d)", indiceLectura);
    gtk_label_set_text(GTK_LABEL(g_lbl_leyendo), stringLabel);
}

// Llamar esta función para actualizar (1) interfaz el gráfica del buffer y (2) posición del puntero de la etiqueta ESCRITURA
void ActualizarIndiceEscritura(int indiceEscritura){
    // Actualizar el gráfico de la Interfaz
    // TODO

    // Actualizar la etiqueta
    int pos = _calcularPosicionEtiquetaEnPX(g_lbl_escribiendo, indiceEscritura);
    gtk_widget_set_margin_start(g_lbl_escribiendo, pos);
    char stringLabel[40];
    sprintf(stringLabel, "▼ Escribiendo (pos: %d)", indiceEscritura);
    gtk_label_set_text(GTK_LABEL(g_lbl_escribiendo), stringLabel);
}


void InicializarBuffer(){
    char stringValue[20];   // stringValue Debe ser lo bastante grande
    sprintf(stringValue, "%i", tamanoTotalBuffer);
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
    gint lineaSiguiente = gtk_text_buffer_get_line_count (g_buffer_bitacora)/bitacora_lineasUsadasPorEscritura - bitacora_lineasInicialesEscritas;
    char strLineaSiguiente[6];
    sprintf(strLineaSiguiente, "%d", lineaSiguiente);
    gchar *textoParaEscribir = g_strjoin("", " > [", strLineaSiguiente, "]:\n   ", texto, "\n\n",  NULL);
    gtk_text_buffer_insert_at_cursor(g_buffer_bitacora, textoParaEscribir, -1);
}

void IniciarBitacora(){
    gchar *text = "Bitácora :)  \n\n";
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
}

// Llamar esta función para RENDERIZAR/PINTAR el número de CONSUMIDORES en la pantalla
void RenderizarCantidadConsumidoresActivos(int cantidad)
{
    _RenderizarCantidadProductoresOConsumidoresActivos(cantidad, false);
}



/*
    NOMBRE DEL BUFFER
*/
void DefinirNombreBuffer(){
    char stringValue[20];   // stringValue Debe ser lo bastante grande
    sprintf(stringValue, "Nombre del Buffer");
    gtk_label_set_text(GTK_LABEL(g_lbl_nombreBuffer ), stringValue);
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
    g_lbl_cantidadProductoresActivos = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_cantidadProductoresActivos"));
    g_lbl_cantidadConsumidoresActivos = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_cantidadConsumidoresActivos"));
    g_lbl_bufferFin = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_bufferFin"));
    g_lbl_leyendo = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_leyendo"));
    g_lbl_escribiendo = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_escribiendo"));
    g_txt_bitacora = GTK_WIDGET(gtk_builder_get_object(builder, "txt_bitacora"));
    g_buffer_bitacora = gtk_text_view_get_buffer( GTK_TEXT_VIEW(g_txt_bitacora) );
    g_contenedorBuffer = GTK_WIDGET(gtk_builder_get_object(builder, "contenedorBuffer"));
    g_entry_escritura = GTK_WIDGET(gtk_builder_get_object(builder, "entry_escritura"));
    g_entry_lectura = GTK_WIDGET(gtk_builder_get_object(builder, "entry_lectura"));
    

    // Ejemplo de Renderizar/Pintar en interfaz
    InicializarBuffer();
    DefinirNombreBuffer();
    RenderizarCantidadProductoresActivos(9);
    RenderizarCantidadConsumidoresActivos(180);
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

int main(int argc, char **argv)
{
    IniciarInterfaz(argc, argv);

    return 0;
}