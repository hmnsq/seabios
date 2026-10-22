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

static struct romfile_s *RomfileRoot VARVERIFY32INIT;

void
romfile_add(struct romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = RomfileRoot;
    RomfileRoot = file;
}

// Search for the specified file.
static struct romfile_s *
__romfile_findprefix(const char *prefix, số_nguyên prefixlen, struct romfile_s *prev)
{
    struct romfile_s *cur = RomfileRoot;
    nếu (prev)
        cur = prev->next;
    trong_khi (cur) {
        nếu (memcmp(prefix, cur->name, prefixlen) == 0)
            trả_về cur;
        cur = cur->next;
    }
    trả_về NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    trả_về __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    trả_về __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, số_nguyên *psize)
{
    struct romfile_s *file = romfile_find(name);
    nếu (!file)
        trả_về NULL;

    số_nguyên filesize = file->size;
    nếu (!filesize)
        trả_về NULL;

    char *data = malloc_tmphigh(filesize+1);
    nếu (!data) {
        warn_noalloc();
        trả_về NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    số_nguyên ret = file->copy(file, data, filesize);
    nếu (ret < 0) {
        free(data);
        trả_về NULL;
    }
    nếu (psize)
        *psize = filesize;
    data[filesize] = '\0';
    trả_về data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    nếu (!file)
        trả_về defval;

    số_nguyên filesize = file->size;
    nếu (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        trả_về defval;

    u64 val = 0;
    số_nguyên ret = file->copy(file, &val, sizeof(val));
    nếu (ret < 0)
        trả_về defval;
    trả_về val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    số_nguyên i;

    nếu (!str)
        trả_về defval;

    cho (i = 0; i < ARRAY_SIZE(true_strings); i++)
        nếu (0 == strcmp(str, true_strings[i]))
            trả_về 1;

    cho (i = 0; i < ARRAY_SIZE(false_strings); i++)
        nếu (0 == strcmp(str, false_strings[i]))
            trả_về 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    trả_về defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static số_nguyên
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    nếu (file->size > maxlen)
        trả_về -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    nếu (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    trả_về file->size;
}

static void
const_romfile_add(char *name, void *data, số_nguyên size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    nếu (!cfile) {
        warn_noalloc();
        trả_về;
    }
    memset(cfile, 0, sizeof(*cfile));
    strtcpy(cfile->file.name, name, sizeof(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

void
const_romfile_add_int(char *name, u32 value)
{
    u32 *data = malloc_tmp(sizeof(*data));
    nếu (!data) {
        warn_noalloc();
        trả_về;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
