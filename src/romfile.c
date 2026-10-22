// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // struct romfile_s
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcmp

ਸਥਿਰ ਬਣਤਰ romfile_s *Romfile_ਜੜ੍ਹ VARVERIFY32INIT;

ਖਾਲੀ
romfile_ਜੋੜੋ(ਬਣਤਰ romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->ਮਾਪ);
    file->ਅਗਲਾ = Romfile_ਜੜ੍ਹ;
    Romfile_ਜੜ੍ਹ = file;
}

// Search for the specified file.
ਸਥਿਰ ਬਣਤਰ romfile_s *
__romfile_findprefix(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਪੂਰਨ prefixlen, ਬਣਤਰ romfile_s *prev)
{
    ਬਣਤਰ romfile_s *cur = Romfile_ਜੜ੍ਹ;
    ਜੇ (prev)
        cur = prev->ਅਗਲਾ;
    ਜਦੋਂ (cur) {
        ਜੇ (memcmp(ਅਗੇਤਰ, cur->name, prefixlen) == 0)
            ਵਾਪਸ cur;
        cur = cur->ਅਗਲਾ;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ romfile_s *
romfile_findprefix(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਬਣਤਰ romfile_s *prev)
{
    ਵਾਪਸ __romfile_findprefix(ਅਗੇਤਰ, ਲੜੀ_ਲੰਬਾਈ(ਅਗੇਤਰ), prev);
}

ਬਣਤਰ romfile_s *
romfile_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *name)
{
    ਵਾਪਸ __romfile_findprefix(name, ਲੜੀ_ਲੰਬਾਈ(name) + 1, ਨਿਰਾ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
ਖਾਲੀ *
romfile_loadfile(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ *psize)
{
    ਬਣਤਰ romfile_s *file = romfile_ਲੱਭੋ(name);
    ਜੇ (!file)
        ਵਾਪਸ ਨਿਰਾ;

    ਪੂਰਨ filesize = file->ਮਾਪ;
    ਜੇ (!filesize)
        ਵਾਪਸ ਨਿਰਾ;

    ਅੱਖਰ *ਅੰਕੜੇ = malloc_tmphigh(filesize+1);
    ਜੇ (!ਅੰਕੜੇ) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ਪੂਰਨ ret = file->copy(file, ਅੰਕੜੇ, filesize);
    ਜੇ (ret < 0) {
        ਮੁਕਤ(ਅੰਕੜੇ);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਜੇ (psize)
        *psize = filesize;
    ਅੰਕੜੇ[filesize] = '\0';
    ਵਾਪਸ ਅੰਕੜੇ;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ਅਚਲ ਅੱਖਰ *name, u64 defval)
{
    ਬਣਤਰ romfile_s *file = romfile_ਲੱਭੋ(name);
    ਜੇ (!file)
        ਵਾਪਸ defval;

    ਪੂਰਨ filesize = file->ਮਾਪ;
    ਜੇ (!filesize || filesize > ਆਕਾਰ(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ਵਾਪਸ defval;

    u64 val = 0;
    ਪੂਰਨ ret = file->copy(file, &val, ਆਕਾਰ(val));
    ਜੇ (ret < 0)
        ਵਾਪਸ defval;
    ਵਾਪਸ val;
}

u32
romfile_loadbool(ਅਚਲ ਅੱਖਰ *name, u32 defval)
{
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *true_ਲੜੀਆਂ[] = { "1", "on", "yes" };
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *false_ਲੜੀਆਂ[] = { "0", "off", "no" };
    ਅੱਖਰ *str = romfile_loadfile(name, ਨਿਰਾ);
    ਪੂਰਨ i;

    ਜੇ (!str)
        ਵਾਪਸ defval;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(true_ਲੜੀਆਂ); i++)
        ਜੇ (0 == strcmp(str, true_ਲੜੀਆਂ[i]))
            ਵਾਪਸ 1;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(false_ਲੜੀਆਂ); i++)
        ਜੇ (0 == strcmp(str, false_ਲੜੀਆਂ[i]))
            ਵਾਪਸ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ਵਾਪਸ defval;
}

ਬਣਤਰ const_romfile_s {
    ਬਣਤਰ romfile_s file;
    ਖਾਲੀ *ਅੰਕੜੇ;
};

ਸਥਿਰ ਪੂਰਨ
const_read_file(ਬਣਤਰ romfile_s *file, ਖਾਲੀ *dst, u32 maxlen)
{
    ਜੇ (file->ਮਾਪ > maxlen)
        ਵਾਪਸ -1;
    ਬਣਤਰ const_romfile_s *cfile;
    cfile = container_ਦਾ(file, ਬਣਤਰ const_romfile_s, file);
    ਜੇ (maxlen > file->ਮਾਪ)
        maxlen = file->ਮਾਪ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(dst, cfile->ਅੰਕੜੇ, maxlen);
    ਵਾਪਸ file->ਮਾਪ;
}

ਸਥਿਰ ਖਾਲੀ
const_romfile_ਜੋੜੋ(ਅੱਖਰ *name, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ)
{
    ਬਣਤਰ const_romfile_s *cfile = malloc_tmp(ਆਕਾਰ(*cfile));
    ਜੇ (!cfile) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(cfile, 0, ਆਕਾਰ(*cfile));
    strtcpy(cfile->file.name, name, ਆਕਾਰ(cfile->file.name));
    cfile->file.ਮਾਪ = ਮਾਪ;
    cfile->file.copy = const_read_file;
    cfile->ਅੰਕੜੇ = ਅੰਕੜੇ;
    romfile_ਜੋੜੋ(&cfile->file);
}

ਖਾਲੀ
const_romfile_ਜੋੜੋ_int(ਅੱਖਰ *name, u32 ਮੁੱਲ)
{
    u32 *ਅੰਕੜੇ = malloc_tmp(ਆਕਾਰ(*ਅੰਕੜੇ));
    ਜੇ (!ਅੰਕੜੇ) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    *ਅੰਕੜੇ = ਮੁੱਲ;
    const_romfile_ਜੋੜੋ(name, ਅੰਕੜੇ, ਆਕਾਰ(*ਅੰਕੜੇ));
}
