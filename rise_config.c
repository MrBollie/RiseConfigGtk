#include "rise_config.h"

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
                const char* cn = 
                    snd_seq_client_info_get_name(midi_client_info);
                gtk_list_store_append(app_data->ls_midi_input, &iter);
                gtk_list_store_set(app_data->ls_midi_input, &iter, COL_ID,
                    id, COL_MAIN, cn, -1);
            }
        }
    }
    snd_seq_client_info_free(midi_client_info);
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
    gtk_tree_model_get_value((GtkTreeModel*)app_data->ls_midi_input, &iter, 
        COL_ID, &val);
        
    if (app_data->is_connected) {
        snd_seq_disconnect_to(app_data->seqp, app_data->local_out.port, 
            app_data->remote_in.client, app_data->remote_in.port
        );
        
        snd_seq_disconnect_from(app_data->seqp, app_data->local_in.port, 
            app_data->remote_out.client, app_data->remote_out.port
        );
        app_data->is_connected = false;
    }
    
    app_data->remote_in.client = g_value_get_int(&val);
    app_data->remote_in.port = 0;
    snd_seq_connect_to(app_data->seqp, app_data->local_out.port, 
        app_data->remote_in.client, app_data->remote_in.port);
    
    app_data->remote_out.client = g_value_get_int(&val);
    app_data->remote_out.port = 0;    
    snd_seq_connect_from(app_data->seqp, app_data->local_in.port, 
        app_data->remote_out.client, app_data->remote_out.port);
        
    app_data->is_connected = true;
    GtkTextIter bufiter;
    gtk_text_buffer_get_end_iter(app_data->event_buffer, &bufiter);
    gtk_text_buffer_insert(app_data->event_buffer, &bufiter,
        "Port connected\n\0", -1);

    send_sysex_init(app_data);
}


/**
* Slide slider has been slided
*/
void on_slider_slide_value_changed(GtkRange *scale, AppData *app_data) {
    double v = gtk_range_get_value(scale);
    printf( "Slide Value: %.2f\n", v);
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, app_data->local_out.port);
    //snd_seq_ev_set_dest(&ev, app_data->remote_in.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    char data[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3d, 0x18, v, 0xf7 };

    snd_seq_ev_set_sysex(&ev, 9, data);

    snd_seq_event_output(app_data->seqp, &ev);
    snd_seq_drain_output(app_data->seqp);
}


/**
* Timeout callback, that will be called by the main loop every n ms.
* \data the app data
*/ 
gint on_timeout(gpointer data) {
    snd_seq_event_t *ev;
    while (snd_seq_event_input(((AppData*)data)->seqp, &ev) >= 0) {
        process_midi((AppData*)data, ev);
        snd_seq_free_event(ev);
    }
    return true;
}


/**
* called when window is closed
*/
void on_window_main_destroy()
{
    gtk_main_quit();
}


/**
* Processes incoming midi
* \data the app data
*/ 
void process_midi(AppData* app_data, snd_seq_event_t* ev) {
        printf("MIDI event %d\n", ev->type);
        if (ev->type == SND_SEQ_EVENT_SYSEX) {
            printf("Sysex received\n");
            for (int i = 0 ; i < ev->data.ext.len ; ++i) {
                printf("0x%02x ", ((char*)ev->data.ext.ptr)[i]);
            }
            printf("\n");
        }
        else if (snd_seq_ev_is_note_type(ev)) 
            printf("Note!\n");
}


/**
* Send sysex
*/
void send_sysex_init(AppData *app_data) {
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, app_data->local_out.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    char data[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3f, 0xf7 };

    snd_seq_ev_set_sysex(&ev, 7, data);

    snd_seq_event_output(app_data->seqp, &ev);
    snd_seq_drain_output(app_data->seqp);
}


/**
* main function
*/
int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkListStore    *ls_cc_left_slider;
    AppData         *app_data = g_slice_new(AppData);

    gtk_init(&argc, &argv);
    
    // new builder
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "RiseConfig.glade", NULL);
 
    // Get various builder objects
    window = GTK_WIDGET(
        gtk_builder_get_object(builder, "window_main"));

    app_data->ls_midi_input = GTK_LIST_STORE(
        gtk_builder_get_object(builder, "ls_midi_input"));

    app_data->event_buffer = GTK_TEXT_BUFFER(
        gtk_builder_get_object(builder, "event_buffer"));

    app_data->slider_glide = GTK_RANGE(
        gtk_builder_get_object(builder, "slider_glide"));

    app_data->slider_lift = GTK_RANGE(
        gtk_builder_get_object(builder, "slider_lift"));

    app_data->slider_press = GTK_RANGE(
        gtk_builder_get_object(builder, "slider_press"));

    app_data->slider_slide = GTK_RANGE(
        gtk_builder_get_object(builder, "slider_slide"));

    app_data->slider_strike = GTK_RANGE(
        gtk_builder_get_object(builder, "slider_strike"));

    gtk_widget_show(window);                

    gtk_builder_connect_signals(builder, app_data);

    // Init alsa
    g_debug("Init ALSA");
    int err = snd_seq_open(&app_data->seqp, "default", 
        SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    if (err < 0) {
        g_slice_free(AppData, app_data);
        printf("Cannot connect to ALSA");
        return -1;
    }
    snd_seq_set_client_name(app_data->seqp, "RiseConfigGtk");

    g_debug("Creating simple ports");
    int in = snd_seq_create_simple_port(
        app_data->seqp, 
        "in", 
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, 
        SND_SEQ_PORT_TYPE_MIDI_GENERIC
    );

    int out = snd_seq_create_simple_port(
        app_data->seqp, 
        "out", 
        SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, 
        SND_SEQ_PORT_TYPE_MIDI_GENERIC
    );
    app_data->is_connected = false;
    
    // Prepare sender 
    app_data->local_out.client = snd_seq_client_id(app_data->seqp);
    app_data->local_out.port = out;
    
    // Prepare receiver
    app_data->local_in.client = snd_seq_client_id(app_data->seqp);
    app_data->local_in.port = in;

    // Load midi client list initially
    load_client_list(app_data);
    
    app_data->queue_in = snd_seq_alloc_named_queue(app_data->seqp, 
        "RiseConfigGtk_in");
    app_data->queue_out = snd_seq_alloc_named_queue(app_data->seqp, 
        "RiseConfigGtk_out");

    // will be called every ms
    g_timeout_add(1, on_timeout, app_data);

    // init done
    gtk_adjustment_set_value(slider_slide, 10);

    g_object_unref(builder);
 
    g_debug("Entering loop");
    gtk_main();

    g_slice_free(AppData, app_data);
 
    return 0;
}

