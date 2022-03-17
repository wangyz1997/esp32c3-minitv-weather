#ifndef __UI_CONNECTING_SCREEN_H
#define __UI_CONNECTING_SCREEN_H

typedef enum {
    status_waiting_wifi,
    status_fetching_data,
    status_wifi_error,
    status_data_error,
} ui_connecting_screen_status_t;

void ui_connecting_screen_show(ui_connecting_screen_status_t status);
void ui_connecting_screen_update(ui_connecting_screen_status_t status);
void ui_connecting_screen_delete();

#endif
