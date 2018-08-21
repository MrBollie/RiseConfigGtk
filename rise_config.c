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
* Logs data into the little textfield at the bottom of the window
*/
void winlog(AppData* app_data, char *buf, int8_t len) {
    GtkTextIter bufiter;
    gtk_text_buffer_get_end_iter(app_data->event_buffer, &bufiter);
    gtk_text_buffer_insert(app_data->event_buffer, &bufiter, buf, len);
}


/**
* Refreshes the client list
*/
void on_bt_midi_input_refresh_clicked(GtkButton *button, AppData *app_data) {
    load_client_list(app_data);
}


/**
* Activates MPE
*/
void on_bt_mpe_toggled(GtkToggleButton *button, AppData *app_data) {
    send_sysex_mpe_state(app_data, gtk_toggle_button_get_active(button));
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
        
    // Disconnect if it was connected before
    if (app_data->is_connected) {

        // Output
        snd_seq_unsubscribe_port(app_data->seqp, app_data->subs_out);
        snd_seq_free_queue(app_data->seqp, app_data->queue_out);

        // Input
        snd_seq_unsubscribe_port(app_data->seqp, app_data->subs_in);
        snd_seq_free_queue(app_data->seqp, app_data->queue_in);

        app_data->is_connected = false;
    }
    
    // Output
    app_data->queue_out = snd_seq_alloc_named_queue(app_data->seqp, 
        "RiseConfigGtk_out");
    app_data->remote_in.client = g_value_get_int(&val);
    app_data->remote_in.port = 0;
    snd_seq_port_subscribe_alloca(&app_data->subs_out);
    snd_seq_port_subscribe_set_sender(app_data->subs_out, &app_data->local_out);
    snd_seq_port_subscribe_set_dest(app_data->subs_out, &app_data->remote_in);
    snd_seq_port_subscribe_set_time_update(app_data->subs_out, 1);
    snd_seq_port_subscribe_set_queue(app_data->subs_out, app_data->queue_out);
    snd_seq_subscribe_port(app_data->seqp, app_data->subs_out);
    
    // Input
    app_data->queue_in = snd_seq_alloc_named_queue(app_data->seqp, 
        "RiseConfigGtk_in");
    app_data->remote_out.client = g_value_get_int(&val);
    app_data->remote_out.port = 0;    
    snd_seq_port_subscribe_alloca(&app_data->subs_in);
    snd_seq_port_subscribe_set_sender(app_data->subs_in, &app_data->remote_out);
    snd_seq_port_subscribe_set_dest(app_data->subs_in, &app_data->local_in);
    snd_seq_port_subscribe_set_time_update(app_data->subs_in, 1);
    snd_seq_port_subscribe_set_queue(app_data->subs_in, app_data->queue_in);
    snd_seq_subscribe_port(app_data->seqp, app_data->subs_in);

    // TODO: check if connection was made

    winlog(app_data, "Connected.\n\0", -1);
    app_data->is_connected = true;
    send_sysex_init(app_data);
}


/**
* Glide slider has been slided.
*/
void on_slider_glide_value_changed(GtkRange *scale, AppData *app_data) {
    if (app_data->surpress_signals) return;
    double v = gtk_range_get_value(scale);
    printf( "Glide Value: %.2f\n", v);
    send_sysex_slider(app_data, 0x17, v);
}

/**
* Lift slider has been slided.
*/
void on_slider_lift_value_changed(GtkRange *scale, AppData *app_data) {
    if (app_data->surpress_signals) return;
    double v = gtk_range_get_value(scale);
    printf( "Lift Value: %.2f\n", v);
    send_sysex_slider(app_data, 0x1b, v);
}


/**
* Press slider has been slided.
*/
void on_slider_press_value_changed(GtkRange *scale, AppData *app_data) {
    if (app_data->surpress_signals) return;
    double v = gtk_range_get_value(scale);
    printf( "Press Value: %.2f\n", v);
    send_sysex_slider(app_data, 0x19, v);
}

