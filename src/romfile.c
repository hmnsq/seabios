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
__romfile_findprefix(const char *prefix, целое prefixlen, struct romfile_s *prev)
{
    struct romfile_s *cur = RomfileRoot;
    если (prev)
        cur = prev->next;
    пока (cur) {
        если (memcmp(prefix, cur->name, prefixlen) == 0)
            вернуть cur;
        cur = cur->next;
    }
    вернуть NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    вернуть __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    вернуть __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, целое *psize)
{
    struct romfile_s *file = romfile_find(name);
    если (!file)
        вернуть NULL;

    целое filesize = file->size;
    если (!filesize)
        вернуть NULL;

    char *data = malloc_tmphigh(filesize+1);
    если (!data) {
        warn_noalloc();
        вернуть NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    целое ret = file->copy(file, data, filesize);
    если (ret < 0) {
        free(data);
        вернуть NULL;
    }
    если (psize)
        *psize = filesize;
    data[filesize] = '\0';
    вернуть data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    если (!file)
        вернуть defval;

    целое filesize = file->size;
    если (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        вернуть defval;

    u64 val = 0;
    целое ret = file->copy(file, &val, sizeof(val));
    если (ret < 0)
        вернуть defval;
    вернуть val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    целое i;

    если (!str)
        вернуть defval;

    для (i = 0; i < ARRAY_SIZE(true_strings); i++)
        если (0 == strcmp(str, true_strings[i]))
            вернуть 1;

    для (i = 0; i < ARRAY_SIZE(false_strings); i++)
        если (0 == strcmp(str, false_strings[i]))
            вернуть 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    вернуть defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static целое
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    если (file->size > maxlen)
        вернуть -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    если (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    вернуть file->size;
}

static void
const_romfile_add(char *name, void *data, целое size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    если (!cfile) {
        warn_noalloc();
        вернуть;
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
    если (!data) {
        warn_noalloc();
        вернуть;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
