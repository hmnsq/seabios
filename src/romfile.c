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

ساكن بنية romfile_s *Romfile_ریښه VARVERIFY32INIT;

فراغ
romfile_جمع_کړه(بنية romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->اندازه);
    file->بل = Romfile_ریښه;
    Romfile_ریښه = file;
}

// Search for the specified file.
ساكن بنية romfile_s *
__romfile_findprefix(ثابت حرف *مختاړی, صحيح prefixlen, بنية romfile_s *prev)
{
    بنية romfile_s *cur = Romfile_ریښه;
    إذا (prev)
        cur = prev->بل;
    طالما (cur) {
        إذا (memcmp(مختاړی, cur->name, prefixlen) == 0)
            أرجع cur;
        cur = cur->بل;
    }
    أرجع عدم;
}

بنية romfile_s *
romfile_findprefix(ثابت حرف *مختاړی, بنية romfile_s *prev)
{
    أرجع __romfile_findprefix(مختاړی, مزي_اوږدوالی(مختاړی), prev);
}

بنية romfile_s *
romfile_پیدا_کړه(ثابت حرف *name)
{
    أرجع __romfile_findprefix(name, مزي_اوږدوالی(name) + 1, عدم);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
فراغ *
romfile_loadfile(ثابت حرف *name, صحيح *psize)
{
    بنية romfile_s *file = romfile_پیدا_کړه(name);
    إذا (!file)
        أرجع عدم;

    صحيح filesize = file->اندازه;
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
    بنية romfile_s *file = romfile_پیدا_کړه(name);
    إذا (!file)
        أرجع defval;

    صحيح filesize = file->اندازه;
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
    ساكن ثابت حرف *true_مزي_ګانې[] = { "1", "on", "yes" };
    ساكن ثابت حرف *false_مزي_ګانې[] = { "0", "off", "no" };
    حرف *str = romfile_loadfile(name, عدم);
    صحيح i;

    إذا (!str)
        أرجع defval;

    لكل (i = 0; i < ARRAY_اندازه(true_مزي_ګانې); i++)
        إذا (0 == strcmp(str, true_مزي_ګانې[i]))
            أرجع 1;

    لكل (i = 0; i < ARRAY_اندازه(false_مزي_ګانې); i++)
        إذا (0 == strcmp(str, false_مزي_ګانې[i]))
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
    إذا (file->اندازه > maxlen)
        أرجع -1;
    بنية const_romfile_s *cfile;
    cfile = container_د(file, بنية const_romfile_s, file);
    إذا (maxlen > file->اندازه)
        maxlen = file->اندازه;
    حافظه_نقل(dst, cfile->معلومات, maxlen);
    أرجع file->اندازه;
}

ساكن فراغ
const_romfile_جمع_کړه(حرف *name, فراغ *معلومات, صحيح اندازه)
{
    بنية const_romfile_s *cfile = malloc_tmp(حجم(*cfile));
    إذا (!cfile) {
        warn_noalloc();
        أرجع;
    }
    memset(cfile, 0, حجم(*cfile));
    strtcpy(cfile->file.name, name, حجم(cfile->file.name));
    cfile->file.اندازه = اندازه;
    cfile->file.copy = const_read_file;
    cfile->معلومات = معلومات;
    romfile_جمع_کړه(&cfile->file);
}

فراغ
const_romfile_جمع_کړه_int(حرف *name, u32 ارزښت)
{
    u32 *معلومات = malloc_tmp(حجم(*معلومات));
    إذا (!معلومات) {
        warn_noalloc();
        أرجع;
    }
    *معلومات = ارزښت;
    const_romfile_جمع_کړه(name, معلومات, حجم(*معلومات));
}
