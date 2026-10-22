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

ساکن ساخت romfile_s *Romfile_جڑ VARVERIFY32INIT;

خالی
romfile_جوڑو(ساخت romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->سائز);
    file->اگلا = Romfile_جڑ;
    Romfile_جڑ = file;
}

// Search for the specified file.
ساکن ساخت romfile_s *
__romfile_findprefix(ثابت حرف *سابقہ, صحیح prefixlen, ساخت romfile_s *پچھلا)
{
    ساخت romfile_s *cur = Romfile_جڑ;
    اگر (پچھلا)
        cur = پچھلا->اگلا;
    جب (cur) {
        اگر (memcmp(سابقہ, cur->name, prefixlen) == 0)
            واپس cur;
        cur = cur->اگلا;
    }
    واپس تہی;
}

ساخت romfile_s *
romfile_findprefix(ثابت حرف *سابقہ, ساخت romfile_s *پچھلا)
{
    واپس __romfile_findprefix(سابقہ, لڑی_لمبائی(سابقہ), پچھلا);
}

ساخت romfile_s *
romfile_ڈھونڈو(ثابت حرف *name)
{
    واپس __romfile_findprefix(name, لڑی_لمبائی(name) + 1, تہی);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
خالی *
romfile_loadfile(ثابت حرف *name, صحیح *psize)
{
    ساخت romfile_s *file = romfile_ڈھونڈو(name);
    اگر (!file)
        واپس تہی;

    صحیح filesize = file->سائز;
    اگر (!filesize)
        واپس تہی;

    حرف *ڈیٹا = malloc_tmphigh(filesize+1);
    اگر (!ڈیٹا) {
        warn_noalloc();
        واپس تہی;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    صحیح ret = file->copy(file, ڈیٹا, filesize);
    اگر (ret < 0) {
        آزاد(ڈیٹا);
        واپس تہی;
    }
    اگر (psize)
        *psize = filesize;
    ڈیٹا[filesize] = '\0';
    واپس ڈیٹا;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ثابت حرف *name, u64 defval)
{
    ساخت romfile_s *file = romfile_ڈھونڈو(name);
    اگر (!file)
        واپس defval;

    صحیح filesize = file->سائز;
    اگر (!filesize || filesize > حجم(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        واپس defval;

    u64 val = 0;
    صحیح ret = file->copy(file, &val, حجم(val));
    اگر (ret < 0)
        واپس defval;
    واپس val;
}

u32
romfile_loadbool(ثابت حرف *name, u32 defval)
{
    ساکن ثابت حرف *true_لڑیاں[] = { "1", "on", "yes" };
    ساکن ثابت حرف *false_لڑیاں[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, تہی);
    صحیح i;

    اگر (!str)
        واپس defval;

    برائے (i = 0; i < ARRAY_سائز(true_لڑیاں); i++)
        اگر (0 == strcmp(str, true_لڑیاں[i]))
            واپس 1;

    برائے (i = 0; i < ARRAY_سائز(false_لڑیاں); i++)
        اگر (0 == strcmp(str, false_لڑیاں[i]))
            واپس 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    واپس defval;
}

ساخت const_romfile_s {
    ساخت romfile_s file;
    خالی *ڈیٹا;
};

ساکن صحیح
const_read_file(ساخت romfile_s *file, خالی *dst, u32 maxlen)
{
    اگر (file->سائز > maxlen)
        واپس -1;
    ساخت const_romfile_s *cfile;
    cfile = container_کا(file, ساخت const_romfile_s, file);
    اگر (maxlen > file->سائز)
        maxlen = file->سائز;
    حافظہ_نقل(dst, cfile->ڈیٹا, maxlen);
    واپس file->سائز;
}

ساکن خالی
const_romfile_جوڑو(حرف *name, خالی *ڈیٹا, صحیح سائز)
{
    ساخت const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    اگر (!cfile) {
        warn_noalloc();
        واپس;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.سائز = سائز;
    cfile->file.copy = const_read_file;
    cfile->ڈیٹا = ڈیٹا;
    romfile_جوڑو(&cfile->file);
}

خالی
const_romfile_جوڑو_int(حرف *name, u32 قدر)
{
    u32 *ڈیٹا = malloc_tmp(حجم(*ڈیٹا));
    اگر (!ڈیٹا) {
        warn_noalloc();
        واپس;
    }
    *ڈیٹا = قدر;
    const_romfile_جوڑو(name, ڈیٹا, حجم(*ڈیٹا));
}
