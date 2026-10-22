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
__romfile_findprefix(const char *prefix, 整数 prefixlen, struct romfile_s *prev)
{
    struct romfile_s *cur = RomfileRoot;
    如果 (prev)
        cur = prev->next;
    当 (cur) {
        如果 (memcmp(prefix, cur->name, prefixlen) == 0)
            返回 cur;
        cur = cur->next;
    }
    返回 NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    返回 __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    返回 __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, 整数 *psize)
{
    struct romfile_s *file = romfile_find(name);
    如果 (!file)
        返回 NULL;

    整数 filesize = file->size;
    如果 (!filesize)
        返回 NULL;

    char *data = malloc_tmphigh(filesize+1);
    如果 (!data) {
        warn_noalloc();
        返回 NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    整数 ret = file->copy(file, data, filesize);
    如果 (ret < 0) {
        free(data);
        返回 NULL;
    }
    如果 (psize)
        *psize = filesize;
    data[filesize] = '\0';
    返回 data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    如果 (!file)
        返回 defval;

    整数 filesize = file->size;
    如果 (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        返回 defval;

    u64 val = 0;
    整数 ret = file->copy(file, &val, sizeof(val));
    如果 (ret < 0)
        返回 defval;
    返回 val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    整数 i;

    如果 (!str)
        返回 defval;

    为 (i = 0; i < ARRAY_SIZE(true_strings); i++)
        如果 (0 == strcmp(str, true_strings[i]))
            返回 1;

    为 (i = 0; i < ARRAY_SIZE(false_strings); i++)
        如果 (0 == strcmp(str, false_strings[i]))
            返回 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    返回 defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static 整数
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    如果 (file->size > maxlen)
        返回 -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    如果 (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    返回 file->size;
}

static void
const_romfile_add(char *name, void *data, 整数 size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    如果 (!cfile) {
        warn_noalloc();
        返回;
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
    如果 (!data) {
        warn_noalloc();
        返回;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
