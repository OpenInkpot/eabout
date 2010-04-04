#define _GNU_SOURCE 1
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Efreet.h>

#include "eabout.h"

static void
_get_freespace(Efreet_Ini *ini, const char *section, int *total, int *avail)
{
    efreet_ini_section_set(ini, section);
    const char *path = efreet_ini_string_get(ini, "Path");
    struct statfs fs;
    statfs(path, &fs);
    *total = fs.f_blocks * fs.f_bsize / ( 1024 * 1024 );
    *avail = fs.f_bavail * fs.f_bsize / ( 1024 * 1024 );
}

static void
eabout_get_freespace(eabout_info *info)
{
    Efreet_Ini *ini = efreet_ini_new("/etc/madshelf/disks.conf");
    if(ini)
    {
        _get_freespace(ini, "X-Madshelf-Disk-1", &info->card, &info->card_avail);
        _get_freespace(ini, "X-Madshelf-Disk-2", &info->storage,
                        &info->storage_avail);
        efreet_ini_free(ini);
    }
}

char *
_ini_get(Efreet_Ini *ini, const char *key)
{
    return strdup(efreet_ini_localestring_get(ini, key));
}

eabout_info *
eabout_load_info()
{
    struct sysinfo si;
    eabout_info *info=calloc(1, sizeof(eabout_info));
    info->version = eabout_load_text_file("/etc/openinkpot-version");
    char *eol = strchr(info->version, '\n');
    if(eol)
        *eol='\0';
    Efreet_Ini *ini = efreet_ini_new("/etc/eabout.conf");
    if(ini)
    {
        efreet_ini_section_set(ini, "overview");
        info->device_name = _ini_get(ini, "device_name");
        info->vendor_site = _ini_get(ini, "vendor_site");
        info->developers_site = _ini_get(ini, "developers_site");
        info->cpu = _ini_get(ini, "cpu");
        info->cpu_freq = _ini_get(ini, "cpu_freq");
        efreet_ini_free(ini);
    }
    eabout_get_freespace(info);
    sysinfo(&si);
    info->ram = si.totalram / 1024;
    return info;
}


void
eabout_free_info(eabout_info *info)
{
    free(info->device_name);
    free(info->version);
    free(info->vendor_site);
    free(info->developers_site);
    free(info->cpu);
    free(info->cpu_freq);
    free(info);
}
