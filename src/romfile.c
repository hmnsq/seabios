// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // struct romfile_s
#അന്തര്ഭാവ "string.h" // memcmp

സ്ഥിതം ഘടന romfile_s *Romfile_വേര് VARVERIFY32INIT;

ശൂന്യം
romfile_add(ഘടന romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_വേര്;
    Romfile_വേര് = file;
}

// Search for the specified file.
സ്ഥിതം ഘടന romfile_s *
__romfile_findprefix(സ്ഥിരം അക്ഷരം *prefix, പൂർണ്ണസംഖ്യ prefixlen, ഘടന romfile_s *prev)
{
    ഘടന romfile_s *cur = Romfile_വേര്;
    എങ്കിൽ (prev)
        cur = prev->next;
    ആകുമ്പോൾ (cur) {
        എങ്കിൽ (memcmp(prefix, cur->name, prefixlen) == 0)
            തിരികെ cur;
        cur = cur->next;
    }
    തിരികെ ശൂന്യ;
}

ഘടന romfile_s *
romfile_findprefix(സ്ഥിരം അക്ഷരം *prefix, ഘടന romfile_s *prev)
{
    തിരികെ __romfile_findprefix(prefix, strlen(prefix), prev);
}

ഘടന romfile_s *
romfile_find(സ്ഥിരം അക്ഷരം *name)
{
    തിരികെ __romfile_findprefix(name, strlen(name) + 1, ശൂന്യ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
ശൂന്യം *
romfile_loadfile(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ *psize)
{
    ഘടന romfile_s *file = romfile_find(name);
    എങ്കിൽ (!file)
        തിരികെ ശൂന്യ;

    പൂർണ്ണസംഖ്യ filesize = file->size;
    എങ്കിൽ (!filesize)
        തിരികെ ശൂന്യ;

    അക്ഷരം *data = malloc_tmphigh(filesize+1);
    എങ്കിൽ (!data) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    പൂർണ്ണസംഖ്യ ret = file->copy(file, data, filesize);
    എങ്കിൽ (ret < 0) {
        സ്വതന്ത്രം(data);
        തിരികെ ശൂന്യ;
    }
    എങ്കിൽ (psize)
        *psize = filesize;
    data[filesize] = '\0';
    തിരികെ data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(സ്ഥിരം അക്ഷരം *name, u64 defval)
{
    ഘടന romfile_s *file = romfile_find(name);
    എങ്കിൽ (!file)
        തിരികെ defval;

    പൂർണ്ണസംഖ്യ filesize = file->size;
    എങ്കിൽ (!filesize || filesize > വലുപ്പം(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        തിരികെ defval;

    u64 val = 0;
    പൂർണ്ണസംഖ്യ ret = file->copy(file, &val, വലുപ്പം(val));
    എങ്കിൽ (ret < 0)
        തിരികെ defval;
    തിരികെ val;
}

u32
romfile_loadbool(സ്ഥിരം അക്ഷരം *name, u32 defval)
{
    സ്ഥിതം സ്ഥിരം അക്ഷരം *true_strings[] = { "1", "on", "yes" };
    സ്ഥിതം സ്ഥിരം അക്ഷരം *false_strings[] = { "0", "off", "no" };
    അക്ഷരം *str = romfile_loadfile(name, ശൂന്യ);
    പൂർണ്ണസംഖ്യ i;

    എങ്കിൽ (!str)
        തിരികെ defval;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(true_strings); i++)
        എങ്കിൽ (0 == strcmp(str, true_strings[i]))
            തിരികെ 1;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(false_strings); i++)
        എങ്കിൽ (0 == strcmp(str, false_strings[i]))
            തിരികെ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    തിരികെ defval;
}

ഘടന const_romfile_s {
    ഘടന romfile_s file;
    ശൂന്യം *data;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ
const_read_file(ഘടന romfile_s *file, ശൂന്യം *dst, u32 maxlen)
{
    എങ്കിൽ (file->size > maxlen)
        തിരികെ -1;
    ഘടന const_romfile_s *cfile;
    cfile = container_of(file, ഘടന const_romfile_s, file);
    എങ്കിൽ (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    തിരികെ file->size;
}

സ്ഥിതം ശൂന്യം
const_romfile_add(അക്ഷരം *name, ശൂന്യം *data, പൂർണ്ണസംഖ്യ size)
{
    ഘടന const_romfile_s *cfile = malloc_tmp(വലുപ്പം(*cfile));
    എങ്കിൽ (!cfile) {
        warn_noalloc();
        തിരികെ;
    }
    memset(cfile, 0, വലുപ്പം(*cfile));
    strtcpy(cfile->file.name, name, വലുപ്പം(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

ശൂന്യം
const_romfile_add_int(അക്ഷരം *name, u32 മൂല്യം)
{
    u32 *data = malloc_tmp(വലുപ്പം(*data));
    എങ്കിൽ (!data) {
        warn_noalloc();
        തിരികെ;
    }
    *data = മൂല്യം;
    const_romfile_add(name, data, വലുപ്പം(*data));
}
