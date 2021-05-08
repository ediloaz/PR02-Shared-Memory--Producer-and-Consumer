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

// Constantes para usar en bitácora
const int bitacora_lineasInicialesEscritas = 0;
const int bitacora_lineasUsadasPorEscritura = 3;

int tamanoTotalBuffer = 9500;

void EscribirBuffer(){
    printf("escribo");
    // GtkAlign prueba = gtk_widget_get_halign (g_lbl_leyendo);
    // // printf(prueba);
    // // double asd = prueba;
    // char stringValue[10];   // stringValue Debe ser lo bastante grande
    // sprintf(stringValue, "%d", prueba);
    // gtk_label_set_text(GTK_LABEL(g_lbl_leyendo), stringValue);
    // gtk_widget_set_halign(GTK_LABEL(g_lbl_leyendo), 0.4);
}

void LeerBuffer(){
    printf("leo");
    // GtkAlign prueba = gtk_widget_get_halign (g_lbl_leyendo);
    // // gint marginLeft = gtk_widget_get_margin_left (g_lbl_leyendo);
    // float marginLeft = 123;
    // char stringValue[100];   // stringValue Debe ser lo bastante grande
    // sprintf(stringValue, "%d", marginLeft);
    // gtk_label_set_text(GTK_LABEL(g_lbl_leyendo), stringValue);
}

void InicializarBuffer(){
    char stringValue[20];   // stringValue Debe ser lo bastante grande
    sprintf(stringValue, "%i", tamanoTotalBuffer);
    gtk_label_set_text(GTK_LABEL(g_lbl_bufferFin), stringValue);
}


void EscribirEnBitacora(char *texto){
    gint lineaSiguiente = gtk_text_buffer_get_line_count (g_buffer_bitacora)/bitacora_lineasUsadasPorEscritura - bitacora_lineasInicialesEscritas;
    char strLineaSiguiente[6];
    sprintf(strLineaSiguiente, "%d", lineaSiguiente);
    gchar *textoParaEscribir = g_strjoin("", " > [", strLineaSiguiente, "]:\n   ", texto, "\n\n",  NULL);
    gtk_text_buffer_insert_at_cursor(g_buffer_bitacora, textoParaEscribir, -1);
}

void TestearEscribirEnBitacora(){
    EscribirEnBitacora("Ésto es un ejemplo del mensaje a imprimir en la bitácora");
}

void IniciarBitacora(){
    gchar *text = "Bitácora :)  \n\n";
    gtk_text_buffer_set_text (g_buffer_bitacora, text, -1);
}

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