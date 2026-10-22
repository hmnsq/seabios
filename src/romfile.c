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
__romfile_findprefix(const var_na *prefix, ကိန်းပြည့် prefixlen, sa_nracanaa romfile_s *prev)
{
    sa_nracanaa romfile_s *cur = RomfileRoot;
    အကယ်၍ (prev)
        cur = prev->next;
    နေစဉ် (cur) {
        အကယ်၍ (memcmp(prefix, cur->name, prefixlen) == 0)
            ပြန်ပေး cur;
        cur = cur->next;
    }
    ပြန်ပေး NULL;
}

sa_nracanaa romfile_s *
romfile_findprefix(const var_na *prefix, sa_nracanaa romfile_s *prev)
{
    ပြန်ပေး __romfile_findprefix(prefix, strlen(prefix), prev);
}

sa_nracanaa romfile_s *
romfile_find(const var_na *name)
{
    ပြန်ပေး __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const var_na *name, ကိန်းပြည့် *psize)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    အကယ်၍ (!file)
        ပြန်ပေး NULL;

    ကိန်းပြည့် filesize = file->size;
    အကယ်၍ (!filesize)
        ပြန်ပေး NULL;

    var_na *data = malloc_tmphigh(filesize+1);
    အကယ်၍ (!data) {
        warn_noalloc();
        ပြန်ပေး NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ကိန်းပြည့် ret = file->copy(file, data, filesize);
    အကယ်၍ (ret < 0) {
        free(data);
        ပြန်ပေး NULL;
    }
    အကယ်၍ (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ပြန်ပေး data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const var_na *name, u64 defval)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    အကယ်၍ (!file)
        ပြန်ပေး defval;

    ကိန်းပြည့် filesize = file->size;
    အကယ်၍ (!filesize || filesize > parimaa_na(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ပြန်ပေး defval;

    u64 val = 0;
    ကိန်းပြည့် ret = file->copy(file, &val, parimaa_na(val));
    အကယ်၍ (ret < 0)
        ပြန်ပေး defval;
    ပြန်ပေး val;
}

u32
romfile_loadbool(const var_na *name, u32 defval)
{
    static const var_na *true_strings[] = { "1", "on", "yes" };
    static const var_na *false_strings[] = { "0", "off", "no" };
    var_na *str = romfile_loadfile(name, NULL);
    ကိန်းပြည့် i;

    အကယ်၍ (!str)
        ပြန်ပေး defval;

    အတွက် (i = 0; i < ARRAY_SIZE(true_strings); i++)
        အကယ်၍ (0 == strcmp(str, true_strings[i]))
            ပြန်ပေး 1;

    အတွက် (i = 0; i < ARRAY_SIZE(false_strings); i++)
        အကယ်၍ (0 == strcmp(str, false_strings[i]))
            ပြန်ပေး 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ပြန်ပေး defval;
}

sa_nracanaa const_romfile_s {
    sa_nracanaa romfile_s file;
    void *data;
};

static ကိန်းပြည့်
const_read_file(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    အကယ်၍ (file->size > maxlen)
        ပြန်ပေး -1;
    sa_nracanaa const_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa const_romfile_s, file);
    အကယ်၍ (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ပြန်ပေး file->size;
}

static void
const_romfile_add(var_na *name, void *data, ကိန်းပြည့် size)
{
    sa_nracanaa const_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
    အကယ်၍ (!cfile) {
        warn_noalloc();
        ပြန်ပေး;
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
    အကယ်၍ (!data) {
        warn_noalloc();
        ပြန်ပေး;
    }
    *data = value;
    const_romfile_add(name, data, parimaa_na(*data));
}
