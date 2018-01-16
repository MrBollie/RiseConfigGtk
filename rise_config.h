#ifndef __RISE_CONFIG_H__
#define __RISE_CONFIG_H__

#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

enum {
    COL_ID = 0,
    COL_MAIN
};

typedef enum _bool {
    false = 0,
    true
} bool;

typedef struct {
    snd_seq_t                   *seqp;
    snd_seq_client_info_t       *dest_client_info;
    int                         port_in;
    int                         port_out;
    snd_seq_addr_t              local_in;    
    snd_seq_addr_t              local_out;
    snd_seq_addr_t              remote_in;    
    snd_seq_addr_t              remote_out;  
    bool                        is_connected;
    int                         queue_in;
    int                         queue_out;      
    GtkListStore                *ls_midi_input;
    GtkTextBuffer               *event_buffer;
} AppData;

void load_client_list(AppData*);
void on_bt_midi_input_refresh_clicked(GtkButton*, AppData*);
void on_cb_midi_input_changed(GtkComboBox*, AppData*);
void on_slider_slide_value_changed(GtkRange*, AppData*);
gint on_timeout(gpointer data);
void on_window_main_destroy();
void process_midi(AppData*, snd_seq_event_t*);
void send_sysex_init(AppData*);


#endif
