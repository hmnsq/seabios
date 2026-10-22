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
__romfile_findprefix(const char *prefix, 정수 prefixlen, struct romfile_s *prev)
{
    struct romfile_s *cur = RomfileRoot;
    만약 (prev)
        cur = prev->next;
    동안 (cur) {
        만약 (memcmp(prefix, cur->name, prefixlen) == 0)
            반환 cur;
        cur = cur->next;
    }
    반환 NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    반환 __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    반환 __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, 정수 *psize)
{
    struct romfile_s *file = romfile_find(name);
    만약 (!file)
        반환 NULL;

    정수 filesize = file->size;
    만약 (!filesize)
        반환 NULL;

    char *data = malloc_tmphigh(filesize+1);
    만약 (!data) {
        warn_noalloc();
        반환 NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    정수 ret = file->copy(file, data, filesize);
    만약 (ret < 0) {
        free(data);
        반환 NULL;
    }
    만약 (psize)
        *psize = filesize;
    data[filesize] = '\0';
    반환 data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    만약 (!file)
        반환 defval;

    정수 filesize = file->size;
    만약 (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        반환 defval;

    u64 val = 0;
    정수 ret = file->copy(file, &val, sizeof(val));
    만약 (ret < 0)
        반환 defval;
    반환 val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    정수 i;

    만약 (!str)
        반환 defval;

    위해 (i = 0; i < ARRAY_SIZE(true_strings); i++)
        만약 (0 == strcmp(str, true_strings[i]))
            반환 1;

    위해 (i = 0; i < ARRAY_SIZE(false_strings); i++)
        만약 (0 == strcmp(str, false_strings[i]))
            반환 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    반환 defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static 정수
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    만약 (file->size > maxlen)
        반환 -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    만약 (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    반환 file->size;
}

static void
const_romfile_add(char *name, void *data, 정수 size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    만약 (!cfile) {
        warn_noalloc();
        반환;
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
    만약 (!data) {
        warn_noalloc();
        반환;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
