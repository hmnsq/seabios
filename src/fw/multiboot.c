// Multiboot interface support.
//
// Copyright (C) 2015  Vladimir Serbinenko <phcoder@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_add
#समावेश "std/multiboot.h" // MULTIBOOT_*
#समावेश "string.h" // memset
#समावेश "util.h" // multiboot_init

काष्ठा mbfs_romfile_s {
    काष्ठा romfile_s file;
    व्योम *तथ्य;
};

अटल पूर्णांक
extract_filename(अक्षर *dest, अक्षर *src, माप_प्रकार lim)
{
    अक्षर *ptr;
    क्रम (ptr = src; *ptr; ptr++) {
        अगर (!(ptr == src || ptr[-1] == ' ' || ptr[-1] == '\t'))
            जारी;
        /* memcmp stops early if it encounters \0 as it doesn't match name=.  */
        अगर (स_भेद(ptr, "name=", 5) == 0) {
            पूर्णांक i;
            अक्षर *optr = dest;
            क्रम (i = 0, ptr += 5; *ptr && *ptr != ' ' && i < lim; i++) {
                *optr++ = *ptr++;
            }
            *optr++ = '\0';
            वापस 1;
        }
    }
    वापस 0;
}

// Copy a file to memory
अटल पूर्णांक
mbfs_copyfile(काष्ठा romfile_s *file, व्योम *dst, u32 maxlen)
{
    काष्ठा mbfs_romfile_s *cfile;
    cfile = container_of(file, काष्ठा mbfs_romfile_s, file);
    u32 आकार = cfile->file.आकार;
    व्योम *src = cfile->तथ्य;

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", आकार, src, maxlen, dst);
    अगर (आकार > maxlen) {
        warn_noalloc();
        वापस -1;
    }
    iomemcpy(dst, src, आकार);
    वापस आकार;
}

u32 __VISIBLE entry_elf_eax, entry_elf_ebx;

व्योम
multiboot_init(व्योम)
{
    काष्ठा multiboot_info *mbi;
    अगर (!CONFIG_MULTIBOOT)
        वापस;
    dprintf(1, "multiboot: eax=%x, ebx=%x\n", entry_elf_eax, entry_elf_ebx);
    अगर (entry_elf_eax != MULTIBOOT_BOOTLOADER_MAGIC)
        वापस;
    mbi = (व्योम *)entry_elf_ebx;
    dprintf(1, "mbptr=%p\n", mbi);
    dprintf(1, "flags=0x%x, mods=0x%x, mods_c=%d\n", mbi->flags, mbi->mods_addr,
            mbi->mods_गिनती);
    अगर (!(mbi->flags & MULTIBOOT_INFO_MODS))
        वापस;
    पूर्णांक i;
    काष्ठा multiboot_mod_list *mod = (व्योम *)mbi->mods_addr;
    क्रम (i = 0; i < mbi->mods_गिनती; i++) {
        काष्ठा mbfs_romfile_s *cfile;
        u8 *copy;
        u32 len;
        अगर (!mod[i].cmdline)
            जारी;
        len = mod[i].mod_end - mod[i].mod_start;
        cfile = malloc_tmp(माप(*cfile));
        अगर (!cfile) {
            warn_noalloc();
            वापस;
        }
        स_रखो(cfile, 0, माप(*cfile));
        dprintf(1, "module %s, size 0x%x\n", (अक्षर *)mod[i].cmdline, len);
        अगर (!extract_filename(cfile->file.name, (अक्षर *)mod[i].cmdline,
                              माप(cfile->file.name))) {
            मुक्त(cfile);
            जारी;
        }
        dprintf(1, "assigned file name <%s>\n", cfile->file.name);
        cfile->file.आकार = len;
        copy = malloc_tmp(len);
        अगर (!copy) {
            warn_noalloc();
            मुक्त(cfile);
            वापस;
        }
        स_नकल(copy, (व्योम *)mod[i].mod_start, len);
        cfile->file.copy = mbfs_copyfile;
        cfile->तथ्य = copy;
        romfile_add(&cfile->file);
    }
}
