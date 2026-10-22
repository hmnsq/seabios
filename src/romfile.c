// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // struct romfile_s
#অন্তর্ভাব "string.h" // memcmp

স্থির কাঠামো romfile_s *Romfile_মূল VARVERIFY32INIT;

শূন্য
romfile_add(কাঠামো romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->আকার_2);
    file->পরবর্তী = Romfile_মূল;
    Romfile_মূল = file;
}

// Search for the specified file.
স্থির কাঠামো romfile_s *
__romfile_findprefix(ধ্রুবক অক্ষর *prefix, পূর্ণসংখ্যা prefixlen, কাঠামো romfile_s *পূর্ব)
{
    কাঠামো romfile_s *cur = Romfile_মূল;
    যদি (পূর্ব)
        cur = পূর্ব->পরবর্তী;
    যখন (cur) {
        যদি (memcmp(prefix, cur->name, prefixlen) == 0)
            ফেরত cur;
        cur = cur->পরবর্তী;
    }
    ফেরত ফাঁকা;
}

কাঠামো romfile_s *
romfile_findprefix(ধ্রুবক অক্ষর *prefix, কাঠামো romfile_s *পূর্ব)
{
    ফেরত __romfile_findprefix(prefix, strlen(prefix), পূর্ব);
}

কাঠামো romfile_s *
romfile_find(ধ্রুবক অক্ষর *name)
{
    ফেরত __romfile_findprefix(name, strlen(name) + 1, ফাঁকা);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
শূন্য *
romfile_loadfile(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা *psize)
{
    কাঠামো romfile_s *file = romfile_find(name);
    যদি (!file)
        ফেরত ফাঁকা;

    পূর্ণসংখ্যা filesize = file->আকার_2;
    যদি (!filesize)
        ফেরত ফাঁকা;

    অক্ষর *তথ্য = malloc_tmphigh(filesize+1);
    যদি (!তথ্য) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    পূর্ণসংখ্যা ret = file->copy(file, তথ্য, filesize);
    যদি (ret < 0) {
        মুক্ত(তথ্য);
        ফেরত ফাঁকা;
    }
    যদি (psize)
        *psize = filesize;
    তথ্য[filesize] = '\0';
    ফেরত তথ্য;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ধ্রুবক অক্ষর *name, u64 defval)
{
    কাঠামো romfile_s *file = romfile_find(name);
    যদি (!file)
        ফেরত defval;

    পূর্ণসংখ্যা filesize = file->আকার_2;
    যদি (!filesize || filesize > আকার(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ফেরত defval;

    u64 val = 0;
    পূর্ণসংখ্যা ret = file->copy(file, &val, আকার(val));
    যদি (ret < 0)
        ফেরত defval;
    ফেরত val;
}

u32
romfile_loadbool(ধ্রুবক অক্ষর *name, u32 defval)
{
    স্থির ধ্রুবক অক্ষর *true_strings[] = { "1", "on", "yes" };
    স্থির ধ্রুবক অক্ষর *false_strings[] = { "0", "off", "no" };
    অক্ষর *str = romfile_loadfile(name, ফাঁকা);
    পূর্ণসংখ্যা i;

    যদি (!str)
        ফেরত defval;

    জন্য (i = 0; i < ARRAY_আকার(true_strings); i++)
        যদি (0 == strcmp(str, true_strings[i]))
            ফেরত 1;

    জন্য (i = 0; i < ARRAY_আকার(false_strings); i++)
        যদি (0 == strcmp(str, false_strings[i]))
            ফেরত 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ফেরত defval;
}

কাঠামো const_romfile_s {
    কাঠামো romfile_s file;
    শূন্য *তথ্য;
};

স্থির পূর্ণসংখ্যা
const_read_file(কাঠামো romfile_s *file, শূন্য *dst, u32 maxlen)
{
    যদি (file->আকার_2 > maxlen)
        ফেরত -1;
    কাঠামো const_romfile_s *cfile;
    cfile = container_of(file, কাঠামো const_romfile_s, file);
    যদি (maxlen > file->আকার_2)
        maxlen = file->আকার_2;
    memcpy(dst, cfile->তথ্য, maxlen);
    ফেরত file->আকার_2;
}

স্থির শূন্য
const_romfile_add(অক্ষর *name, শূন্য *তথ্য, পূর্ণসংখ্যা আকার_2)
{
    কাঠামো const_romfile_s *cfile = malloc_tmp(আকার(*cfile));
    যদি (!cfile) {
        warn_noalloc();
        ফেরত;
    }
    memset(cfile, 0, আকার(*cfile));
    strtcpy(cfile->file.name, name, আকার(cfile->file.name));
    cfile->file.আকার_2 = আকার_2;
    cfile->file.copy = const_read_file;
    cfile->তথ্য = তথ্য;
    romfile_add(&cfile->file);
}

শূন্য
const_romfile_add_int(অক্ষর *name, u32 মান)
{
    u32 *তথ্য = malloc_tmp(আকার(*তথ্য));
    যদি (!তথ্য) {
        warn_noalloc();
        ফেরত;
    }
    *তথ্য = মান;
    const_romfile_add(name, তথ্য, আকার(*তথ্য));
}
