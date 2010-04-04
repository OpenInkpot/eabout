#ifndef _EABOUT_H
#define _EABOUT_H 1
#include "Evas.h"
#include "libkeys.h"
#define THEME_EDJE "eabout"
#define TEXTBOX_WIDGET_ID "eabout-textbox-widget"
#define OVERVIEW_WIDGET_ID "eabout-overview-widget"
#define MAIN_EDJE_ID "eabout-main-edje"

typedef struct eabout_info eabout_info;
struct eabout_info {
    int card;
    int card_avail;
    int storage;
    int storage_avail;
    int ram;
    int ram_free;

    char *device_name;
    char *version;
    char *vendor_site;
    char *developers_site;
    char *cpu;
    char *cpu_freq;
};

Evas_Object *
eabout_packages_choicebox(Evas *canvas, keys_t *keys);

char *
eabout_load_text_file(const char *filename);

eabout_info *eabout_load_info();
void eabout_free_info(eabout_info *);


#endif
