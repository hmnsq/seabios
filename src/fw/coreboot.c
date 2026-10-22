// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // MAXDESCSIZE
#ضمن "byteorder.h" // be32_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "e820map.h" // e820_add
#ضمن "hw/pcidevice.h" // pci_probe_devices
#ضمن "lzmadecode.h" // LzmaDecode
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // PlatformRunningOn
#ضمن "romfile.h" // romfile_findprefix
#ضمن "stacks.h" // yield
#ضمن "string.h" // memset
#ضمن "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

بنية cb_سرو {
    u32 signature;
    u32 سرو_بائيٽون;
    u32 سرو_checksum;
    u32 جدول_بائيٽون;
    u32 جدول_checksum;
    u32 جدول_entries;
};

#وصف_ڪريو CB_SIGNATURE 0x4f49424C // "LBIO"

بنية cb_يادگيري_range {
    u64 شروعات;
    u64 سائيز;
    u32 type;
};

#وصف_ڪريو CB_MEM_جدول    16

بنية cb_يادگيري {
    u32 ٽيگ;
    u32 سائيز;
    بنية cb_يادگيري_range map[0];
};

#وصف_ڪريو CB_ٽيگ_يادگيري 0x01

#وصف_ڪريو MEM_RANGE_ڳڻپ(_rec) \
        (((_rec)->سائيز - حجم(*(_rec))) / حجم((_rec)->map[0]))

بنية cb_mainboard {
    u32 ٽيگ;
    u32 سائيز;
    u8  vendor_idx;
    u8  part_idx;
    حرف  لڙيون[0];
};

#وصف_ڪريو CB_ٽيگ_MAINBOARD 0x0003

بنية cb_forward {
    u32 ٽيگ;
    u32 سائيز;
    u64 forward;
};

#وصف_ڪريو CB_ٽيگ_FORWARD 0x11

بنية cb_cbmem_ref {
    u32 ٽيگ;
    u32 سائيز;
    u64 cbmem_addr;
};

#وصف_ڪريو CB_ٽيگ_CBMEM_CONSOLE 0x17

بنية cbmem_console {
    u32 سائيز;
    u32 cursor;
    u8  body[0];
} PACKED;
#وصف_ڪريو CBMC_CURSOR_نقاب ((1 << 28) - 1)
#وصف_ڪريو CBMC_حد_کان_مٿي (1 << 31)
ساكن بنية cbmem_console *cbcon = عدم;

ساكن u16
ipchksum(حرف *buf, صحيح ڳڻپ)
{
    u16 *p = (u16*)buf;
    u32 جوڙ = 0;
    طالما (ڳڻپ > 1) {
        جوڙ += GET_FARVAR(0, *p);
        p++;
        ڳڻپ -= 2;
    }
    إذا (ڳڻپ)
        جوڙ += GET_FARVAR(0, *(u8*)p);
    جوڙ = (جوڙ >> 16) + (جوڙ & 0xffff);
    جوڙ += (جوڙ >> 16);
    أرجع ~جوڙ;
}

