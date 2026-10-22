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
__romfile_findprefix(const char *prefix, ціле prefixlen, struct romfile_s *prev)
{
    struct romfile_s *cur = RomfileRoot;
    якщо (prev)
        cur = prev->next;
    поки (cur) {
        якщо (memcmp(prefix, cur->name, prefixlen) == 0)
            повернути cur;
        cur = cur->next;
    }
    повернути NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    повернути __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    повернути __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, ціле *psize)
{
    struct romfile_s *file = romfile_find(name);
    якщо (!file)
        повернути NULL;

    ціле filesize = file->size;
    якщо (!filesize)
        повернути NULL;

    char *data = malloc_tmphigh(filesize+1);
    якщо (!data) {
        warn_noalloc();
        повернути NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ціле ret = file->copy(file, data, filesize);
    якщо (ret < 0) {
        free(data);
        повернути NULL;
    }
    якщо (psize)
        *psize = filesize;
    data[filesize] = '\0';
    повернути data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    якщо (!file)
        повернути defval;

    ціле filesize = file->size;
    якщо (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        повернути defval;

    u64 val = 0;
    ціле ret = file->copy(file, &val, sizeof(val));
    якщо (ret < 0)
        повернути defval;
    повернути val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    ціле i;

    якщо (!str)
        повернути defval;

    для (i = 0; i < ARRAY_SIZE(true_strings); i++)
        якщо (0 == strcmp(str, true_strings[i]))
            повернути 1;

    для (i = 0; i < ARRAY_SIZE(false_strings); i++)
        якщо (0 == strcmp(str, false_strings[i]))
            повернути 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    повернути defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static ціле
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    якщо (file->size > maxlen)
        повернути -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    якщо (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    повернути file->size;
}

static void
const_romfile_add(char *name, void *data, ціле size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    якщо (!cfile) {
        warn_noalloc();
        повернути;
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
    якщо (!data) {
        warn_noalloc();
        повернути;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
