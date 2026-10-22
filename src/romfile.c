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

ساكن بنية romfile_s *Romfile_جڑ VARVERIFY32INIT;

فراغ
romfile_جوڑو(بنية romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->ناپ);
    file->اگلا = Romfile_جڑ;
    Romfile_جڑ = file;
}

// Search for the specified file.
ساكن بنية romfile_s *
__romfile_findprefix(ثابت حرف *اگیتر, صحيح prefixlen, بنية romfile_s *prev)
{
    بنية romfile_s *cur = Romfile_جڑ;
    إذا (prev)
        cur = prev->اگلا;
    طالما (cur) {
        إذا (memcmp(اگیتر, cur->name, prefixlen) == 0)
            أرجع cur;
        cur = cur->اگلا;
    }
    أرجع عدم;
}

بنية romfile_s *
romfile_findprefix(ثابت حرف *اگیتر, بنية romfile_s *prev)
{
    أرجع __romfile_findprefix(اگیتر, لڑی_لمبائی(اگیتر), prev);
}

بنية romfile_s *
romfile_لبھو(ثابت حرف *name)
{
    أرجع __romfile_findprefix(name, لڑی_لمبائی(name) + 1, عدم);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
فراغ *
romfile_loadfile(ثابت حرف *name, صحيح *psize)
{
    بنية romfile_s *file = romfile_لبھو(name);
    إذا (!file)
        أرجع عدم;

    صحيح filesize = file->ناپ;
    إذا (!filesize)
        أرجع عدم;

    حرف *انکڑے = malloc_tmphigh(filesize+1);
    إذا (!انکڑے) {
        warn_noalloc();
        أرجع عدم;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحيح ret = file->copy(file, انکڑے, filesize);
    إذا (ret < 0) {
        حرر(انکڑے);
        أرجع عدم;
    }
    إذا (psize)
        *psize = filesize;
    انکڑے[filesize] = '\0';
    أرجع انکڑے;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت حرف *name, u64 defval)
{
    بنية romfile_s *file = romfile_لبھو(name);
    إذا (!file)
        أرجع defval;

    صحيح filesize = file->ناپ;
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
    ساكن ثابت حرف *true_لڑیاں[] = { "1", "on", "yes" };
    ساكن ثابت حرف *false_لڑیاں[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, عدم);
    صحيح i;

    إذا (!str)
        أرجع defval;

    لكل (i = 0; i < ARRAY_ناپ(true_لڑیاں); i++)
        إذا (0 == strcmp(str, true_لڑیاں[i]))
            أرجع 1;

    لكل (i = 0; i < ARRAY_ناپ(false_لڑیاں); i++)
        إذا (0 == strcmp(str, false_لڑیاں[i]))
            أرجع 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    أرجع defval;
}

بنية const_romfile_s {
    بنية romfile_s file;
    فراغ *انکڑے;
};

ساكن صحيح
const_read_file(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (file->ناپ > maxlen)
        أرجع -1;
    بنية const_romfile_s *cfile;
    cfile = container_دا(file, بنية const_romfile_s, file);
    إذا (maxlen > file->ناپ)
        maxlen = file->ناپ;
    حافظہ_نقل(dst, cfile->انکڑے, maxlen);
    أرجع file->ناپ;
}

ساكن فراغ
const_romfile_جوڑو(حرف *name, فراغ *انکڑے, صحيح ناپ)
{
    بنية const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    إذا (!cfile) {
        warn_noalloc();
        أرجع;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.ناپ = ناپ;
    cfile->file.copy = const_read_file;
    cfile->انکڑے = انکڑے;
    romfile_جوڑو(&cfile->file);
}

فراغ
const_romfile_جوڑو_int(حرف *name, u32 قدر)
{
    u32 *انکڑے = malloc_tmp(حجم(*انکڑے));
    إذا (!انکڑے) {
        warn_noalloc();
        أرجع;
    }
    *انکڑے = قدر;
    const_romfile_جوڑو(name, انکڑے, حجم(*انکڑے));
}
