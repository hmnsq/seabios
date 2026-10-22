// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // struct romfile_s
#ଅନ୍ତର୍ଭାଵ "string.h" // memcmp

ସ୍ଥିର ସଂରଚନା romfile_s *Romfile_ମୂଳ VARVERIFY32INIT;

ଶୂନ୍ୟ
romfile_add(ସଂରଚନା romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_ମୂଳ;
    Romfile_ମୂଳ = file;
}

// Search for the specified file.
ସ୍ଥିର ସଂରଚନା romfile_s *
__romfile_findprefix(ଅଚଳ ଅକ୍ଷର *prefix, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prefixlen, ସଂରଚନା romfile_s *prev)
{
    ସଂରଚନା romfile_s *cur = Romfile_ମୂଳ;
    ଯଦି (prev)
        cur = prev->next;
    ଯେତେବେଳେ (cur) {
        ଯଦି (memcmp(prefix, cur->name, prefixlen) == 0)
            ଫେରନ୍ତୁ cur;
        cur = cur->next;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା romfile_s *
romfile_findprefix(ଅଚଳ ଅକ୍ଷର *prefix, ସଂରଚନା romfile_s *prev)
{
    ଫେରନ୍ତୁ __romfile_findprefix(prefix, strlen(prefix), prev);
}

ସଂରଚନା romfile_s *
romfile_find(ଅଚଳ ଅକ୍ଷର *name)
{
    ଫେରନ୍ତୁ __romfile_findprefix(name, strlen(name) + 1, ଖାଲି);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
ଶୂନ୍ୟ *
romfile_loadfile(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *psize)
{
    ସଂରଚନା romfile_s *file = romfile_find(name);
    ଯଦି (!file)
        ଫେରନ୍ତୁ ଖାଲି;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା filesize = file->size;
    ଯଦି (!filesize)
        ଫେରନ୍ତୁ ଖାଲି;

    ଅକ୍ଷର *data = malloc_tmphigh(filesize+1);
    ଯଦି (!data) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = file->copy(file, data, filesize);
    ଯଦି (ret < 0) {
        ମୁକ୍ତ(data);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଯଦି (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ଫେରନ୍ତୁ data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ଅଚଳ ଅକ୍ଷର *name, u64 defval)
{
    ସଂରଚନା romfile_s *file = romfile_find(name);
    ଯଦି (!file)
        ଫେରନ୍ତୁ defval;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା filesize = file->size;
    ଯଦି (!filesize || filesize > ଆକାର(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ଫେରନ୍ତୁ defval;

    u64 val = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = file->copy(file, &val, ଆକାର(val));
    ଯଦି (ret < 0)
        ଫେରନ୍ତୁ defval;
    ଫେରନ୍ତୁ val;
}

u32
romfile_loadbool(ଅଚଳ ଅକ୍ଷର *name, u32 defval)
{
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *true_strings[] = { "1", "on", "yes" };
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *false_strings[] = { "0", "off", "no" };
    ଅକ୍ଷର *str = romfile_loadfile(name, ଖାଲି);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    ଯଦି (!str)
        ଫେରନ୍ତୁ defval;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(true_strings); i++)
        ଯଦି (0 == strcmp(str, true_strings[i]))
            ଫେରନ୍ତୁ 1;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(false_strings); i++)
        ଯଦି (0 == strcmp(str, false_strings[i]))
            ଫେରନ୍ତୁ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ଫେରନ୍ତୁ defval;
}

ସଂରଚନା const_romfile_s {
    ସଂରଚନା romfile_s file;
    ଶୂନ୍ୟ *data;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
const_read_file(ସଂରଚନା romfile_s *file, ଶୂନ୍ୟ *dst, u32 maxlen)
{
    ଯଦି (file->size > maxlen)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା const_romfile_s *cfile;
    cfile = container_of(file, ସଂରଚନା const_romfile_s, file);
    ଯଦି (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ଫେରନ୍ତୁ file->size;
}

ସ୍ଥିର ଶୂନ୍ୟ
const_romfile_add(ଅକ୍ଷର *name, ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
    ସଂରଚନା const_romfile_s *cfile = malloc_tmp(ଆକାର(*cfile));
    ଯଦି (!cfile) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(cfile, 0, ଆକାର(*cfile));
    strtcpy(cfile->file.name, name, ଆକାର(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

ଶୂନ୍ୟ
const_romfile_add_int(ଅକ୍ଷର *name, u32 ମୂଲ୍ୟ)
{
    u32 *data = malloc_tmp(ଆକାର(*data));
    ଯଦି (!data) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    *data = ମୂଲ୍ୟ;
    const_romfile_add(name, data, ଆକାର(*data));
}
