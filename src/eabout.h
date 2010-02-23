#ifndef _EABOUT_H
#define _EABOUT_H 1

#define THEME_EDJE "eabout"
#define TEXTBOX_WIDGET_ID "eabout-textbox-widget"
#define OVERVIEW_WIDGET_ID "eabout-overview-widget"
#define MAIN_EDJE_ID "eabout-main-edje"

Evas_Object *
eabout_packages_choicebox(Evas *canvas, keys_t *keys);

#endif
