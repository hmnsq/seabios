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
    शून्य *data;
};

स्थिर पूर्णाङ्क
extract_filename(वर्ण *dest, वर्ण *src, size_t lim)
{
    वर्ण *ptr;
    अत्थाय (ptr = src; *ptr; ptr++) {
        सचे (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            अनुवर्त;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        सचे (memcmp(ptr, "name=", 5) == 0) {
            पूर्णाङ्क i;
            वर्ण *optr = dest;
            अत्थाय (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            पटिनिवत्त 1;
        }
    }
    पटिनिवत्त 0;
}

// Copy a file to memory
स्थिर पूर्णाङ्क
mbfs_copyfile(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    संरचना mbfs_romfile_s *cfile;
    cfile = container_of(file, संरचना mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    शून्य *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    सचे (size > maxlen) {
        warn_noalloc();
        पटिनिवत्त -1;
    }
    iomemcpy(dst, src, size);
    पटिनिवत्त size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

शून्य
multiboot_init(शून्य)
{
    संरचना multiboot_info *mbi;
    सचे (!CONFIG_MULTIBOOT)
        पटिनिवत्त;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    सचे (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        पटिनिवत्त;
    mbi = (शून्य *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_गणनम्);
    सचे (!(mbi->flags & MULTIBOOT_INFO_MODS))
        पटिनिवत्त;
    पूर्णाङ्क i;
    संरचना multiboot_mod_list *mod = (शून्य *)mbi->mods_addr;
    अत्थाय (i = 0; i < mbi->mods_गणनम्; i++) {
        संरचना mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        सचे (!mod[i].cmdline)
            अनुवर्त;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(परिमाण(*cfile));
        सचे (!cfile) {
            warn_noalloc();
            पटिनिवत्त;
        }
        memset(cfile, 0, परिमाण(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (वर्ण *)mod[i].cmdline, len);
        सचे (!extract_filename(cfile->file.name, (वर्ण *)mod[i].cmdline,
                              परिमाण(cfile->file.name))) {
            मुक्त(cfile);
            अनुवर्त;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        सचे (!copy) {
            warn_noalloc();
            मुक्त(cfile);
            पटिनिवत्त;
        }
        memcpy(copy, (शून्य *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
