#define _GNU_SOURCE 1
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <Ecore.h>
#include <Evas.h>
#include "script.h"

static bool _lock;

struct callback_info
{
    Evas_Object *object;
    char *filename;
    void *data;
    load_script_callback_t callback;
    Ecore_Event_Handler *handler;
};

static Eina_Bool
eabout_load_script_callback(void *data, int type __attribute__((unused)),
    void *event __attribute__((unused)))
{
    struct callback_info *info = data;
    info->callback(info->object, info->filename, info->data);
    ecore_event_handler_del(info->handler);
    unlink(info->filename);
    free(info->filename);
    free(info);
    _lock = false;
    return ECORE_CALLBACK_CANCEL;
}

void
eabout_load_script(Evas_Object *object, const char *script,
    load_script_callback_t callback, void *param)
{
    struct callback_info *info = calloc(1, sizeof(struct callback_info));
    if(_lock)
    {
        printf("Already run\n");
        return;
    }
    _lock = true;
    char *tmpfile = strdup("/tmp/eaboutXXXXXX");
    char *cmd;
    mktemp(tmpfile);
    asprintf(&cmd, "%s %s", script, tmpfile);
    info->data = param;
    info->object = object;
    info->callback = callback;
    info->filename = tmpfile;
    printf("run: %s\n", cmd);
    Ecore_Exe *exe = ecore_exe_run(cmd, NULL);
    info->handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                            eabout_load_script_callback,
                                            info);
    free(cmd);
}


