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

स्थिर संरचना romfile_s *Romfile_मूळ VARVERIFY32INIT;

शून्य
romfile_add(संरचना romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_मूळ;
    Romfile_मूळ = file;
}

// Search for the specified file.
स्थिर संरचना romfile_s *
__romfile_findprefix(स्थिरांक अक्षर *prefix, पूर्णांक prefixlen, संरचना romfile_s *prev)
{
    संरचना romfile_s *cur = Romfile_मूळ;
    जर (prev)
        cur = prev->next;
    पर्यंत (cur) {
        जर (स_भेद(prefix, cur->name, prefixlen) == 0)
            परत cur;
        cur = cur->next;
    }
    परत रिक्त;
}

संरचना romfile_s *
romfile_findprefix(स्थिरांक अक्षर *prefix, संरचना romfile_s *prev)
{
    परत __romfile_findprefix(prefix, म_माप(prefix), prev);
}

संरचना romfile_s *
romfile_find(स्थिरांक अक्षर *name)
{
    परत __romfile_findprefix(name, म_माप(name) + 1, रिक्त);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
शून्य *
romfile_loadfile(स्थिरांक अक्षर *name, पूर्णांक *psize)
{
    संरचना romfile_s *file = romfile_find(name);
    जर (!file)
        परत रिक्त;

    पूर्णांक filesize = file->size;
    जर (!filesize)
        परत रिक्त;

    अक्षर *data = malloc_tmphigh(filesize+1);
    जर (!data) {
        warn_noalloc();
        परत रिक्त;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    पूर्णांक ret = file->copy(file, data, filesize);
    जर (ret < 0) {
        मुक्त(data);
        परत रिक्त;
    }
    जर (psize)
        *psize = filesize;
    data[filesize] = '\0';
    परत data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(स्थिरांक अक्षर *name, u64 defval)
{
    संरचना romfile_s *file = romfile_find(name);
    जर (!file)
        परत defval;

    पूर्णांक filesize = file->size;
    जर (!filesize || filesize > आकार(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        परत defval;

    u64 val = 0;
    पूर्णांक ret = file->copy(file, &val, आकार(val));
    जर (ret < 0)
        परत defval;
    परत val;
}

u32
romfile_loadbool(स्थिरांक अक्षर *name, u32 defval)
{
    स्थिर स्थिरांक अक्षर *true_strings[] = { "1", "on", "yes" };
    स्थिर स्थिरांक अक्षर *false_strings[] = { "0", "off", "no" };
    अक्षर *str = romfile_loadfile(name, रिक्त);
    पूर्णांक i;

    जर (!str)
        परत defval;

    साठी (i = 0; i < ARRAY_SIZE(true_strings); i++)
        जर (0 == म_भेद(str, true_strings[i]))
            परत 1;

    साठी (i = 0; i < ARRAY_SIZE(false_strings); i++)
        जर (0 == म_भेद(str, false_strings[i]))
            परत 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    परत defval;
}

संरचना const_romfile_s {
    संरचना romfile_s file;
    शून्य *data;
};

स्थिर पूर्णांक
const_read_file(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    जर (file->size > maxlen)
        परत -1;
    संरचना const_romfile_s *cfile;
    cfile = container_of(file, संरचना const_romfile_s, file);
    जर (maxlen > file->size)
        maxlen = file->size;
    स_नकल(dst, cfile->data, maxlen);
    परत file->size;
}

स्थिर शून्य
const_romfile_add(अक्षर *name, शून्य *data, पूर्णांक size)
{
    संरचना const_romfile_s *cfile = malloc_tmp(आकार(*cfile));
    जर (!cfile) {
        warn_noalloc();
        परत;
    }
    स_रखो(cfile, 0, आकार(*cfile));
    strtcpy(cfile->file.name, name, आकार(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

शून्य
const_romfile_add_int(अक्षर *name, u32 मूल्य)
{
    u32 *data = malloc_tmp(आकार(*data));
    जर (!data) {
        warn_noalloc();
        परत;
    }
    *data = मूल्य;
    const_romfile_add(name, data, आकार(*data));
}
