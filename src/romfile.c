// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // struct romfile_s
#అన్తర్భావ "string.h" // memcmp

స్థిర నిర్మాణం romfile_s *Romfile_మూలం VARVERIFY32INIT;

శూన్య
romfile_కూడు(నిర్మాణం romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->కొలత);
    file->తదుపరి = Romfile_మూలం;
    Romfile_మూలం = file;
}

// Search for the specified file.
స్థిర నిర్మాణం romfile_s *
__romfile_findprefix(స్థిరరాశి అక్షరం *ఉపసర్గ, పూర్ణాంకం prefixlen, నిర్మాణం romfile_s *prev)
{
    నిర్మాణం romfile_s *cur = Romfile_మూలం;
    ఒకవేళ (prev)
        cur = prev->తదుపరి;
    అయితే (cur) {
        ఒకవేళ (memcmp(ఉపసర్గ, cur->name, prefixlen) == 0)
            తిరిగి cur;
        cur = cur->తదుపరి;
    }
    తిరిగి ఖాళీ;
}

నిర్మాణం romfile_s *
romfile_findprefix(స్థిరరాశి అక్షరం *ఉపసర్గ, నిర్మాణం romfile_s *prev)
{
    తిరిగి __romfile_findprefix(ఉపసర్గ, అక్షరశ్రేణి_పొడవు(ఉపసర్గ), prev);
}

నిర్మాణం romfile_s *
romfile_కనుగొను(స్థిరరాశి అక్షరం *name)
{
    తిరిగి __romfile_findprefix(name, అక్షరశ్రేణి_పొడవు(name) + 1, ఖాళీ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
శూన్య *
romfile_loadfile(స్థిరరాశి అక్షరం *name, పూర్ణాంకం *psize)
{
    నిర్మాణం romfile_s *file = romfile_కనుగొను(name);
    ఒకవేళ (!file)
        తిరిగి ఖాళీ;

    పూర్ణాంకం filesize = file->కొలత;
    ఒకవేళ (!filesize)
        తిరిగి ఖాళీ;

    అక్షరం *దత్తాంశం = malloc_tmphigh(filesize+1);
    ఒకవేళ (!దత్తాంశం) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    పూర్ణాంకం ret = file->copy(file, దత్తాంశం, filesize);
    ఒకవేళ (ret < 0) {
        విడుదల(దత్తాంశం);
        తిరిగి ఖాళీ;
    }
    ఒకవేళ (psize)
        *psize = filesize;
    దత్తాంశం[filesize] = '\0';
    తిరిగి దత్తాంశం;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(స్థిరరాశి అక్షరం *name, u64 defval)
{
    నిర్మాణం romfile_s *file = romfile_కనుగొను(name);
    ఒకవేళ (!file)
        తిరిగి defval;

    పూర్ణాంకం filesize = file->కొలత;
    ఒకవేళ (!filesize || filesize > పరిమాణం(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        తిరిగి defval;

    u64 val = 0;
    పూర్ణాంకం ret = file->copy(file, &val, పరిమాణం(val));
    ఒకవేళ (ret < 0)
        తిరిగి defval;
    తిరిగి val;
}

u32
romfile_loadbool(స్థిరరాశి అక్షరం *name, u32 defval)
{
    స్థిర స్థిరరాశి అక్షరం *true_అక్షరశ్రేణులు[] = { "1", "on", "yes" };
    స్థిర స్థిరరాశి అక్షరం *false_అక్షరశ్రేణులు[] = { "0", "off", "no" };
    అక్షరం *str = romfile_loadfile(name, ఖాళీ);
    పూర్ణాంకం i;

    ఒకవేళ (!str)
        తిరిగి defval;

    కొరకు (i = 0; i < ARRAY_కొలత(true_అక్షరశ్రేణులు); i++)
        ఒకవేళ (0 == strcmp(str, true_అక్షరశ్రేణులు[i]))
            తిరిగి 1;

    కొరకు (i = 0; i < ARRAY_కొలత(false_అక్షరశ్రేణులు); i++)
        ఒకవేళ (0 == strcmp(str, false_అక్షరశ్రేణులు[i]))
            తిరిగి 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    తిరిగి defval;
}

నిర్మాణం const_romfile_s {
    నిర్మాణం romfile_s file;
    శూన్య *దత్తాంశం;
};

స్థిర పూర్ణాంకం
const_read_file(నిర్మాణం romfile_s *file, శూన్య *dst, u32 maxlen)
{
    ఒకవేళ (file->కొలత > maxlen)
        తిరిగి -1;
    నిర్మాణం const_romfile_s *cfile;
    cfile = container_యొక్క(file, నిర్మాణం const_romfile_s, file);
    ఒకవేళ (maxlen > file->కొలత)
        maxlen = file->కొలత;
    జ్ఞాపకం_నకలు(dst, cfile->దత్తాంశం, maxlen);
    తిరిగి file->కొలత;
}

స్థిర శూన్య
const_romfile_కూడు(అక్షరం *name, శూన్య *దత్తాంశం, పూర్ణాంకం కొలత)
{
    నిర్మాణం const_romfile_s *cfile = malloc_tmp(పరిమాణం(*cfile));
    ఒకవేళ (!cfile) {
        warn_noalloc();
        తిరిగి;
    }
    memset(cfile, 0, పరిమాణం(*cfile));
    strtcpy(cfile->file.name, name, పరిమాణం(cfile->file.name));
    cfile->file.కొలత = కొలత;
    cfile->file.copy = const_read_file;
    cfile->దత్తాంశం = దత్తాంశం;
    romfile_కూడు(&cfile->file);
}

శూన్య
const_romfile_కూడు_int(అక్షరం *name, u32 విలువ)
{
    u32 *దత్తాంశం = malloc_tmp(పరిమాణం(*దత్తాంశం));
    ఒకవేళ (!దత్తాంశం) {
        warn_noalloc();
        తిరిగి;
    }
    *దత్తాంశం = విలువ;
    const_romfile_కూడు(name, దత్తాంశం, పరిమాణం(*దత్తాంశం));
}
