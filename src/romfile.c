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

ساكن بنية romfile_s *Romfile_جذر VARVERIFY32INIT;

فراغ
romfile_add(بنية romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_جذر;
    Romfile_جذر = file;
}

// Search for the specified file.
ساكن بنية romfile_s *
__romfile_findprefix(ثابت حرف *prefix, صحيح prefixlen, بنية romfile_s *prev)
{
    بنية romfile_s *cur = Romfile_جذر;
    إذا (prev)
        cur = prev->next;
    طالما (cur) {
        إذا (memcmp(prefix, cur->name, prefixlen) == 0)
            أرجع cur;
        cur = cur->next;
    }
    أرجع عدم;
}

بنية romfile_s *
romfile_findprefix(ثابت حرف *prefix, بنية romfile_s *prev)
{
    أرجع __romfile_findprefix(prefix, strlen(prefix), prev);
}

بنية romfile_s *
romfile_find(ثابت حرف *name)
{
    أرجع __romfile_findprefix(name, strlen(name) + 1, عدم);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
فراغ *
romfile_loadfile(ثابت حرف *name, صحيح *psize)
{
    بنية romfile_s *file = romfile_find(name);
    إذا (!file)
        أرجع عدم;

    صحيح filesize = file->size;
    إذا (!filesize)
        أرجع عدم;

    حرف *data = malloc_tmphigh(filesize+1);
    إذا (!data) {
        warn_noalloc();
        أرجع عدم;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحيح ret = file->copy(file, data, filesize);
    إذا (ret < 0) {
        حرر(data);
        أرجع عدم;
    }
    إذا (psize)
        *psize = filesize;
    data[filesize] = '\0';
    أرجع data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت حرف *name, u64 defval)
{
    بنية romfile_s *file = romfile_find(name);
    إذا (!file)
        أرجع defval;

    صحيح filesize = file->size;
    إذا (!filesize || filesize > حجم(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        أرجع defval;

    u64 val = 0;
    صحيح ret = file->copy(file, &val, حجم(val));
    إذا (ret < 0)
        أرجع defval;
    أرجع val;
}

u32
romfile_loadbool(ثابت حرف *name, u32 defval)
{
    ساكن ثابت حرف *true_strings[] = { "1", "on", "yes" };
    ساكن ثابت حرف *false_strings[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, عدم);
    صحيح i;

    إذا (!str)
        أرجع defval;

    لكل (i = 0; i < ARRAY_SIZE(true_strings); i++)
        إذا (0 == strcmp(str, true_strings[i]))
            أرجع 1;

    لكل (i = 0; i < ARRAY_SIZE(false_strings); i++)
        إذا (0 == strcmp(str, false_strings[i]))
            أرجع 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    أرجع defval;
}

بنية const_romfile_s {
    بنية romfile_s file;
    فراغ *data;
};

ساكن صحيح
const_read_file(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (file->size > maxlen)
        أرجع -1;
    بنية const_romfile_s *cfile;
    cfile = container_of(file, بنية const_romfile_s, file);
    إذا (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    أرجع file->size;
}

ساكن فراغ
const_romfile_add(حرف *name, فراغ *data, صحيح size)
{
    بنية const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    إذا (!cfile) {
        warn_noalloc();
        أرجع;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

فراغ
const_romfile_add_int(حرف *name, u32 قيمة)
{
    u32 *data = malloc_tmp(حجم(*data));
    إذا (!data) {
        warn_noalloc();
        أرجع;
    }
    *data = قيمة;
    const_romfile_add(name, data, حجم(*data));
}
