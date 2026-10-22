// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // struct romfile_s
#अन्तर्भाव "string.h" // memcmp

स्थिर संरचना romfile_s *Romfile_मूलम् VARVERIFY32INIT;

शून्य
romfile_add(संरचना romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_मूलम्;
    Romfile_मूलम् = file;
}

// Search for the specified file.
स्थिर संरचना romfile_s *
__romfile_findprefix(ध्रुव वर्ण *prefix, पूर्णाङ्क prefixlen, संरचना romfile_s *prev)
{
    संरचना romfile_s *cur = Romfile_मूलम्;
    सचे (prev)
        cur = prev->next;
    याव (cur) {
        सचे (memcmp(prefix, cur->name, prefixlen) == 0)
            पटिनिवत्त cur;
        cur = cur->next;
    }
    पटिनिवत्त रिक्त;
}

संरचना romfile_s *
romfile_findprefix(ध्रुव वर्ण *prefix, संरचना romfile_s *prev)
{
    पटिनिवत्त __romfile_findprefix(prefix, strlen(prefix), prev);
}

संरचना romfile_s *
romfile_find(ध्रुव वर्ण *name)
{
    पटिनिवत्त __romfile_findprefix(name, strlen(name) + 1, रिक्त);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
शून्य *
romfile_loadfile(ध्रुव वर्ण *name, पूर्णाङ्क *psize)
{
    संरचना romfile_s *file = romfile_find(name);
    सचे (!file)
        पटिनिवत्त रिक्त;

    पूर्णाङ्क filesize = file->size;
    सचे (!filesize)
        पटिनिवत्त रिक्त;

    वर्ण *data = malloc_tmphigh(filesize+1);
    सचे (!data) {
        warn_noalloc();
        पटिनिवत्त रिक्त;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    पूर्णाङ्क ret = file->copy(file, data, filesize);
    सचे (ret < 0) {
        मुक्त(data);
        पटिनिवत्त रिक्त;
    }
    सचे (psize)
        *psize = filesize;
    data[filesize] = '\0';
    पटिनिवत्त data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ध्रुव वर्ण *name, u64 defval)
{
    संरचना romfile_s *file = romfile_find(name);
    सचे (!file)
        पटिनिवत्त defval;

    पूर्णाङ्क filesize = file->size;
    सचे (!filesize || filesize > परिमाण(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        पटिनिवत्त defval;

    u64 val = 0;
    पूर्णाङ्क ret = file->copy(file, &val, परिमाण(val));
    सचे (ret < 0)
        पटिनिवत्त defval;
    पटिनिवत्त val;
}

u32
romfile_loadbool(ध्रुव वर्ण *name, u32 defval)
{
    स्थिर ध्रुव वर्ण *true_strings[] = { "1", "on", "yes" };
    स्थिर ध्रुव वर्ण *false_strings[] = { "0", "off", "no" };
    वर्ण *str = romfile_loadfile(name, रिक्त);
    पूर्णाङ्क i;

    सचे (!str)
        पटिनिवत्त defval;

    अत्थाय (i = 0; i < ARRAY_SIZE(true_strings); i++)
        सचे (0 == strcmp(str, true_strings[i]))
            पटिनिवत्त 1;

    अत्थाय (i = 0; i < ARRAY_SIZE(false_strings); i++)
        सचे (0 == strcmp(str, false_strings[i]))
            पटिनिवत्त 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    पटिनिवत्त defval;
}

संरचना const_romfile_s {
    संरचना romfile_s file;
    शून्य *data;
};

स्थिर पूर्णाङ्क
const_read_file(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    सचे (file->size > maxlen)
        पटिनिवत्त -1;
    संरचना const_romfile_s *cfile;
    cfile = container_of(file, संरचना const_romfile_s, file);
    सचे (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    पटिनिवत्त file->size;
}

स्थिर शून्य
const_romfile_add(वर्ण *name, शून्य *data, पूर्णाङ्क size)
{
    संरचना const_romfile_s *cfile = malloc_tmp(परिमाण(*cfile));
    सचे (!cfile) {
        warn_noalloc();
        पटिनिवत्त;
    }
    memset(cfile, 0, परिमाण(*cfile));
    strtcpy(cfile->file.name, name, परिमाण(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

शून्य
const_romfile_add_int(वर्ण *name, u32 मानम्)
{
    u32 *data = malloc_tmp(परिमाण(*data));
    सचे (!data) {
        warn_noalloc();
        पटिनिवत्त;
    }
    *data = मानम्;
    const_romfile_add(name, data, परिमाण(*data));
}