// Try to locate the coreboot header in a given address range.
ساكن بنية cb_سرو *
ڳوليو_cb_سرو(u32 addr, صحيح len)
{
    u32 پڇاڙي = addr + len;
    لكل (; addr < پڇاڙي; addr += 16) {
        بنية cb_سرو *cbh = (فراغ*)addr;
        إذا (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            استمر;
        u32 tsize = GET_FARVAR(0, cbh->جدول_بائيٽون);
        إذا (! tsize)
            استمر;
        إذا (ipchksum((فراغ*)addr, حجم(*cbh)) != 0)
            استمر;
        إذا (ipchksum((فراغ*)addr + حجم(*cbh), tsize)
            != GET_FARVAR(0, cbh->جدول_checksum))
            استمر;
        أرجع cbh;
    }
    أرجع عدم;
}

// Try to find the coreboot memory table in the given coreboot table.
فراغ *
ڳوليو_cb_subtable(بنية cb_سرو *cbh, u32 ٽيگ)
{
    حرف *tbl = (حرف *)cbh + حجم(*cbh);
    u32 ڳڻپ = GET_FARVAR(0, cbh->جدول_entries);
    صحيح i;
    لكل (i=0; i<ڳڻپ; i++) {
        بنية cb_يادگيري *cbm = (فراغ*)tbl;
        tbl += GET_FARVAR(0, cbm->سائيز);
        إذا (GET_FARVAR(0, cbm->ٽيگ) == ٽيگ)
            أرجع cbm;
    }
    أرجع عدم;
}

بنية cb_سرو *
ڳوليو_cb_جدول(فراغ)
{
    بنية cb_سرو *cbh = ڳوليو_cb_سرو(0, 0x1000);
    إذا (!cbh)
        أرجع عدم;
    بنية cb_forward *cbf = ڳوليو_cb_subtable(cbh, CB_ٽيگ_FORWARD);
    إذا (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = ڳوليو_cb_سرو(GET_FARVAR(0, cbf->forward), 0x100);
        إذا (!cbh)
            أرجع عدم;
    }
    أرجع cbh;
}

ساكن بنية cb_يادگيري *CBMem_جدول;
ثابت حرف *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
فراغ
coreboot_preinit(فراغ)
{
    إذا (!CONFIG_COREBOOT)
        أرجع;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    بنية cb_سرو *cbh = ڳوليو_cb_جدول();
    إذا (!cbh)
        اذهب fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    بنية cb_يادگيري *cbm = CBMem_جدول = ڳوليو_cb_subtable(cbh, CB_ٽيگ_يادگيري);
    إذا (!cbm)
        اذهب fail;

    صحيح i, ڳڻپ = MEM_RANGE_ڳڻپ(cbm);
    لكل (i=0; i<ڳڻپ; i++) {
        بنية cb_يادگيري_range *m = &cbm->map[i];
        u32 type = m->type;
        إذا (type == CB_MEM_جدول)
            type = E820_RESERVED;
        e820_جوڙيو(m->شروعات, m->سائيز, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_جوڙيو(0, 16*1024, E820_RAM);

    بنية cb_cbmem_ref *cbref = ڳوليو_cb_subtable(cbh, CB_ٽيگ_CBMEM_CONSOLE);
    إذا (cbref) {
        cbcon = (فراغ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    بنية cb_mainboard *cbmb = ڳوليو_cb_subtable(cbh, CB_ٽيگ_MAINBOARD);
    إذا (cbmb) {
        CBvendor = &cbmb->لڙيون[cbmb->vendor_idx];
        CBpart = &cbmb->لڙيون[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    أرجع;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_جوڙيو(0, 16*1024*1024, E820_RAM);
    أرجع;
}

فراغ coreboot_debug_putc(حرف c)
{
    إذا (!CONFIG_DEBUG_COREBOOT)
        أرجع;
    إذا (!cbcon)
        أرجع;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_نقاب;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_نقاب;
    إذا (cursor >= cbcon->سائيز)
        أرجع; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    إذا (cursor >= cbcon->سائيز) {
        cursor = 0;
        flags |= CBMC_حد_کان_مٿي;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
ساكن فراغ
جاچ_tables(u32 شروعات, u32 سائيز)
{
    فراغ *p = (فراغ*)ALIGN(شروعات, 16);
    فراغ *پڇاڙي = (فراغ*)شروعات + سائيز;
    لكل (; p<پڇاڙي; p += 16)
        copy_جدول(p);
}

فراغ
coreboot_platform_تياري(فراغ)
{
    إذا (!CONFIG_COREBOOT)
        أرجع;
    pci_probe_devices();

    بنية cb_يادگيري *cbm = CBMem_جدول;
    إذا (!cbm)
        أرجع;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    صحيح i, ڳڻپ = MEM_RANGE_ڳڻپ(cbm);
    لكل (i=0; i<ڳڻپ; i++) {
        بنية cb_يادگيري_range *m = &cbm->map[i];
        إذا (m->type == CB_MEM_جدول)
            جاچ_tables(m->شروعات, m->سائيز);
    }

    ڳوليو_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ساكن صحيح
ulzma(u8 *dst, u32 maxlen, ثابت u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    صحيح ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_سائيز);
    إذا (ret != LZMA_نتيجو_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        أرجع -1;
    }
    u8 scratch[15980];
    صحيح need = (Lzma_Get_انگ_Probs(&state.Properties) * حجم(CProb));
    إذا (need > حجم(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (بنانشان صحيح)حجم(scratch));
        أرجع -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_سائيز);
    إذا (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        أرجع -1;
    }
    u32 اندريون_Processed, ٻاهريون_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_سائيز + 8, srclen
                     , &اندريون_Processed, dst, dstlen, &ٻاهريون_Processed);
    إذا (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        أرجع -1;
    }
    أرجع dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#وصف_ڪريو CBFS_سرو_MAGIC 0x4F524243
#وصف_ڪريو CBFS_VERSION1 0x31313131

بنية cbfs_سرو {
    u32 magic;
    u32 نسخو_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 وٿي_2;
    u32 pad[2];
} PACKED;

#وصف_ڪريو CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

بنية cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 وٿي_2;
    حرف filename[0];
} PACKED;

بنية cbfs_romfile_s {
    بنية romfile_s file;
    بنية cbfs_file *fhdr;
    فراغ *معلومات;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ساكن صحيح
cbfs_copyfile(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع -1;

    بنية cbfs_romfile_s *cfile;
    cfile = container_جو(file, بنية cbfs_romfile_s, file);
    u32 سائيز = cfile->rawsize;
    فراغ *src = cfile->معلومات;
    إذا (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        فراغ *عارضي = malloc_tmphigh(سائيز);
        إذا (!عارضي) {
            warn_noalloc();
            أرجع -1;
        }
        iomemcpy(عارضي, src, سائيز);
        صحيح ret = ulzma(dst, maxlen, عارضي, سائيز);
        yield();
        حرر(عارضي);
        أرجع ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", سائيز, src, maxlen, dst);
    إذا (سائيز > maxlen) {
        warn_noalloc();
        أرجع -1;
    }
    iomemcpy(dst, src, سائيز);
    أرجع سائيز;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ساكن فراغ
process_links_file(فراغ)
{
    حرف *links = romfile_loadfile("links", عدم), *ايندڙ = links;
    طالما (ايندڙ) {
        // Parse out linkname and destname
        حرف *linkname = ايندڙ;
        ايندڙ = strchr(linkname, '\n');
        إذا (ايندڙ)
            *ايندڙ++ = '\0';
        حرف *comment = strchr(linkname, '#');
        إذا (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        حرف *destname = strchr(linkname, ' ');
        إذا (!destname)
            استمر;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        بنية romfile_s *ufile = romfile_ڳوليو(destname);
        إذا (!ufile)
            استمر;
        بنية cbfs_romfile_s *cufile
            = container_جو(ufile, بنية cbfs_romfile_s, file);
        بنية cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        إذا (!cfile) {
            warn_noalloc();
            اكسر;
        }
        يادگيري_نقل(cfile, cufile, حجم(*cfile));
        strtcpy(cfile->file.name, linkname, حجم(cfile->file.name));
        romfile_جوڙيو(&cfile->file);
    }
    حرر(links);
}

فراغ
coreboot_cbfs_init(فراغ)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع;

    بنية cbfs_سرو *hdr = *(فراغ **)(CONFIG_CBFS_LOCATION - 4);
    إذا ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        أرجع;
    }
    إذا (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (فراغ*)hdr + CONFIG_CBFS_LOCATION;
    إذا (hdr->magic != cpu_منزل_be32(CBFS_سرو_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_منزل_be32(CBFS_سرو_MAGIC));
        أرجع;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_منزل_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    بنية cbfs_file *fhdr = (فراغ*)romstart + be32_منزل_cpu(hdr->وٿي_2);
    لكل (;;) {
        إذا ((u32)fhdr - romstart > romsize)
            اكسر;
        u64 magic = fhdr->magic;
        إذا (magic != CBFS_FILE_MAGIC)
            اكسر;
        بنية cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        إذا (!cfile) {
            warn_noalloc();
            اكسر;
        }
        memset(cfile, 0, حجم(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, حجم(cfile->file.name));
        cfile->file.سائيز = cfile->rawsize = be32_منزل_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->معلومات = (فراغ*)fhdr + be32_منزل_cpu(fhdr->وٿي_2);
        صحيح len = لڙي_ڊيگهه(cfile->file.name);
        إذا (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.سائيز = *(u32*)(cfile->معلومات + LZMA_PROPERTIES_سائيز);
        }
        romfile_جوڙيو(&cfile->file);

        fhdr = (فراغ*)ALIGN((u32)cfile->معلومات + cfile->rawsize
                            , be32_منزل_cpu(hdr->align));
    }

    process_links_file();
}

بنية cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 وٿي_2;
    u64 ڀريو_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#وصف_ڪريو PAYLOAD_SEGMENT_BSS    0x20535342
#وصف_ڪريو PAYLOAD_SEGMENT_داخلا  0x52544E45

#وصف_ڪريو CBFS_COMPRESS_NONE  0
#وصف_ڪريو CBFS_COMPRESS_LZMA  1

بنية cbfs_payload {
    بنية cbfs_payload_segment segments[1];
};

فراغ
cbfs_لاڳيتو_payload(بنية cbfs_file *fhdr)
{
    إذا (!CONFIG_COREBOOT_FLASH || !fhdr)
        أرجع;
    dprintf(1, "Run %s\n", fhdr->filename);
    بنية cbfs_payload *pay = (فراغ*)fhdr + be32_منزل_cpu(fhdr->وٿي_2);
    بنية cbfs_payload_segment *seg = pay->segments;
    لكل (;;) {
        فراغ *src = (فراغ*)pay + be32_منزل_cpu(seg->وٿي_2);
        فراغ *dest = (فراغ*)(u32)be64_منزل_cpu(seg->ڀريو_addr);
        u32 src_len = be32_منزل_cpu(seg->len);
        u32 dest_len = be32_منزل_cpu(seg->mem_len);
        بدل (seg->type) {
        حالة PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            اكسر;
        حالة PAYLOAD_SEGMENT_داخلا: {
            dprintf(1, "Calling addr %p\n", dest);
            فراغ (*func)(فراغ) = dest;
            func();
            أرجع;
        }
        افتراضي:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            إذا (seg->compression == cpu_منزل_be32(CBFS_COMPRESS_NONE)) {
                إذا (src_len > dest_len)
                    src_len = dest_len;
                يادگيري_نقل(dest, src, src_len);
            } وإلا إذا (CONFIG_LZMA
                       && seg->compression == cpu_منزل_be32(CBFS_COMPRESS_LZMA)) {
                صحيح ret = ulzma(dest, dest_len, src, src_len);
                إذا (ret < 0)
                    أرجع;
                src_len = ret;
            } وإلا {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                أرجع;
            }
            إذا (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            اكسر;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
فراغ
cbfs_payload_تياري(فراغ)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع;
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix("img/", file);
        إذا (!file)
            اكسر;
        بنية cbfs_romfile_s *cfile;
        cfile = container_جو(file, بنية cbfs_romfile_s, file);
        ثابت حرف *filename = file->name;
        حرف *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_جوڙيو_cbfs(cfile->fhdr, desc, bootprio_ڳوليو_named_rom(filename, 0));
    }
}
