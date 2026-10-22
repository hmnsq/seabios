// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "romfile.h" // struct romfile_s
#অন্তর্ভাৱ "string.h" // memcmp

স্থির সংরচনা romfile_s *Romfile_মূল VARVERIFY32INIT;

শূন্য
romfile_add(সংরচনা romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_মূল;
    Romfile_মূল = file;
}

// Search for the specified file.
স্থির সংরচনা romfile_s *
__romfile_findprefix(ধ্রুৱ ৱর্ণ *prefix, পূর্ণাঙ্ক prefixlen, সংরচনা romfile_s *prev)
{
    সংরচনা romfile_s *cur = Romfile_মূল;
    যদি (prev)
        cur = prev->next;
    যেতিয়ালৈ (cur) {
        যদি (memcmp(prefix, cur->name, prefixlen) == 0)
            ঘূৰাই cur;
        cur = cur->next;
    }
    ঘূৰাই রিক্ত;
}

সংরচনা romfile_s *
romfile_findprefix(ধ্রুৱ ৱর্ণ *prefix, সংরচনা romfile_s *prev)
{
    ঘূৰাই __romfile_findprefix(prefix, strlen(prefix), prev);
}

সংরচনা romfile_s *
romfile_find(ধ্রুৱ ৱর্ণ *name)
{
    ঘূৰাই __romfile_findprefix(name, strlen(name) + 1, রিক্ত);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
শূন্য *
romfile_loadfile(ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক *psize)
{
    সংরচনা romfile_s *file = romfile_find(name);
    যদি (!file)
        ঘূৰাই রিক্ত;

    পূর্ণাঙ্ক filesize = file->size;
    যদি (!filesize)
        ঘূৰাই রিক্ত;

    ৱর্ণ *data = malloc_tmphigh(filesize+1);
    যদি (!data) {
        warn_noalloc();
        ঘূৰাই রিক্ত;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    পূর্ণাঙ্ক ret = file->copy(file, data, filesize);
    যদি (ret < 0) {
        মুক্ত(data);
        ঘূৰাই রিক্ত;
    }
    যদি (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ঘূৰাই data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ধ্রুৱ ৱর্ণ *name, u64 defval)
{
    সংরচনা romfile_s *file = romfile_find(name);
    যদি (!file)
        ঘূৰাই defval;

    পূর্ণাঙ্ক filesize = file->size;
    যদি (!filesize || filesize > পরিমাণ(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ঘূৰাই defval;

    u64 val = 0;
    পূর্ণাঙ্ক ret = file->copy(file, &val, পরিমাণ(val));
    যদি (ret < 0)
        ঘূৰাই defval;
    ঘূৰাই val;
}

u32
romfile_loadbool(ধ্রুৱ ৱর্ণ *name, u32 defval)
{
    স্থির ধ্রুৱ ৱর্ণ *true_strings[] = { "1", "on", "yes" };
    স্থির ধ্রুৱ ৱর্ণ *false_strings[] = { "0", "off", "no" };
    ৱর্ণ *str = romfile_loadfile(name, রিক্ত);
    পূর্ণাঙ্ক i;

    যদি (!str)
        ঘূৰাই defval;

    বাবে (i = 0; i < ARRAY_SIZE(true_strings); i++)
        যদি (0 == strcmp(str, true_strings[i]))
            ঘূৰাই 1;

    বাবে (i = 0; i < ARRAY_SIZE(false_strings); i++)
        যদি (0 == strcmp(str, false_strings[i]))
            ঘূৰাই 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ঘূৰাই defval;
}

সংরচনা const_romfile_s {
    সংরচনা romfile_s file;
    শূন্য *data;
};

স্থির পূর্ণাঙ্ক
const_read_file(সংরচনা romfile_s *file, শূন্য *dst, u32 maxlen)
{
    যদি (file->size > maxlen)
        ঘূৰাই -1;
    সংরচনা const_romfile_s *cfile;
    cfile = container_of(file, সংরচনা const_romfile_s, file);
    যদি (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ঘূৰাই file->size;
}

স্থির শূন্য
const_romfile_add(ৱর্ণ *name, শূন্য *data, পূর্ণাঙ্ক size)
{
    সংরচনা const_romfile_s *cfile = malloc_tmp(পরিমাণ(*cfile));
    যদি (!cfile) {
        warn_noalloc();
        ঘূৰাই;
    }
    memset(cfile, 0, পরিমাণ(*cfile));
    strtcpy(cfile->file.name, name, পরিমাণ(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

শূন্য
const_romfile_add_int(ৱর্ণ *name, u32 মান)
{
    u32 *data = malloc_tmp(পরিমাণ(*data));
    যদি (!data) {
        warn_noalloc();
        ঘূৰাই;
    }
    *data = মান;
    const_romfile_add(name, data, পরিমাণ(*data));
}
