#ifndef _SCRIPT_H
#define _SCRIPT_H

typedef void (*load_script_callback_t)(Evas_Object *obj,
        const char *filename, void *data);

void
eabout_load_script(Evas_Object *object, const char *script,
    load_script_callback_t callback, void *param);

#endif
