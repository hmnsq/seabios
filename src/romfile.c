// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // struct romfile_s
#அந்தர்பாவ "string.h" // memcmp

நிலையான கட்டமைப்பு romfile_s *Romfile_வேர் VARVERIFY32INIT;

வெறுமை
romfile_add(கட்டமைப்பு romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_வேர்;
    Romfile_வேர் = file;
}

// Search for the specified file.
நிலையான கட்டமைப்பு romfile_s *
__romfile_findprefix(மாறிலி எழுத்து *prefix, முழு prefixlen, கட்டமைப்பு romfile_s *prev)
{
    கட்டமைப்பு romfile_s *cur = Romfile_வேர்;
    எனில் (prev)
        cur = prev->next;
    போது (cur) {
        எனில் (memcmp(prefix, cur->name, prefixlen) == 0)
            திரும்பு cur;
        cur = cur->next;
    }
    திரும்பு வெற்று;
}

கட்டமைப்பு romfile_s *
romfile_findprefix(மாறிலி எழுத்து *prefix, கட்டமைப்பு romfile_s *prev)
{
    திரும்பு __romfile_findprefix(prefix, strlen(prefix), prev);
}

கட்டமைப்பு romfile_s *
romfile_find(மாறிலி எழுத்து *name)
{
    திரும்பு __romfile_findprefix(name, strlen(name) + 1, வெற்று);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
வெறுமை *
romfile_loadfile(மாறிலி எழுத்து *name, முழு *psize)
{
    கட்டமைப்பு romfile_s *file = romfile_find(name);
    எனில் (!file)
        திரும்பு வெற்று;

    முழு filesize = file->size;
    எனில் (!filesize)
        திரும்பு வெற்று;

    எழுத்து *data = malloc_tmphigh(filesize+1);
    எனில் (!data) {
        warn_noalloc();
        திரும்பு வெற்று;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    முழு ret = file->copy(file, data, filesize);
    எனில் (ret < 0) {
        விடுவி(data);
        திரும்பு வெற்று;
    }
    எனில் (psize)
        *psize = filesize;
    data[filesize] = '\0';
    திரும்பு data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(மாறிலி எழுத்து *name, u64 defval)
{
    கட்டமைப்பு romfile_s *file = romfile_find(name);
    எனில் (!file)
        திரும்பு defval;

    முழு filesize = file->size;
    எனில் (!filesize || filesize > அளவு(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        திரும்பு defval;

    u64 val = 0;
    முழு ret = file->copy(file, &val, அளவு(val));
    எனில் (ret < 0)
        திரும்பு defval;
    திரும்பு val;
}

u32
romfile_loadbool(மாறிலி எழுத்து *name, u32 defval)
{
    நிலையான மாறிலி எழுத்து *true_strings[] = { "1", "on", "yes" };
    நிலையான மாறிலி எழுத்து *false_strings[] = { "0", "off", "no" };
    எழுத்து *str = romfile_loadfile(name, வெற்று);
    முழு i;

    எனில் (!str)
        திரும்பு defval;

    க்கு (i = 0; i < ARRAY_SIZE(true_strings); i++)
        எனில் (0 == strcmp(str, true_strings[i]))
            திரும்பு 1;

    க்கு (i = 0; i < ARRAY_SIZE(false_strings); i++)
        எனில் (0 == strcmp(str, false_strings[i]))
            திரும்பு 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    திரும்பு defval;
}

கட்டமைப்பு const_romfile_s {
    கட்டமைப்பு romfile_s file;
    வெறுமை *data;
};

நிலையான முழு
const_read_file(கட்டமைப்பு romfile_s *file, வெறுமை *dst, u32 maxlen)
{
    எனில் (file->size > maxlen)
        திரும்பு -1;
    கட்டமைப்பு const_romfile_s *cfile;
    cfile = container_of(file, கட்டமைப்பு const_romfile_s, file);
    எனில் (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    திரும்பு file->size;
}

நிலையான வெறுமை
const_romfile_add(எழுத்து *name, வெறுமை *data, முழு size)
{
    கட்டமைப்பு const_romfile_s *cfile = malloc_tmp(அளவு(*cfile));
    எனில் (!cfile) {
        warn_noalloc();
        திரும்பு;
    }
    memset(cfile, 0, அளவு(*cfile));
    strtcpy(cfile->file.name, name, அளவு(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

வெறுமை
const_romfile_add_int(எழுத்து *name, u32 மதிப்பு)
{
    u32 *data = malloc_tmp(அளவு(*data));
    எனில் (!data) {
        warn_noalloc();
        திரும்பு;
    }
    *data = மதிப்பு;
    const_romfile_add(name, data, அளவு(*data));
}
