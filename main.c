#include <gtk/gtk.h>

enum {
    COL_ID = 0,
    COL_MAIN
};
 
int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkListStore    *ls_cc_left_slider;
 
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "RiseConfig.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    // Get Tree Models
    ls_cc_left_slider = GTK_LIST_STORE(gtk_builder_get_object(builder, 
        "ls_cc_left_slider")
    );

    GtkTreeIter iter;
    gtk_list_store_append(ls_cc_left_slider, &iter);
    gtk_list_store_set(ls_cc_left_slider, &iter, COL_ID, 123, COL_MAIN, "CC-123", -1);
 
    gtk_list_store_append(ls_cc_left_slider, &iter);
    gtk_list_store_set(ls_cc_left_slider, &iter, COL_ID, 124, COL_MAIN, "CC-124", -1);
 
    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();
 
    return 0;
}
 
// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
