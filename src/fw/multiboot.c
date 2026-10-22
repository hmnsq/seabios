// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_add
#अन्तर्भाव "std/multiboot.h" // MULTIBOOT_*
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // multiboot_init

संरचना mbfs_romfile_s {
    संरचना romfile_s file;
    शून्य *दत्त;
};

स्थिर पूर्णाङ्क
extract_filename(वर्ण *dest, वर्ण *src, आकार_t lim)
{
    वर्ण *ptr;
    प्रति (ptr = src; *ptr; ptr++) {
        यदि (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            अनुवर्त;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        यदि (memcmp(ptr, "name=", 5) == 0) {
            पूर्णाङ्क i;
            वर्ण *optr = dest;
            प्रति (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            प्रत्यागम 1;
        }
    }
    प्रत्यागम 0;
}

// Copy a file to memory
स्थिर पूर्णाङ्क
mbfs_copyfile(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    संरचना mbfs_romfile_s *cfile;
    cfile = container_सम्बन्धि(file, संरचना mbfs_romfile_s, file);
    u32 आकार = cfile->file.आकार;
    शून्य *src = cfile->दत्त;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", आकार, src, maxlen, dst);
    यदि (आकार > maxlen) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    iomemcpy(dst, src, आकार);
    प्रत्यागम आकार;
}

u32 __VISIBLE प्रविष्टिः_elf_eax, प्रविष्टिः_elf_ebx;

शून्य
multiboot_init(शून्य)
{
    संरचना multiboot_info *mbi;
    यदि (!CONFIG_MULTIBOOT)
        प्रत्यागम;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", प्रविष्टिः_elf_eax, प्रविष्टिः_elf_ebx);
    यदि (प्रविष्टिः_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        प्रत्यागम;
    mbi = (शून्य *)प्रविष्टिः_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_गणनम्);
    यदि (!(mbi->flags & MULTIBOOT_INFO_MODS))
        प्रत्यागम;
    पूर्णाङ्क i;
    संरचना multiboot_mod_list *mod = (शून्य *)mbi->mods_addr;
    प्रति (i = 0; i < mbi->mods_गणनम्; i++) {
        संरचना mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        यदि (!mod[i].cmdline)
            अनुवर्त;
        len = mod[i].mod_अन्तः - mod[i].mod_start;
        cfile = malloc_tmp(परिमाण(*cfile));
        यदि (!cfile) {
            warn_noalloc();
            प्रत्यागम;
        }
        स्मृतिं_पूरय(cfile, 0, परिमाण(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (वर्ण *)mod[i].cmdline, len);
        यदि (!extract_filename(cfile->file.name, (वर्ण *)mod[i].cmdline,
                              परिमाण(cfile->file.name))) {
            मुक्त(cfile);
            अनुवर्त;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.आकार = len;
        copy = malloc_tmp(len);
        यदि (!copy) {
            warn_noalloc();
            मुक्त(cfile);
            प्रत्यागम;
        }
        स्मृतिं_प्रतिलिख(copy, (शून्य *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->दत्त = copy;
        romfile_सङ्कलय(&cfile->file);
    }
}
