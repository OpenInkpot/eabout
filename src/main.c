#define _GNU_SOURCE 1
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <unistd.h>
#define _(x) x

#include <liblops.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include <libeoi.h>
#include <libeoi_textbox.h>
#include <libeoi_themes.h>
#include <libeoi_battery.h>
#include <libeoi_clock.h>
#include <libeoi_help.h>
#include <libchoicebox.h>
#include <libkeys.h>
#include "script.h"
#include "eabout.h"

static keys_t *_keys;

keys_t *get_keys()
{
    if(!_keys)
        _keys = keys_alloc("eabout");
    return _keys;
}




static Eina_Bool exit_handler(void *param __attribute__((unused)),
                        int ev_type __attribute__((unused)),
                        void *event __attribute__((unused)))
{
    ecore_main_loop_quit();
    return 1;
}

static void main_win_close_handler(Ecore_Evas *main_win __attribute__((unused)))
{
    ecore_main_loop_quit();
}


static void
eabout_default_footer(Evas *evas)
{
    Evas_Object *main_edje = evas_object_name_find(evas, MAIN_EDJE_ID);
    edje_object_part_text_set(main_edje, "footer",
        gettext("OK - Packages version"));
}

static void
eabout_page_handler(Evas_Object *object, int page, int pages,
    void *param __attribute__ ((unused)))
{
    Evas *evas = evas_object_evas_get(object);
    Evas_Object *win = evas_object_name_find(evas, MAIN_EDJE_ID);
    if(pages > 1)
        choicebox_aux_edje_footer_handler(win, "footer", page, pages);
    else
        eabout_default_footer(evas);
}

/* Following function copypasted from gm/src/setup.c */
#define VERSION_SIZE 1024

static void
eabout_version_draw(Evas_Object *item)
{
    char version_str[VERSION_SIZE]="N/A";
    int fd = open("/etc/openinkpot-version", O_RDONLY);
    if (fd != -1) {
        int r = readn(fd, version_str, VERSION_SIZE-1);
        if( r > 0) {
            version_str[r-1]='\0';
            char *c = strchr(version_str,'\n');
            if(c)
                *c = '\0';
        }
        close(fd);
    }
    edje_object_part_text_set(item, "version", version_str);
    eabout_default_footer(evas_object_evas_get(item));
}
/* end of copypaste */

static char *
eabout_load_text_file_internal(const char *filename, bool br)
{

    FILE *f;
    FILE *stream;
    size_t size;
    char *text;
    int c;

    f = fopen(filename, "r");
    if (!f)
    {
        fprintf(stderr, "can't open %s\n", filename);
        return NULL;
    }

    stream = open_memstream(&text, &size);

    while (true)  {
        c = fgetc(f);
        if(feof(f))
            break;
        if(br && c == '\n')
            fputs("<br>", stream);
        fputc(c, stream);
    }

    fclose(f);
    fclose(stream);
    return text;
}

char *
eabout_load_text_file(const char *filename)
{
    return eabout_load_text_file_internal(filename, false);
}

static void
eabout_load_file(Evas_Object *textbox, const char *filename, bool br)
{
    char *text = eabout_load_text_file_internal(filename, br);
    eoi_textbox_text_set(textbox, text);
    free(text);
}

static void
eabout_fill_info(Evas_Object *textbox)
{
    char *text;
    size_t size;
    struct eabout_info *info = eabout_load_info();
    FILE *stream = open_memstream(&text, &size);
    /* format "overview" text */
    fprintf(stream, "%s: %s<br>", gettext("Device name"), info->device_name);
    fprintf(stream, "%s: %s\n<br>", gettext("Firmware version"), info->version);
    fprintf(stream, "%s: %s (%s MHz)<br>", gettext("CPU"),
                    info->cpu, info->cpu_freq);
    fprintf(stream, "%s: %dKiB<br>", gettext("RAM"), info->ram);
    fprintf(stream, "%s: %dMB (%s: %dMB)<br>",
        gettext("Internal memory"), info->storage,
        gettext("Available for user"), info->storage_avail);
    fprintf(stream, "%s: %dMB (%s: %dMB)<br>",
        gettext("Memory card"), info->card,
        gettext("Available for user"), info->card_avail);
    fprintf(stream, "%s: %s<br>", gettext("Manufacturer website"),
            info->vendor_site);
    fprintf(stream, "%s: %s<br>", gettext("Developers website"),
            info->developers_site);
    fclose(stream);
    eoi_textbox_text_set(textbox, text);
    free(text);
    eabout_free_info(info);
}


static void
eabout_script_callback(Evas_Object *textbox, const char *tmpfile,
    void *param __attribute__((unused)))
{
    eabout_load_file(textbox, tmpfile, true);
}

static void
eabout_script(Evas_Object *textbox, const char *script)
{
    eabout_load_script(textbox, script, eabout_script_callback, NULL);
}

static Evas_Object *
eabout_swap_widget(Evas *evas, const char *widget)
{
    Evas_Object *main_edje = evas_object_name_find(evas, MAIN_EDJE_ID);
    Evas_Object *obj = edje_object_part_swallow_get(main_edje, "contents");
    assert(main_edje);
    Evas_Object *replacement = evas_object_name_find(evas, widget);
    if(obj == replacement)
        return replacement;

    if(!replacement)
    {
        printf("No widget %s\n", widget);
        return NULL;
    }
    if(obj)
    {
        evas_object_focus_set(obj, 0);
        edje_object_part_unswallow(main_edje, obj);
        evas_object_hide(obj);
    }
    evas_object_show(replacement);
    edje_object_part_swallow(main_edje, "contents", replacement);
    evas_object_raise(replacement);
    evas_object_focus_set(replacement, 1);
    return replacement;
}

