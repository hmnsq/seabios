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
__romfile_findprefix(const var_na *prefix, ចំនួនគត់ prefixlen, sa_nracanaa romfile_s *prev)
{
    sa_nracanaa romfile_s *cur = RomfileRoot;
    ប្រសិនបើ (prev)
        cur = prev->next;
    ខណៈពេល (cur) {
        ប្រសិនបើ (memcmp(prefix, cur->name, prefixlen) == 0)
            ត្រឡប់ cur;
        cur = cur->next;
    }
    ត្រឡប់ NULL;
}

sa_nracanaa romfile_s *
romfile_findprefix(const var_na *prefix, sa_nracanaa romfile_s *prev)
{
    ត្រឡប់ __romfile_findprefix(prefix, strlen(prefix), prev);
}

sa_nracanaa romfile_s *
romfile_find(const var_na *name)
{
    ត្រឡប់ __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const var_na *name, ចំនួនគត់ *psize)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    ប្រសិនបើ (!file)
        ត្រឡប់ NULL;

    ចំនួនគត់ filesize = file->size;
    ប្រសិនបើ (!filesize)
        ត្រឡប់ NULL;

    var_na *data = malloc_tmphigh(filesize+1);
    ប្រសិនបើ (!data) {
        warn_noalloc();
        ត្រឡប់ NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ចំនួនគត់ ret = file->copy(file, data, filesize);
    ប្រសិនបើ (ret < 0) {
        free(data);
        ត្រឡប់ NULL;
    }
    ប្រសិនបើ (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ត្រឡប់ data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const var_na *name, u64 defval)
{
    sa_nracanaa romfile_s *file = romfile_find(name);
    ប្រសិនបើ (!file)
        ត្រឡប់ defval;

    ចំនួនគត់ filesize = file->size;
    ប្រសិនបើ (!filesize || filesize > parimaa_na(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ត្រឡប់ defval;

    u64 val = 0;
    ចំនួនគត់ ret = file->copy(file, &val, parimaa_na(val));
    ប្រសិនបើ (ret < 0)
        ត្រឡប់ defval;
    ត្រឡប់ val;
}

u32
romfile_loadbool(const var_na *name, u32 defval)
{
    static const var_na *true_strings[] = { "1", "on", "yes" };
    static const var_na *false_strings[] = { "0", "off", "no" };
    var_na *str = romfile_loadfile(name, NULL);
    ចំនួនគត់ i;

    ប្រសិនបើ (!str)
        ត្រឡប់ defval;

    សម្រាប់ (i = 0; i < ARRAY_SIZE(true_strings); i++)
        ប្រសិនបើ (0 == strcmp(str, true_strings[i]))
            ត្រឡប់ 1;

    សម្រាប់ (i = 0; i < ARRAY_SIZE(false_strings); i++)
        ប្រសិនបើ (0 == strcmp(str, false_strings[i]))
            ត្រឡប់ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ត្រឡប់ defval;
}

sa_nracanaa const_romfile_s {
    sa_nracanaa romfile_s file;
    void *data;
};

static ចំនួនគត់
const_read_file(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    ប្រសិនបើ (file->size > maxlen)
        ត្រឡប់ -1;
    sa_nracanaa const_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa const_romfile_s, file);
    ប្រសិនបើ (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ត្រឡប់ file->size;
}

static void
const_romfile_add(var_na *name, void *data, ចំនួនគត់ size)
{
    sa_nracanaa const_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
    ប្រសិនបើ (!cfile) {
        warn_noalloc();
        ត្រឡប់;
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
    ប្រសិនបើ (!data) {
        warn_noalloc();
        ត្រឡប់;
    }
    *data = value;
    const_romfile_add(name, data, parimaa_na(*data));
}
