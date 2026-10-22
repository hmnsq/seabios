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
    もし (prev)
        cur = prev->next;
    ながら (cur) {
        もし (memcmp(prefix, cur->name, prefixlen) == 0)
            戻る cur;
        cur = cur->next;
    }
    戻る NULL;
}

struct romfile_s *
romfile_findprefix(const char *prefix, struct romfile_s *prev)
{
    戻る __romfile_findprefix(prefix, strlen(prefix), prev);
}

struct romfile_s *
romfile_find(const char *name)
{
    戻る __romfile_findprefix(name, strlen(name) + 1, NULL);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
void *
romfile_loadfile(const char *name, 整数 *psize)
{
    struct romfile_s *file = romfile_find(name);
    もし (!file)
        戻る NULL;

    整数 filesize = file->size;
    もし (!filesize)
        戻る NULL;

    char *data = malloc_tmphigh(filesize+1);
    もし (!data) {
        warn_noalloc();
        戻る NULL;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    整数 ret = file->copy(file, data, filesize);
    もし (ret < 0) {
        free(data);
        戻る NULL;
    }
    もし (psize)
        *psize = filesize;
    data[filesize] = '\0';
    戻る data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(const char *name, u64 defval)
{
    struct romfile_s *file = romfile_find(name);
    もし (!file)
        戻る defval;

    整数 filesize = file->size;
    もし (!filesize || filesize > sizeof(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        戻る defval;

    u64 val = 0;
    整数 ret = file->copy(file, &val, sizeof(val));
    もし (ret < 0)
        戻る defval;
    戻る val;
}

u32
romfile_loadbool(const char *name, u32 defval)
{
    static const char *true_strings[] = { "1", "on", "yes" };
    static const char *false_strings[] = { "0", "off", "no" };
    char *str = romfile_loadfile(name, NULL);
    整数 i;

    もし (!str)
        戻る defval;

    ために (i = 0; i < ARRAY_SIZE(true_strings); i++)
        もし (0 == strcmp(str, true_strings[i]))
            戻る 1;

    ために (i = 0; i < ARRAY_SIZE(false_strings); i++)
        もし (0 == strcmp(str, false_strings[i]))
            戻る 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    戻る defval;
}

struct const_romfile_s {
    struct romfile_s file;
    void *data;
};

static 整数
const_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    もし (file->size > maxlen)
        戻る -1;
    struct const_romfile_s *cfile;
    cfile = container_of(file, struct const_romfile_s, file);
    もし (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    戻る file->size;
}

static void
const_romfile_add(char *name, void *data, 整数 size)
{
    struct const_romfile_s *cfile = malloc_tmp(sizeof(*cfile));
    もし (!cfile) {
        warn_noalloc();
        戻る;
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
    もし (!data) {
        warn_noalloc();
        戻る;
    }
    *data = value;
    const_romfile_add(name, data, sizeof(*data));
}
