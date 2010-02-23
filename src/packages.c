#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <Eina.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore_Str.h>
#include <libeoi.h>
#include <libkeys.h>
#include <libchoicebox.h>
#include "eabout.h"
#include "script.h"

typedef struct package_t package_t;
struct package_t
{
    char *package;
    char *version;
};

static void
eabout_package_item_free(package_t *package)
{
    free(package->package);
    free(package->version);
}

void
eabout_packages_free(Eina_List *list)
{
    package_t *data;
    EINA_LIST_FREE(list, data)
        eabout_package_item_free(data);
}

static struct package_t *
eabout_packages_parse_package(const char *line)
{
    package_t *package = NULL;
    char **tokens = ecore_str_split(line, " ", 0);
    if(!strcmp(tokens[2], "install"))
    {
        package = calloc(1, sizeof(package_t));
        package->package = strdup(tokens[0]);
        package->version = strdup(tokens[1]);
    }
    free(tokens[0]);
    free(tokens);
    return package;
}

static Eina_List *
eabout_load_packages_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    Eina_List *list = NULL;
    package_t *package;
    char buf[1024];
    if(!file)
        return NULL;
    while(!feof(file))
    {
        fgets(buf, 1024, file);
        package = eabout_packages_parse_package(buf);
        if(package)
            list = eina_list_append(list, package);
    }
    return list;
}

static void
eabout_load_packages_callback(Evas_Object *choicebox, const char *tmpfile,
        void *data __attribute__((unused)))
{
    Eina_List *packages = evas_object_data_get(choicebox, "dpkg");
    if(packages)
        eabout_packages_free(packages);
    packages = eabout_load_packages_file(tmpfile);
    evas_object_data_set(choicebox, "dpkg", packages);
    choicebox_set_size(choicebox, eina_list_count(packages));
}

static void
eabout_load_packages(Evas_Object *choicebox)
{
    eabout_load_script(choicebox, PKGLIBDIR "/dpkg.sh",
        eabout_load_packages_callback, NULL);
}

static void
_close_handler(Evas_Object *choicebox, void *param __attribute__((unused)))
{
    Eina_List *packages = evas_object_data_get(choicebox, "dpkg");
    if(packages)
        eabout_packages_free(packages);
}

static void
_handler(Evas_Object *choicebox __attribute__((unused)),
         int item_num __attribute__((unused)),
         bool is_alt  __attribute__((unused)),
         void *param  __attribute__((unused)))
{
}

static void
_draw_handler(Evas_Object *choicebox, Evas_Object *item, int item_num,
    int page_position __attribute__((unused)),
    void *param __attribute__((unused)))
{

    Eina_List *packages = evas_object_data_get(choicebox, "dpkg");
    if(packages)
    {
        package_t *package = eina_list_nth(packages, item_num);
        if(package)
        {
            edje_object_part_text_set(item, "package", package->package);
            edje_object_part_text_set(item, "version", package->version);
        }
    }
}

static void
_page_handler(Evas_Object *choicebox, int cur_page, int total_pages,
              void *param __attribute__((unused)))
{
     Evas *canvas = evas_object_evas_get(choicebox);
     Evas_Object *edje = evas_object_name_find(canvas, MAIN_EDJE_ID);
     Evas_Object *obj = edje_object_part_swallow_get(edje, "contents");
     if(obj != choicebox)
        return;
     choicebox_aux_edje_footer_handler(edje, "footer", cur_page, total_pages);
}

static void
_key_handler(void *param,
             Evas *e __attribute__((unused)),
             Evas_Object *choicebox, void *event_info)
{
    keys_t *keys = param;
    Evas_Event_Key_Up *ev = event_info;
    const char *action = keys_lookup_by_event(keys,  "eabout", ev);
    if(!strcmp(action,"PageUp"))
        choicebox_prevpage(choicebox);
    else if(!strcmp(action, "PageDown"))
        choicebox_nextpage(choicebox);
}

Evas_Object *
eabout_packages_choicebox(Evas *canvas, keys_t *keys)
{
    choicebox_info_t info = {
        .background = NULL,
        .frame_theme_file = THEME_EDJE,
        .frame_theme_group = "packages-choicebox",
        .item_theme_file = THEME_EDJE,
        .item_theme_group = "package-items",
        .handler = _handler,
        .draw_handler = _draw_handler,
        .page_handler = _page_handler,
        .close_handler = _close_handler,
    };
    Evas_Object *choicebox = choicebox_new(canvas, &info, NULL);
    choicebox_set_hinted(choicebox, false);
    evas_object_name_set(choicebox, "dpkg");
    eabout_load_packages(choicebox);
    evas_object_event_callback_add(choicebox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &_key_handler,
                                   keys);
    eoi_register_fullscreen_choicebox(choicebox);
    return choicebox;
}