static void
eabout_page_set(Evas *evas, const char *action)
{
    Evas_Object *main_edje = evas_object_name_find(evas, MAIN_EDJE_ID);
    if(!strcmp(action, "Info")){
        Evas_Object *overview = eabout_swap_widget(evas, OVERVIEW_WIDGET_ID);
        eabout_default_footer(evas);
        edje_object_part_text_set(main_edje, "title", gettext("Overview"));
        eabout_fill_info(overview);
        return;
    }
    if(!strcmp(action, "Misc"))
    {
        Evas_Object *textbox = eabout_swap_widget(evas, TEXTBOX_WIDGET_ID);
        eabout_script(textbox, PKGLIBDIR "/misc.sh");
        eabout_default_footer(evas);
        edje_object_part_text_set(main_edje, "title", gettext("System Report"));
        return;
    }
    if(!strcmp(action, "Versions"))
    {
        eabout_default_footer(evas);
        eabout_swap_widget(evas, "dpkg");
        edje_object_part_text_set(main_edje, "title", gettext("Packages"));
    }
}


static void
eabout_pagination_key_handler(void *data __attribute__((unused)),
                  Evas *evas,
                  Evas_Object *textbox,
                  void *event_info)
{
    keys_t *keys = get_keys();
    const char *action = keys_lookup_by_event(keys, "eabout",
                        (Evas_Event_Key_Up *)event_info);
    if (!action || !strlen(action))
        return;

    if (!strcmp(action, "PageDown"))
        eoi_textbox_page_next(textbox);
    else if (!strcmp(action, "PageUp"))
        eoi_textbox_page_prev(textbox);
}

static void
eabout_packages_key_handler(void *data __attribute__((unused)),
                   Evas *evas,
                   Evas_Object *obj __attribute__((unused)),
                   void *event_info)
{
    keys_t *keys = get_keys();
    const char *action = keys_lookup_by_event(keys, "eabout",
            (Evas_Event_Key_Up *) event_info);
    if (!action || !strlen(action))
        return;
    else if(!strcmp(action, "Quit"))
    {
        eabout_page_set(evas, "Info");
    }
}

static void
eabout_key_handler(void *data __attribute__((unused)),
                   Evas *evas,
                   Evas_Object *obj __attribute__((unused)),
                   void *event_info)
{
    keys_t *keys = get_keys();
    const char *action = keys_lookup_by_event(keys, "eabout",
            (Evas_Event_Key_Up *) event_info);
    if (!action || !strlen(action))
        return;
    else if(!strcmp(action, "Quit"))
    {
        ecore_main_loop_quit();
        return;
    }
    else if(!strcmp(action, "Help"))
    {
        eoi_help_show(evas, "eabout", NULL,
            gettext("About OpenInkpot: Help"),
            keys, "eabout");
        return;
    }
    eabout_page_set(evas, action);
}

static void run()
{
    int width, height;
    Ecore_Evas *main_win = ecore_evas_new(NULL, 0, 0, 1, 1, NULL);
    ecore_evas_screen_geometry_get(main_win, NULL, NULL, &width, &height);
    ecore_evas_resize(main_win, width, height);
    ecore_evas_title_set(main_win, "EAbout");
    ecore_evas_name_class_set(main_win, "EAbout", "EAbout");

    Evas *main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

    Evas_Object *main_canvas_edje = eoi_main_window_create(main_canvas);
    evas_object_name_set(main_canvas_edje, MAIN_EDJE_ID);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "title", "");
    evas_object_move(main_canvas_edje, 0, 0);
    evas_object_resize(main_canvas_edje, width, height);


    eoi_fullwindow_object_register(main_win, main_canvas_edje);
    eoi_run_clock(main_canvas_edje);
    eoi_run_battery(main_canvas_edje);
    evas_object_show(main_canvas_edje);

    Evas_Object *overview = eoi_textbox_new(main_canvas, THEME_EDJE, "eabout",
        eabout_page_handler);
    evas_object_name_set(overview, OVERVIEW_WIDGET_ID);
    Evas_Object *textbox = eoi_textbox_new(main_canvas, THEME_EDJE, "eabout",
        eabout_page_handler);
    evas_object_name_set(textbox, TEXTBOX_WIDGET_ID);

    Evas_Object *packages =eabout_packages_choicebox(main_canvas, get_keys());

    eabout_page_set(main_canvas, "Info");

    evas_object_event_callback_add(textbox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_pagination_key_handler, NULL);
    evas_object_event_callback_add(overview,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_pagination_key_handler, NULL);
    evas_object_event_callback_add(textbox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_key_handler, NULL);
    evas_object_event_callback_add(overview,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_key_handler, NULL);
    evas_object_event_callback_add(packages,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_packages_key_handler, NULL);


    ecore_evas_show(main_win);

    ecore_main_loop_begin();

    // FIXME: avoid segfault (crash unless delete textblock manually)
    evas_object_del(overview);
    evas_object_del(textbox);
    evas_object_del(packages);
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    setlocale(LC_ALL, "");
    textdomain("eabout");
    if(!evas_init())
        err(1, "Unable to initialize Evas\n");
    if(!ecore_init())
        err(1, "Unable to initialize Ecore\n");
    if(!ecore_evas_init())
        err(1, "Unable to initialize Ecore_Evas\n");
    if(!edje_init())
        err(1, "Unable to initialize Edje\n");

    efreet_init();

    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    run();

    if(_keys)
        keys_free(_keys);

    /* Keep valgrind happy */
    edje_file_cache_set(0);
    edje_collection_cache_set(0);

    efreet_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    edje_shutdown();
    return 0;
}
