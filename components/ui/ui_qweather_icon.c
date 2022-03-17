#include "ui_qweather_icon.h"

#include <esp_log.h>

typedef struct{
    uint16_t code;
    char *font;
} qweather_icon_desc_t;

static const char *TAG = "ui_qweather_icon";

static const qweather_icon_desc_t qweather_icon_desc[] = {
    { 100, "\uf101" },
    { 101, "\uf102" },
    { 102, "\uf103" },
    { 103, "\uf104" },
    { 104, "\uf105" },
    { 150, "\uf106" },
    { 151, "\uf107" },
    { 152, "\uf108" },
    { 153, "\uf109" },
    { 300, "\uf10a" },
    { 301, "\uf10b" },
    { 302, "\uf10c" },
    { 303, "\uf10d" },
    { 304, "\uf10e" },
    { 305, "\uf10f" },
    { 306, "\uf110" },
    { 307, "\uf111" },
    { 308, "\uf112" },
    { 309, "\uf113" },
    { 310, "\uf114" },
    { 311, "\uf115" },
    { 312, "\uf116" },
    { 313, "\uf117" },
    { 314, "\uf118" },
    { 315, "\uf119" },
    { 316, "\uf11a" },
    { 317, "\uf11b" },
    { 318, "\uf11c" },
    { 350, "\uf11d" },
    { 351, "\uf11e" },
    { 399, "\uf11f" },
    { 400, "\uf120" },
    { 401, "\uf121" },
    { 402, "\uf122" },
    { 403, "\uf123" },
    { 404, "\uf124" },
    { 405, "\uf125" },
    { 406, "\uf126" },
    { 407, "\uf127" },
    { 408, "\uf128" },
    { 409, "\uf129" },
    { 410, "\uf12a" },
    { 456, "\uf12b" },
    { 457, "\uf12c" },
    { 499, "\uf12d" },
    { 500, "\uf12e" },
    { 501, "\uf12f" },
    { 502, "\uf130" },
    { 503, "\uf131" },
    { 504, "\uf132" },
    { 507, "\uf133" },
    { 508, "\uf134" },
    { 509, "\uf135" },
    { 510, "\uf136" },
    { 511, "\uf137" },
    { 512, "\uf138" },
    { 513, "\uf139" },
    { 514, "\uf13a" },
    { 515, "\uf13b" },
    { 0, "\uF146" },
};

const char *ui_qweather_icon_get_from_code(uint16_t code)
{
    const qweather_icon_desc_t *ptr = qweather_icon_desc;

    while(ptr->code != 0) {
        if(ptr->code == code) {
            return ptr->font;
        }
        ptr ++;
    }

    ESP_LOGW(TAG, "unknown qweather icon: %d", code);
    return "\uF146"; //N/A标志
}