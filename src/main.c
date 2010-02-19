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
#include <Ecore_X.h>
#include <Edje.h>

#include <libeoi.h>
#include <libeoi_textbox.h>
#include <libeoi_themes.h>
#include <libeoi_battery.h>
#include <libeoi_clock.h>
#include <libkeys.h>

#define THEME_EDJE "eabout"
#define TEXTBOX_WIDGET_ID "eabout-textbox-widget"
#define OVERVIEW_WIDGET_ID "eabout-overview-widget"
#define MAIN_EDJE_ID "eabout-main-edje"

static keys_t *_keys;

keys_t *get_keys()
{
    if(!_keys)
        _keys = keys_alloc("eabout");
    return _keys;
}




static int exit_handler(void *param __attribute__((unused)),
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
eabout_page_handler(Evas_Object *object, int page, int pages, void *data,
    void *param __attribute__ ((unused)))
{
    Evas *evas = evas_object_evas_get(object);
    Evas_Object *win = evas_object_name_find(evas, MAIN_EDJE_ID);
    choicebox_aux_edje_footer_handler(win, "footer", page,
                                              pages);
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
}
/* end of copypaste */


/* FIXME: copypaste from libeoi, must be  textbox API */
static void
eabout_load_file(Evas_Object *textbox, const char *filename, bool br)
{
    FILE *f;
    FILE *stream;
    size_t size;
    char *text;
    int c;

    f = fopen(filename, "r");
    if (!f)
        return;

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

    eoi_textbox_text_set(textbox, text);
    printf("%s\n", text);
    free(text);
}


static int
eabout_load_script_callback(void *data, int type, void *event)
{
    Evas_Object *textbox = data;
    char *tmpfile = evas_object_data_get(textbox, "script-tmpfile");
    if(!tmpfile)
    {
        printf("Error, no tmpfile set\n");
        return ECORE_CALLBACK_CANCEL;
    }
    eabout_load_file(textbox, tmpfile, true);
    unlink(tmpfile);
    free(tmpfile);
    evas_object_data_set(textbox, "script-tmpfile", NULL);
    return ECORE_CALLBACK_CANCEL;
}

static void
eabout_load_script(Evas_Object *textbox, const char *script)
{
    if(evas_object_data_get(textbox, "script-tmpfile"))
    {
        printf("Already run\n");
        return;
    }
    char *tmpfile = strdup("/tmp/eaboutXXXXXX");
    char *cmd;
    mktemp(tmpfile);
    asprintf(&cmd, "%s %s", script, tmpfile);
    Ecore_Exe *exe = ecore_exe_run(cmd, NULL);
    evas_object_data_set(textbox, "script-tmpfile", tmpfile);
    ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                            eabout_load_script_callback,
                            textbox);
    free(cmd);
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
        evas_object_hide(obj);
    }
        evas_object_show(replacement);
        edje_object_part_swallow(main_edje, "contents", replacement);
        evas_object_focus_set(replacement, 1);
    return replacement;
}

static void
eabout_page_set(Evas *evas, const char *action)
{
    if(!strcmp(action, "Info")){
        Evas_Object *overview = eabout_swap_widget(evas, OVERVIEW_WIDGET_ID);
        eabout_version_draw(overview);
        return;
    }
    Evas_Object *textbox = eabout_swap_widget(evas, TEXTBOX_WIDGET_ID);
    if(!strcmp(action, "Misc"))
        eabout_load_script(textbox, "/usr/lib/eabout/misc.sh");
    else if(!strcmp(action, "Versions"))
        eabout_load_script(textbox, "/usr/lib/eabout/dpkg.sh");
}


static void
eabout_key_handler(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
    keys_t *keys = get_keys();
    const char *action = keys_lookup_by_event(keys, "eabout", event_info);
    if (!action || !strlen(action))
        return;
    Evas_Object *textbox = evas_object_name_find(evas, TEXTBOX_WIDGET_ID);
    if (!strcmp(action, "PageDown"))
        eoi_textbox_page_next(textbox);
    else if (!strcmp(action, "PageUp"))
        eoi_textbox_page_prev(textbox);
    else if(!strcmp(action, "Quit"))
    {
        ecore_main_loop_quit();
        return;
    }
    eabout_page_set(evas, action);
}

static void run()
{
    Ecore_Evas *main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    ecore_evas_title_set(main_win, "EAbout");
    ecore_evas_name_class_set(main_win, "EAbout", "EAbout");

    Evas *main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

    Evas_Object *main_canvas_edje = eoi_main_window_create(main_canvas);
    evas_object_name_set(main_canvas_edje, MAIN_EDJE_ID);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "title", "");
    evas_object_move(main_canvas_edje, 0, 0);
    evas_object_resize(main_canvas_edje, 600, 800);


    eoi_fullwindow_object_register(main_win, main_canvas_edje);
    eoi_run_clock(main_canvas_edje);
    eoi_run_battery(main_canvas_edje);
    evas_object_show(main_canvas_edje);

    Evas_Object *textbox = eoi_textbox_new(main_canvas, THEME_EDJE, "eabout",
        eabout_page_handler);
    evas_object_name_set(textbox, TEXTBOX_WIDGET_ID);

    Evas_Object *overview = eoi_create_themed_edje(main_canvas,
                                                   THEME_EDJE,
                                                   "overview");
    evas_object_name_set(overview, OVERVIEW_WIDGET_ID);

    eabout_page_set(main_canvas, "Info");
    evas_object_event_callback_add(textbox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_key_handler, NULL);
    evas_object_event_callback_add(overview,
                                   EVAS_CALLBACK_KEY_UP,
                                   &eabout_key_handler, NULL);

    ecore_evas_show(main_win);

    ecore_main_loop_begin();

    // FIXME: avoid segfault (crash unless delete textblock manually)
    evas_object_del(overview);
    evas_object_del(textbox);
}

static
void exit_all(void *param __attribute__((unused))) {
    ecore_main_loop_quit();
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    setlocale(LC_ALL, "");
    textdomain("gm");
    if(!evas_init())
        err(1, "Unable to initialize Evas\n");
    if(!ecore_init())
        err(1, "Unable to initialize Ecore\n");
    if(!ecore_evas_init())
        err(1, "Unable to initialize Ecore_Evas\n");
    if(!edje_init())
        err(1, "Unable to initialize Edje\n");

    ecore_x_io_error_handler_set(exit_all, NULL);
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    run();

    if(_keys)
        keys_free(_keys);

    /* Keep valgrind happy */
    edje_file_cache_set(0);
    edje_collection_cache_set(0);

    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    edje_shutdown();
    return 0;
}
