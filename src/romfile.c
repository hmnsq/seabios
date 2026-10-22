// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // struct romfile_s
#અન્તર્ભાવ "string.h" // memcmp

સ્થિર સંરચના romfile_s *Romfile_મૂળ VARVERIFY32INIT;

શૂન્ય
romfile_add(સંરચના romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_મૂળ;
    Romfile_મૂળ = file;
}

// Search for the specified file.
સ્થિર સંરચના romfile_s *
__romfile_findprefix(અચલ અક્ષર *prefix, પૂર્ણાંક prefixlen, સંરચના romfile_s *prev)
{
    સંરચના romfile_s *cur = Romfile_મૂળ;
    જો (prev)
        cur = prev->next;
    જ્યારે (cur) {
        જો (memcmp(prefix, cur->name, prefixlen) == 0)
            પાછા cur;
        cur = cur->next;
    }
    પાછા ખાલી;
}

સંરચના romfile_s *
romfile_findprefix(અચલ અક્ષર *prefix, સંરચના romfile_s *prev)
{
    પાછા __romfile_findprefix(prefix, strlen(prefix), prev);
}

સંરચના romfile_s *
romfile_find(અચલ અક્ષર *name)
{
    પાછા __romfile_findprefix(name, strlen(name) + 1, ખાલી);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
શૂન્ય *
romfile_loadfile(અચલ અક્ષર *name, પૂર્ણાંક *psize)
{
    સંરચના romfile_s *file = romfile_find(name);
    જો (!file)
        પાછા ખાલી;

    પૂર્ણાંક filesize = file->size;
    જો (!filesize)
        પાછા ખાલી;

    અક્ષર *data = malloc_tmphigh(filesize+1);
    જો (!data) {
        warn_noalloc();
        પાછા ખાલી;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    પૂર્ણાંક ret = file->copy(file, data, filesize);
    જો (ret < 0) {
        મુક્ત(data);
        પાછા ખાલી;
    }
    જો (psize)
        *psize = filesize;
    data[filesize] = '\0';
    પાછા data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(અચલ અક્ષર *name, u64 defval)
{
    સંરચના romfile_s *file = romfile_find(name);
    જો (!file)
        પાછા defval;

    પૂર્ણાંક filesize = file->size;
    જો (!filesize || filesize > કદ(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        પાછા defval;

    u64 val = 0;
    પૂર્ણાંક ret = file->copy(file, &val, કદ(val));
    જો (ret < 0)
        પાછા defval;
    પાછા val;
}

u32
romfile_loadbool(અચલ અક્ષર *name, u32 defval)
{
    સ્થિર અચલ અક્ષર *true_strings[] = { "1", "on", "yes" };
    સ્થિર અચલ અક્ષર *false_strings[] = { "0", "off", "no" };
    અક્ષર *str = romfile_loadfile(name, ખાલી);
    પૂર્ણાંક i;

    જો (!str)
        પાછા defval;

    માટે (i = 0; i < ARRAY_SIZE(true_strings); i++)
        જો (0 == strcmp(str, true_strings[i]))
            પાછા 1;

    માટે (i = 0; i < ARRAY_SIZE(false_strings); i++)
        જો (0 == strcmp(str, false_strings[i]))
            પાછા 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    પાછા defval;
}

સંરચના const_romfile_s {
    સંરચના romfile_s file;
    શૂન્ય *data;
};

સ્થિર પૂર્ણાંક
const_read_file(સંરચના romfile_s *file, શૂન્ય *dst, u32 maxlen)
{
    જો (file->size > maxlen)
        પાછા -1;
    સંરચના const_romfile_s *cfile;
    cfile = container_of(file, સંરચના const_romfile_s, file);
    જો (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    પાછા file->size;
}

સ્થિર શૂન્ય
const_romfile_add(અક્ષર *name, શૂન્ય *data, પૂર્ણાંક size)
{
    સંરચના const_romfile_s *cfile = malloc_tmp(કદ(*cfile));
    જો (!cfile) {
        warn_noalloc();
        પાછા;
    }
    memset(cfile, 0, કદ(*cfile));
    strtcpy(cfile->file.name, name, કદ(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

શૂન્ય
const_romfile_add_int(અક્ષર *name, u32 કિંમત)
{
    u32 *data = malloc_tmp(કદ(*data));
    જો (!data) {
        warn_noalloc();
        પાછા;
    }
    *data = કિંમત;
    const_romfile_add(name, data, કદ(*data));
}
