// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_add
#അന്തര്ഭാവ "std/multiboot.h" // MULTIBOOT_*
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // multiboot_init

ഘടന mbfs_romfile_s {
    ഘടന romfile_s file;
    ശൂന്യം *data;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ
extract_filename(അക്ഷരം *dest, അക്ഷരം *src, size_t lim)
{
    അക്ഷരം *ptr;
    വേണ്ടി (ptr = src; *ptr; ptr++) {
        എങ്കിൽ (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            തുടരുക;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        എങ്കിൽ (memcmp(ptr, "name=", 5) == 0) {
            പൂർണ്ണസംഖ്യ i;
            അക്ഷരം *optr = dest;
            വേണ്ടി (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            തിരികെ 1;
        }
    }
    തിരികെ 0;
}

// Copy a file to memory
സ്ഥിതം പൂർണ്ണസംഖ്യ
mbfs_copyfile(ഘടന romfile_s *file, ശൂന്യം *dst, u32 maxlen)
{
    ഘടന mbfs_romfile_s *cfile;
    cfile = container_of(file, ഘടന mbfs_romfile_s, file);
    u32 size = cfile->file.size;
    ശൂന്യം *src = cfile->data;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    എങ്കിൽ (size > maxlen) {
        warn_noalloc();
        തിരികെ -1;
    }
    iomemcpy(dst, src, size);
    തിരികെ size;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

ശൂന്യം
multiboot_init(ശൂന്യം)
{
    ഘടന multiboot_info *mbi;
    എങ്കിൽ (!CONFIG_MULTIBOOT)
        തിരികെ;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    എങ്കിൽ (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        തിരികെ;
    mbi = (ശൂന്യം *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_എണ്ണം);
    എങ്കിൽ (!(mbi->flags & MULTIBOOT_INFO_MODS))
        തിരികെ;
    പൂർണ്ണസംഖ്യ i;
    ഘടന multiboot_mod_list *mod = (ശൂന്യം *)mbi->mods_addr;
    വേണ്ടി (i = 0; i < mbi->mods_എണ്ണം; i++) {
        ഘടന mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        എങ്കിൽ (!mod[i].cmdline)
            തുടരുക;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(വലുപ്പം(*cfile));
        എങ്കിൽ (!cfile) {
            warn_noalloc();
            തിരികെ;
        }
        memset(cfile, 0, വലുപ്പം(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (അക്ഷരം *)mod[i].cmdline, len);
        എങ്കിൽ (!extract_filename(cfile->file.name, (അക്ഷരം *)mod[i].cmdline,
                              വലുപ്പം(cfile->file.name))) {
            സ്വതന്ത്രം(cfile);
            തുടരുക;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.size = len;
        copy = malloc_tmp(len);
        എങ്കിൽ (!copy) {
            warn_noalloc();
            സ്വതന്ത്രം(cfile);
            തിരികെ;
        }
        memcpy(copy, (ശൂന്യം *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->data = copy;
        romfile_add(&cfile->file);
    }
}
