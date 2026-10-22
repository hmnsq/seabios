// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "romfile.h" // struct romfile_s
#include "string.h" // memcmp

static sa_nracanaa romfile_s *RomfileRoot VARVERIFY32INIT;

void
romfile_add(sa_nracanaa romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = RomfileRoot;
    RomfileRoot = file;
}

// Search for the specified file.
static sa_nracanaa romfile_s *
__romfile_findprefix(const var_na *prefix, පූර්ණ_සංඛ්‍යා prefixlen, sa_nracanaa romfile_s *prev)
{
    sa_nracanaa romfile_s *cur = RomfileRoot;
    නම් (prev)
        cur = prev->next;
    අතරතුර (cur) {
        නම් (memcmp(prefix, cur->name, prefixlen) == 0)
            ආපසු cur;
        cur = cur->next;
    }
    ආපසු NULL;
}

sa_nracanaa romfile_s *
romfile_findprefix(const var_na *prefix, sa_nracanaa romfile_s *prev)
{
    ආපසු __romfile_findprefix(prefix, strlen(prefix), prev);
}

sa_nracanaa romfile_s *
romfile_find(const var_na *name)
{
    ආපසු __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const var_na *name, පූර්ණ_සංඛ්‍යා *psize)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    නම් (!file)
        ආපසු NULL;

    පූර්ණ_සංඛ්‍යා filesize = file->size;
    නම් (!filesize)
        ආපසු NULL;

    var_na *data = malloc_tmphigh(filesize+1);
    නම් (!data) {
        warn_noalloc();
        ආපසු NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    පූර්ණ_සංඛ්‍යා ret = file->copy(file, data, filesize);
    නම් (ret < 0) {
        free(data);
        ආපසු NULL;
    }
    නම් (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ආපසු data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const var_na *name, u64 defval)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    නම් (!file)
        ආපසු defval;

    පූර්ණ_සංඛ්‍යා filesize = file->size;
    නම් (!filesize || filesize > parimaa_na(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ආපසු defval;

    u64 val = 0;
    පූර්ණ_සංඛ්‍යා ret = file->copy(file, &val, parimaa_na(val));
    නම් (ret < 0)
        ආපසු defval;
    ආපසු val;
}

u32
romfile_loadbool(const var_na *name, u32 defval)
{
    static const var_na *true_strings[] = { "1", "on", "yes" };
    static const var_na *false_strings[] = { "0", "off", "no" };
    var_na *str = romfile_loadfile(name, NULL);
    පූර්ණ_සංඛ්‍යා i;

    නම් (!str)
        ආපසු defval;

    සඳහා (i = 0; i < ARRAY_SIZE(true_strings); i++)
        නම් (0 == strcmp(str, true_strings[i]))
            ආපසු 1;

    සඳහා (i = 0; i < ARRAY_SIZE(false_strings); i++)
        නම් (0 == strcmp(str, false_strings[i]))
            ආපසු 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ආපසු defval;
}

sa_nracanaa const_romfile_s {
    sa_nracanaa romfile_s file;
    void *data;
};

static පූර්ණ_සංඛ්‍යා
const_read_file(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    නම් (file->size > maxlen)
        ආපසු -1;
    sa_nracanaa const_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa const_romfile_s, file);
    නම් (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ආපසු file->size;
}

static void
const_romfile_add(var_na *name, void *data, පූර්ණ_සංඛ්‍යා size)
{
    sa_nracanaa const_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
    නම් (!cfile) {
        warn_noalloc();
        ආපසු;
    }
    memset(cfile, 0, parimaa_na(*cfile));
    strtcpy(cfile->file.name, name, parimaa_na(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

void
const_romfile_add_int(var_na *name, u32 value)
{
    u32 *data = malloc_tmp(parimaa_na(*data));
    නම් (!data) {
        warn_noalloc();
        ආපසු;
    }
    *data = value;
    const_romfile_add(name, data, parimaa_na(*data));
}
