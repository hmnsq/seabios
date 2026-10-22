// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // struct romfile_s
#ضمن "string.h" // memcmp

ساكن بنية romfile_s *RomfileRoot VARVERIFY32INIT;

فراغ
romfile_add(بنية romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = RomfileRoot;
    RomfileRoot = file;
}

// Search for the specified file.
ساكن بنية romfile_s *
__romfile_findprefix(ثابت حرف *prefix, صحيح prefixlen, بنية romfile_s *prev)
{
    بنية romfile_s *cur = RomfileRoot;
    ܐܢ (prev)
        cur = prev->next;
    ܟܡܐ (cur) {
        ܐܢ (memcmp(prefix, cur->name, prefixlen) == 0)
            ܦܢܐ cur;
        cur = cur->next;
    }
    ܦܢܐ عدم;
}

بنية romfile_s *
romfile_findprefix(ثابت حرف *prefix, بنية romfile_s *prev)
{
    ܦܢܐ __romfile_findprefix(prefix, strlen(prefix), prev);
}

بنية romfile_s *
romfile_find(ثابت حرف *name)
{
    ܦܢܐ __romfile_findprefix(name, strlen(name) + 1, عدم);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
فراغ *
romfile_loadfile(ثابت حرف *name, صحيح *psize)
{
    بنية romfile_s *file = romfile_find(name);
    ܐܢ (!file)
        ܦܢܐ عدم;

    صحيح filesize = file->size;
    ܐܢ (!filesize)
        ܦܢܐ عدم;

    حرف *data = malloc_tmphigh(filesize+1);
    ܐܢ (!data) {
        warn_noalloc();
        ܦܢܐ عدم;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحيح ret = file->copy(file, data, filesize);
    ܐܢ (ret < 0) {
        حرر(data);
        ܦܢܐ عدم;
    }
    ܐܢ (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ܦܢܐ data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت حرف *name, u64 defval)
{
    بنية romfile_s *file = romfile_find(name);
    ܐܢ (!file)
        ܦܢܐ defval;

    صحيح filesize = file->size;
    ܐܢ (!filesize || filesize > حجم(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ܦܢܐ defval;

    u64 val = 0;
    صحيح ret = file->copy(file, &val, حجم(val));
    ܐܢ (ret < 0)
        ܦܢܐ defval;
    ܦܢܐ val;
}

u32
romfile_loadbool(ثابت حرف *name, u32 defval)
{
    ساكن ثابت حرف *true_strings[] = { "1", "on", "yes" };
    ساكن ثابت حرف *false_strings[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, عدم);
    صحيح i;

    ܐܢ (!str)
        ܦܢܐ defval;

    لكل (i = 0; i < ARRAY_SIZE(true_strings); i++)
        ܐܢ (0 == strcmp(str, true_strings[i]))
            ܦܢܐ 1;

    لكل (i = 0; i < ARRAY_SIZE(false_strings); i++)
        ܐܢ (0 == strcmp(str, false_strings[i]))
            ܦܢܐ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ܦܢܐ defval;
}

بنية const_romfile_s {
    بنية romfile_s file;
    فراغ *data;
};

ساكن صحيح
const_read_file(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    ܐܢ (file->size > maxlen)
        ܦܢܐ -1;
    بنية const_romfile_s *cfile;
    cfile = container_of(file, بنية const_romfile_s, file);
    ܐܢ (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ܦܢܐ file->size;
}

ساكن فراغ
const_romfile_add(حرف *name, فراغ *data, صحيح size)
{
    بنية const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    ܐܢ (!cfile) {
        warn_noalloc();
        ܦܢܐ;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

فراغ
const_romfile_add_int(حرف *name, u32 value)
{
    u32 *data = malloc_tmp(حجم(*data));
    ܐܢ (!data) {
        warn_noalloc();
        ܦܢܐ;
    }
    *data = value;
    const_romfile_add(name, data, حجم(*data));
}
