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

ساكن بنية romfile_s *Romfile_پاڙ VARVERIFY32INIT;

فراغ
romfile_جوڙيو(بنية romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->سائيز);
    file->ايندڙ = Romfile_پاڙ;
    Romfile_پاڙ = file;
}

// Search for the specified file.
ساكن بنية romfile_s *
__romfile_findprefix(ثابت حرف *اڳياڙي, صحيح prefixlen, بنية romfile_s *prev)
{
    بنية romfile_s *cur = Romfile_پاڙ;
    إذا (prev)
        cur = prev->ايندڙ;
    طالما (cur) {
        إذا (memcmp(اڳياڙي, cur->name, prefixlen) == 0)
            أرجع cur;
        cur = cur->ايندڙ;
    }
    أرجع عدم;
}

بنية romfile_s *
romfile_findprefix(ثابت حرف *اڳياڙي, بنية romfile_s *prev)
{
    أرجع __romfile_findprefix(اڳياڙي, لڙي_ڊيگهه(اڳياڙي), prev);
}

بنية romfile_s *
romfile_ڳوليو(ثابت حرف *name)
{
    أرجع __romfile_findprefix(name, لڙي_ڊيگهه(name) + 1, عدم);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
فراغ *
romfile_loadfile(ثابت حرف *name, صحيح *psize)
{
    بنية romfile_s *file = romfile_ڳوليو(name);
    إذا (!file)
        أرجع عدم;

    صحيح filesize = file->سائيز;
    إذا (!filesize)
        أرجع عدم;

    حرف *معلومات = malloc_tmphigh(filesize+1);
    إذا (!معلومات) {
        warn_noalloc();
        أرجع عدم;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحيح ret = file->copy(file, معلومات, filesize);
    إذا (ret < 0) {
        حرر(معلومات);
        أرجع عدم;
    }
    إذا (psize)
        *psize = filesize;
    معلومات[filesize] = '\0';
    أرجع معلومات;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت حرف *name, u64 defval)
{
    بنية romfile_s *file = romfile_ڳوليو(name);
    إذا (!file)
        أرجع defval;

    صحيح filesize = file->سائيز;
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
    ساكن ثابت حرف *true_لڙيون[] = { "1", "on", "yes" };
    ساكن ثابت حرف *false_لڙيون[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, عدم);
    صحيح i;

    إذا (!str)
        أرجع defval;

    لكل (i = 0; i < ARRAY_سائيز(true_لڙيون); i++)
        إذا (0 == strcmp(str, true_لڙيون[i]))
            أرجع 1;

    لكل (i = 0; i < ARRAY_سائيز(false_لڙيون); i++)
        إذا (0 == strcmp(str, false_لڙيون[i]))
            أرجع 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    أرجع defval;
}

بنية const_romfile_s {
    بنية romfile_s file;
    فراغ *معلومات;
};

ساكن صحيح
const_read_file(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (file->سائيز > maxlen)
        أرجع -1;
    بنية const_romfile_s *cfile;
    cfile = container_جو(file, بنية const_romfile_s, file);
    إذا (maxlen > file->سائيز)
        maxlen = file->سائيز;
    يادگيري_نقل(dst, cfile->معلومات, maxlen);
    أرجع file->سائيز;
}

ساكن فراغ
const_romfile_جوڙيو(حرف *name, فراغ *معلومات, صحيح سائيز)
{
    بنية const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    إذا (!cfile) {
        warn_noalloc();
        أرجع;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.سائيز = سائيز;
    cfile->file.copy = const_read_file;
    cfile->معلومات = معلومات;
    romfile_جوڙيو(&cfile->file);
}

فراغ
const_romfile_جوڙيو_int(حرف *name, u32 قدر)
{
    u32 *معلومات = malloc_tmp(حجم(*معلومات));
    إذا (!معلومات) {
        warn_noalloc();
        أرجع;
    }
    *معلومات = قدر;
    const_romfile_جوڙيو(name, معلومات, حجم(*معلومات));
}
