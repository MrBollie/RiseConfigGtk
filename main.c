#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

enum {
    COL_ID = 0,
    COL_MAIN
};

typedef struct {
    snd_seq_t       *seqp;
    GtkListStore    *ls_midi_input;
} AppData;
 
int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkListStore    *ls_cc_left_slider;
    AppData         *app_data = g_slice_new(AppData);
    snd_seq_client_info_t *midi_client_info;
    GtkTreeIter     iter;

    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "RiseConfig.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, app_data);

    // Init alsa
    int err = snd_seq_open(&app_data->seqp, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        g_slice_free(AppData, app_data);
        printf("Cannot connect to ALSA");
        return -1;
    }

    // Get a list of MIDI clients
    app_data->ls_midi_input = GTK_LIST_STORE(gtk_builder_get_object(builder, 
        "ls_midi_input")
    );

    int rc = 0;
    snd_seq_client_info_malloc(&midi_client_info);
    
    while (rc == 0) {
        rc = snd_seq_query_next_client(app_data->seqp, midi_client_info);
        if (rc == 0) {
            const char* cn = snd_seq_client_info_get_name(midi_client_info);
            gtk_list_store_append(app_data->ls_midi_input, &iter);
            gtk_list_store_set(app_data->ls_midi_input, &iter, 0, cn, -1);
        }
    }

    // init done

    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();

    g_slice_free(AppData, app_data);
    snd_seq_client_info_free(midi_client_info);
 
    return 0;
}

void on_cb_midi_input_changed(GtkComboBox *combobox, AppData *app_data) {
    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(combobox, &iter);
    GValue val;
    gtk_tree_model_get_value((GtkTreeModel*)app_data->ls_midi_input, &iter, 0, &val);
    printf( "Selcted: %s\n", g_value_get_string(&val));
}
 
// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
