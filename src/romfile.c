// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // struct romfile_s
#समावेश "string.h" // memcmp

अटल काष्ठा romfile_s *Romfile_मूल VARVERIFY32INIT;

व्योम
romfile_add(काष्ठा romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->आकार);
    file->अग्र = Romfile_मूल;
    Romfile_मूल = file;
}

// Search for the specified file.
अटल काष्ठा romfile_s *
__romfile_findprefix(स्थिर अक्षर *prefix, पूर्णांक prefixlen, काष्ठा romfile_s *पूर्व)
{
    काष्ठा romfile_s *cur = Romfile_मूल;
    अगर (पूर्व)
        cur = पूर्व->अग्र;
    जबतक (cur) {
        अगर (स_भेद(prefix, cur->name, prefixlen) == 0)
            वापस cur;
        cur = cur->अग्र;
    }
    वापस शून्य;
}

काष्ठा romfile_s *
romfile_findprefix(स्थिर अक्षर *prefix, काष्ठा romfile_s *पूर्व)
{
    वापस __romfile_findprefix(prefix, म_माप(prefix), पूर्व);
}

काष्ठा romfile_s *
romfile_find(स्थिर अक्षर *name)
{
    वापस __romfile_findprefix(name, म_माप(name) + 1, शून्य);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
व्योम *
romfile_loadfile(स्थिर अक्षर *name, पूर्णांक *psize)
{
    काष्ठा romfile_s *file = romfile_find(name);
    अगर (!file)
        वापस शून्य;

    पूर्णांक filesize = file->आकार;
    अगर (!filesize)
        वापस शून्य;

    अक्षर *तथ्य = malloc_tmphigh(filesize+1);
    अगर (!तथ्य) {
        warn_noalloc();
        वापस शून्य;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    पूर्णांक ret = file->copy(file, तथ्य, filesize);
    अगर (ret < 0) {
        मुक्त(तथ्य);
        वापस शून्य;
    }
    अगर (psize)
        *psize = filesize;
    तथ्य[filesize] = '\0';
    वापस तथ्य;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(स्थिर अक्षर *name, u64 defval)
{
    काष्ठा romfile_s *file = romfile_find(name);
    अगर (!file)
        वापस defval;

    पूर्णांक filesize = file->आकार;
    अगर (!filesize || filesize > माप(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        वापस defval;

    u64 val = 0;
    पूर्णांक ret = file->copy(file, &val, माप(val));
    अगर (ret < 0)
        वापस defval;
    वापस val;
}

u32
romfile_loadbool(स्थिर अक्षर *name, u32 defval)
{
    अटल स्थिर अक्षर *true_strings[] = { "1", "on", "yes" };
    अटल स्थिर अक्षर *false_strings[] = { "0", "off", "no" };
    अक्षर *str = romfile_loadfile(name, शून्य);
    पूर्णांक i;

    अगर (!str)
        वापस defval;

    क्रम (i = 0; i < ARRAY_आकार(true_strings); i++)
        अगर (0 == म_भेद(str, true_strings[i]))
            वापस 1;

    क्रम (i = 0; i < ARRAY_आकार(false_strings); i++)
        अगर (0 == म_भेद(str, false_strings[i]))
            वापस 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    वापस defval;
}

काष्ठा const_romfile_s {
    काष्ठा romfile_s file;
    व्योम *तथ्य;
};

अटल पूर्णांक
const_read_file(काष्ठा romfile_s *file, व्योम *dst, u32 maxlen)
{
    अगर (file->आकार > maxlen)
        वापस -1;
    काष्ठा const_romfile_s *cfile;
    cfile = container_of(file, काष्ठा const_romfile_s, file);
    अगर (maxlen > file->आकार)
        maxlen = file->आकार;
    स_नकल(dst, cfile->तथ्य, maxlen);
    वापस file->आकार;
}

अटल व्योम
const_romfile_add(अक्षर *name, व्योम *तथ्य, पूर्णांक आकार)
{
    काष्ठा const_romfile_s *cfile = malloc_tmp(माप(*cfile));
    अगर (!cfile) {
        warn_noalloc();
        वापस;
    }
    स_रखो(cfile, 0, माप(*cfile));
    strtcpy(cfile->file.name, name, माप(cfile->file.name));
    cfile->file.आकार = आकार;
    cfile->file.copy = const_read_file;
    cfile->तथ्य = तथ्य;
    romfile_add(&cfile->file);
}

व्योम
const_romfile_add_int(अक्षर *name, u32 मूल्य)
{
    u32 *तथ्य = malloc_tmp(माप(*तथ्य));
    अगर (!तथ्य) {
        warn_noalloc();
        वापस;
    }
    *तथ्य = मूल्य;
    const_romfile_add(name, तथ्य, माप(*तथ्य));
}
