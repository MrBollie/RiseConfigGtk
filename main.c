#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

enum {
    COL_ID = 0,
    COL_MAIN
};

typedef struct {
    snd_seq_t               *seqp;
    snd_seq_client_info_t   *dest_client_info;
    GtkListStore            *ls_midi_input;
} AppData;

void load_client_list(AppData*);
 
int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkListStore    *ls_cc_left_slider;
    AppData         *app_data = g_slice_new(AppData);

    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "RiseConfig.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, app_data);

    app_data->ls_midi_input = GTK_LIST_STORE(gtk_builder_get_object(builder, 
        "ls_midi_input")
    );

    // Init alsa
    int err = snd_seq_open(&app_data->seqp, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        g_slice_free(AppData, app_data);
        printf("Cannot connect to ALSA");
        return -1;
    }
    snd_seq_set_client_name(app_data->seqp, "RiseConfigGtk");

    int in_port = snd_seq_create_simple_port(
        app_data->seqp, 
        "in", 
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, 
        SND_SEQ_PORT_TYPE_MIDI_GENERIC
    );

    int out_port = snd_seq_create_simple_port(
        app_data->seqp, 
        "out", 
        SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, 
        SND_SEQ_PORT_TYPE_MIDI_GENERIC
    );

    // Load midi client list initially
    load_client_list(app_data);

    // init done

    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();

    g_slice_free(AppData, app_data);
 
    return 0;
}


/**
*
*/
void on_bt_midi_input_refresh_clicked(GtkButton *button, AppData *app_data) {
    load_client_list(app_data);
}


/**
*
*/
void on_cb_midi_input_changed(GtkComboBox *combobox, AppData *app_data) {
    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(combobox, &iter);
    GValue val;
    gtk_tree_model_get_value((GtkTreeModel*)app_data->ls_midi_input, &iter, COL_ID, &val);
    printf( "Selcted: %d\n", g_value_get_int(&val));
}

void on_slider_slide_value_changed(GtkRange *scale, AppData *app_data) {
    double v = gtk_range_get_value(scale);
    printf( "Slide Value: %.2f\n", v);
}


/**
* Fetches the list of midi clients from alsa and fills the combobox
* \param app_data application data store
*/
void load_client_list(AppData *app_data) {
    GtkTreeIter     iter;
    // Clear the list
    gtk_list_store_clear(app_data->ls_midi_input);

    snd_seq_client_info_t *midi_client_info;
    snd_seq_client_info_malloc(&midi_client_info);
    
    int rc = 0;
    while (rc == 0) {
        rc = snd_seq_query_next_client(app_data->seqp, midi_client_info);
        if (rc == 0) {
            int id = snd_seq_client_info_get_client(midi_client_info);
            if (id != snd_seq_client_id(app_data->seqp)) {
                const char* cn = snd_seq_client_info_get_name(midi_client_info);
                gtk_list_store_append(app_data->ls_midi_input, &iter);
                gtk_list_store_set(app_data->ls_midi_input, &iter, COL_ID, id, 
                    COL_MAIN, cn, -1);
            }
        }
    }
    snd_seq_client_info_free(midi_client_info);
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
