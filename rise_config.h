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

static const uint8_t slider_mask[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3d };
#define SLIDER_MASK_LEN 6

typedef struct {
    snd_seq_t                   *seqp;
    snd_seq_client_info_t       *dest_client_info;
    int                         port_in;
    int                         port_out;
    snd_seq_addr_t              local_in;    
    snd_seq_addr_t              local_out;
    snd_seq_addr_t              remote_in;    
    snd_seq_addr_t              remote_out;  
    snd_seq_port_subscribe_t    *subs_in;
    snd_seq_port_subscribe_t    *subs_out;
    bool                        is_connected;
    int                         queue_in;
    int                         queue_out;      
    GtkListStore                *ls_midi_input;
    GtkRange                    *slider_glide;
    GtkRange                    *slider_lift;
    GtkRange                    *slider_press;
    GtkRange                    *slider_slide;
    GtkRange                    *slider_strike;
    GtkTextBuffer               *event_buffer;
    bool                        surpress_signals;
} AppData;

void load_client_list(AppData*);
void winlog(AppData*, char*, int8_t);
void on_bt_midi_input_refresh_clicked(GtkButton*, AppData*);
void on_bt_mpe_toggled(GtkToggleButton*, AppData*);
void on_cb_midi_input_changed(GtkComboBox*, AppData*);
void on_slider_glide_value_changed(GtkRange*, AppData*);
void on_slider_lift_value_changed(GtkRange*, AppData*);
void on_slider_press_value_changed(GtkRange*, AppData*);
void on_slider_slide_value_changed(GtkRange*, AppData*);
void on_slider_strike_value_changed(GtkRange*, AppData*);
gint on_timeout(gpointer data);
void on_window_main_destroy();
void process_midi(AppData*, snd_seq_event_t*);
void send_sysex_init(AppData*);
void send_sysex_mpe_state(AppData*, bool);
void send_sysex_slider(AppData*, uint8_t, uint8_t);


#endif
