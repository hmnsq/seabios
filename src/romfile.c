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
romfile_सङ्कलय(संरचना romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->आकार);
    file->अग्र = Romfile_मूलम्;
    Romfile_मूलम् = file;
}

// Search for the specified file.
स्थिर संरचना romfile_s *
__romfile_findprefix(ध्रुव वर्ण *उपसर्गः, पूर्णाङ्क prefixlen, संरचना romfile_s *पूर्व)
{
    संरचना romfile_s *cur = Romfile_मूलम्;
    यदि (पूर्व)
        cur = पूर्व->अग्र;
    यावत् (cur) {
        यदि (memcmp(उपसर्गः, cur->name, prefixlen) == 0)
            प्रत्यागम cur;
        cur = cur->अग्र;
    }
    प्रत्यागम रिक्त;
}

संरचना romfile_s *
romfile_findprefix(ध्रुव वर्ण *उपसर्गः, संरचना romfile_s *पूर्व)
{
    प्रत्यागम __romfile_findprefix(उपसर्गः, वाक्यदैर्घ्यम्(उपसर्गः), पूर्व);
}

संरचना romfile_s *
romfile_find(ध्रुव वर्ण *name)
{
    प्रत्यागम __romfile_findprefix(name, वाक्यदैर्घ्यम्(name) + 1, रिक्त);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
शून्य *
romfile_loadfile(ध्रुव वर्ण *name, पूर्णाङ्क *psize)
{
    संरचना romfile_s *file = romfile_find(name);
    यदि (!file)
        प्रत्यागम रिक्त;

    पूर्णाङ्क filesize = file->आकार;
    यदि (!filesize)
        प्रत्यागम रिक्त;

    वर्ण *दत्त = malloc_tmphigh(filesize+1);
    यदि (!दत्त) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    पूर्णाङ्क ret = file->copy(file, दत्त, filesize);
    यदि (ret < 0) {
        मुक्त(दत्त);
        प्रत्यागम रिक्त;
    }
    यदि (psize)
        *psize = filesize;
    दत्त[filesize] = '\0';
    प्रत्यागम दत्त;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ध्रुव वर्ण *name, u64 defval)
{
    संरचना romfile_s *file = romfile_find(name);
    यदि (!file)
        प्रत्यागम defval;

    पूर्णाङ्क filesize = file->आकार;
    यदि (!filesize || filesize > परिमाण(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        प्रत्यागम defval;

    u64 val = 0;
    पूर्णाङ्क ret = file->copy(file, &val, परिमाण(val));
    यदि (ret < 0)
        प्रत्यागम defval;
    प्रत्यागम val;
}

u32
romfile_loadbool(ध्रुव वर्ण *name, u32 defval)
{
    स्थिर ध्रुव वर्ण *true_वाक्यानि[] = { "1", "on", "yes" };
    स्थिर ध्रुव वर्ण *false_वाक्यानि[] = { "0", "off", "no" };
    वर्ण *str = romfile_loadfile(name, रिक्त);
    पूर्णाङ्क i;

    यदि (!str)
        प्रत्यागम defval;

    प्रति (i = 0; i < ARRAY_आकार(true_वाक्यानि); i++)
        यदि (0 == वाक्यं_तुलय(str, true_वाक्यानि[i]))
            प्रत्यागम 1;

    प्रति (i = 0; i < ARRAY_आकार(false_वाक्यानि); i++)
        यदि (0 == वाक्यं_तुलय(str, false_वाक्यानि[i]))
            प्रत्यागम 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    प्रत्यागम defval;
}

संरचना const_romfile_s {
    संरचना romfile_s file;
    शून्य *दत्त;
};

स्थिर पूर्णाङ्क
const_read_file(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    यदि (file->आकार > maxlen)
        प्रत्यागम -1;
    संरचना const_romfile_s *cfile;
    cfile = container_सम्बन्धि(file, संरचना const_romfile_s, file);
    यदि (maxlen > file->आकार)
        maxlen = file->आकार;
    स्मृतिं_प्रतिलिख(dst, cfile->दत्त, maxlen);
    प्रत्यागम file->आकार;
}

स्थिर शून्य
const_romfile_सङ्कलय(वर्ण *name, शून्य *दत्त, पूर्णाङ्क आकार)
{
    संरचना const_romfile_s *cfile = malloc_tmp(परिमाण(*cfile));
    यदि (!cfile) {
        warn_noalloc();
        प्रत्यागम;
    }
    स्मृतिं_पूरय(cfile, 0, परिमाण(*cfile));
    strtcpy(cfile->file.name, name, परिमाण(cfile->file.name));
    cfile->file.आकार = आकार;
    cfile->file.copy = const_read_file;
    cfile->दत्त = दत्त;
    romfile_सङ्कलय(&cfile->file);
}

शून्य
const_romfile_सङ्कलय_int(वर्ण *name, u32 मानम्)
{
    u32 *दत्त = malloc_tmp(परिमाण(*दत्त));
    यदि (!दत्त) {
        warn_noalloc();
        प्रत्यागम;
    }
    *दत्त = मानम्;
    const_romfile_सङ्कलय(name, दत्त, परिमाण(*दत्त));
}