/**
* Strike slider has been slided.
*/
void on_slider_strike_value_changed(GtkRange *scale, AppData *app_data) {
    if (app_data->surpress_signals) return;
    double v = gtk_range_get_value(scale);
    printf( "Strike Value: %.2f\n", v);
    send_sysex_slider(app_data, 0x1a, v);
}


/**
* Slide slider has been slided
*/
void on_slider_slide_value_changed(GtkRange *scale, AppData *app_data) {
    if (app_data->surpress_signals) return;
    double v = gtk_range_get_value(scale);
    printf( "Slide Value: %.2f\n", v);
    send_sysex_slider(app_data, 0x18, v);
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
    printf("MIDI source port %d\n", ev->source.port);
    printf("MIDI source client %d\n", ev->source.client);
    if (ev->type == SND_SEQ_EVENT_SYSEX) {
        printf("Sysex received\n");
        for (int i = 0 ; i < ev->data.ext.len ; ++i) {
            printf("%02d: 0x%02x ", i, ((char*)ev->data.ext.ptr)[i]);
        }
        printf("%d bytes.\n", ev->data.ext.len);

        uint8_t *data = (uint8_t*)ev->data.ext.ptr;
        // If initial sysex
        if (ev->data.ext.len == 224) {
            app_data->surpress_signals = true;
            gtk_range_set_value(app_data->slider_glide, data[105]);
            gtk_range_set_value(app_data->slider_slide, data[106]);
            gtk_range_set_value(app_data->slider_press, data[107]);
            gtk_range_set_value(app_data->slider_strike, data[108]);
            gtk_range_set_value(app_data->slider_lift, data[148]);
            app_data->surpress_signals = false;
        }
        else if (ev->data.ext.len == 9 
            && memcmp(slider_mask, data, SLIDER_MASK_LEN)) {
            app_data->surpress_signals = true;
            switch(data[6]) {
                case 0x17:
                    gtk_range_set_value(app_data->slider_glide, data[7]);
                    break;
                case 0x18:
                    gtk_range_set_value(app_data->slider_slide, data[7]);
                    break;
                case 0x19:
                    gtk_range_set_value(app_data->slider_press, data[7]);
                    break;
            }
            app_data->surpress_signals = false;
        }
    }
    else if (snd_seq_ev_is_note_type(ev)) 
        printf("Note!\n");
}


/**
* Send sysex initially
*/
void send_sysex_init(AppData *app_data) {
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, app_data->local_out.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    uint8_t data[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3f, 0xf7 };

    snd_seq_ev_set_sysex(&ev, 7, data);

    snd_seq_event_output(app_data->seqp, &ev);
    snd_seq_drain_output(app_data->seqp);
}


/**
* Send sysex for MPE on or off
*/
void send_sysex_mpe_state(AppData *app_data, bool state) {
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, app_data->local_out.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    uint8_t data[] = { 
        0xf0, 0x00, 0x21, 0x10, 0x78, 0x3d, 0x0e, (state ? 0x01 : 0), 0xf7 
    };

    snd_seq_ev_set_sysex(&ev, 9, data);

    snd_seq_event_output(app_data->seqp, &ev);
    snd_seq_drain_output(app_data->seqp);
}


/**
* Send sysex for sliders
*/
void send_sysex_slider(AppData *app_data, uint8_t type, uint8_t v) {
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, app_data->local_out.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    uint8_t data[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3d, type, v, 0xf7 };

    snd_seq_ev_set_sysex(&ev, 9, data);

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

    // flag to surpress signals for sysex received messages and subsequently
    // adjusting sliders.
    app_data->surpress_signals = false;
    
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
        SND_SEQ_PORT_TYPE_APPLICATION
    );

    int out = snd_seq_create_simple_port(
        app_data->seqp, 
        "out", 
        SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, 
        SND_SEQ_PORT_TYPE_APPLICATION
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
    
    // will be called every ms
    g_timeout_add(1, on_timeout, app_data);

    // init done
    g_object_unref(builder);
 
    g_debug("Entering loop");
    gtk_main();

    g_slice_free(AppData, app_data);
 
    return 0;
}

