// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // struct romfile_s
#شامل "string.h" // memcmp

ایستا ساختار romfile_s *Romfile_ریشه VARVERIFY32INIT;

تهی
romfile_add(ساختار romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_ریشه;
    Romfile_ریشه = file;
}

// Search for the specified file.
ایستا ساختار romfile_s *
__romfile_findprefix(ثابت نویسه *prefix, صحیح prefixlen, ساختار romfile_s *prev)
{
    ساختار romfile_s *cur = Romfile_ریشه;
    اگر (prev)
        cur = prev->next;
    وقتی (cur) {
        اگر (memcmp(prefix, cur->name, prefixlen) == 0)
            بازگشت cur;
        cur = cur->next;
    }
    بازگشت پوچ;
}

ساختار romfile_s *
romfile_findprefix(ثابت نویسه *prefix, ساختار romfile_s *prev)
{
    بازگشت __romfile_findprefix(prefix, strlen(prefix), prev);
}

ساختار romfile_s *
romfile_find(ثابت نویسه *name)
{
    بازگشت __romfile_findprefix(name, strlen(name) + 1, پوچ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
تهی *
romfile_loadfile(ثابت نویسه *name, صحیح *psize)
{
    ساختار romfile_s *file = romfile_find(name);
    اگر (!file)
        بازگشت پوچ;

    صحیح filesize = file->size;
    اگر (!filesize)
        بازگشت پوچ;

    نویسه *data = malloc_tmphigh(filesize+1);
    اگر (!data) {
        warn_noalloc();
        بازگشت پوچ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحیح ret = file->copy(file, data, filesize);
    اگر (ret < 0) {
        آزاد(data);
        بازگشت پوچ;
    }
    اگر (psize)
        *psize = filesize;
    data[filesize] = '\0';
    بازگشت data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت نویسه *name, u64 defval)
{
    ساختار romfile_s *file = romfile_find(name);
    اگر (!file)
        بازگشت defval;

    صحیح filesize = file->size;
    اگر (!filesize || filesize > اندازه(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        بازگشت defval;

    u64 val = 0;
    صحیح ret = file->copy(file, &val, اندازه(val));
    اگر (ret < 0)
        بازگشت defval;
    بازگشت val;
}

u32
romfile_loadbool(ثابت نویسه *name, u32 defval)
{
    ایستا ثابت نویسه *true_strings[] = { "1", "on", "yes" };
    ایستا ثابت نویسه *false_strings[] = { "0", "off", "no" };
    نویسه *str = romfile_loadfile(name, پوچ);
    صحیح i;

    اگر (!str)
        بازگشت defval;

    برای (i = 0; i < ARRAY_SIZE(true_strings); i++)
        اگر (0 == strcmp(str, true_strings[i]))
            بازگشت 1;

    برای (i = 0; i < ARRAY_SIZE(false_strings); i++)
        اگر (0 == strcmp(str, false_strings[i]))
            بازگشت 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    بازگشت defval;
}

ساختار const_romfile_s {
    ساختار romfile_s file;
    تهی *data;
};

ایستا صحیح
const_read_file(ساختار romfile_s *file, تهی *dst, u32 maxlen)
{
    اگر (file->size > maxlen)
        بازگشت -1;
    ساختار const_romfile_s *cfile;
    cfile = container_of(file, ساختار const_romfile_s, file);
    اگر (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    بازگشت file->size;
}

ایستا تهی
const_romfile_add(نویسه *name, تهی *data, صحیح size)
{
    ساختار const_romfile_s *cfile = malloc_tmp(اندازه(*cfile));
    اگر (!cfile) {
        warn_noalloc();
        بازگشت;
    }
    memset(cfile, 0, اندازه(*cfile));
    strtcpy(cfile->file.name, name, اندازه(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

تهی
const_romfile_add_int(نویسه *name, u32 مقدار)
{
    u32 *data = malloc_tmp(اندازه(*data));
    اگر (!data) {
        warn_noalloc();
        بازگشت;
    }
    *data = مقدار;
    const_romfile_add(name, data, اندازه(*data));
}
