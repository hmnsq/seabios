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

بنية cb_سرآیند {
    u32 signature;
    u32 سرآیند_بایتها;
    u32 سرآیند_checksum;
    u32 جدول_بایتها;
    u32 جدول_checksum;
    u32 جدول_entries;
};

#تعریف_کن CB_SIGNATURE 0x4f49424C // "LBIO"

بنية cb_حافظه_range {
    u64 شروع;
    u64 اندازه;
    u32 type;
};

#تعریف_کن CB_MEM_جدول    16

بنية cb_حافظه {
    u32 برچسب;
    u32 اندازه;
    بنية cb_حافظه_range map[0];
};

#تعریف_کن CB_برچسب_حافظه 0x01

#تعریف_کن MEM_RANGE_شمار(_rec) \
        (((_rec)->اندازه - حجم(*(_rec))) / حجم((_rec)->map[0]))

بنية cb_mainboard {
    u32 برچسب;
    u32 اندازه;
    u8  vendor_idx;
    u8  part_idx;
    حرف  رشتهها[0];
};

#تعریف_کن CB_برچسب_MAINBOARD 0x0003

بنية cb_forward {
    u32 برچسب;
    u32 اندازه;
    u64 forward;
};

#تعریف_کن CB_برچسب_FORWARD 0x11

بنية cb_cbmem_ref {
    u32 برچسب;
    u32 اندازه;
    u64 cbmem_addr;
};

#تعریف_کن CB_برچسب_CBMEM_CONSOLE 0x17

بنية cbmem_console {
    u32 اندازه;
    u32 cursor;
    u8  body[0];
} PACKED;
#تعریف_کن CBMC_CURSOR_ماسک ((1 << 28) - 1)
#تعریف_کن CBMC_سرریز (1 << 31)
ساكن بنية cbmem_console *cbcon = عدم;

ساكن u16
ipchksum(حرف *buf, صحيح شمار)
{
    u16 *p = (u16*)buf;
    u32 جمع = 0;
    طالما (شمار > 1) {
        جمع += GET_FARVAR(0, *p);
        p++;
        شمار -= 2;
    }
    إذا (شمار)
        جمع += GET_FARVAR(0, *(u8*)p);
    جمع = (جمع >> 16) + (جمع & 0xffff);
    جمع += (جمع >> 16);
    أرجع ~جمع;
}

// Try to locate the coreboot header in a given address range.
ساكن بنية cb_سرآیند *
پیدا_کن_cb_سرآیند(u32 addr, صحيح len)
{
    u32 انتها = addr + len;
    لكل (; addr < انتها; addr += 16) {
        بنية cb_سرآیند *cbh = (فراغ*)addr;
        إذا (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            استمر;
        u32 tsize = GET_FARVAR(0, cbh->جدول_بایتها);
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
پیدا_کن_cb_subtable(بنية cb_سرآیند *cbh, u32 برچسب)
{
    حرف *tbl = (حرف *)cbh + حجم(*cbh);
    u32 شمار = GET_FARVAR(0, cbh->جدول_entries);
    صحيح i;
    لكل (i=0; i<شمار; i++) {
        بنية cb_حافظه *cbm = (فراغ*)tbl;
        tbl += GET_FARVAR(0, cbm->اندازه);
        إذا (GET_FARVAR(0, cbm->برچسب) == برچسب)
            أرجع cbm;
    }
    أرجع عدم;
}

بنية cb_سرآیند *
پیدا_کن_cb_جدول(فراغ)
{
    بنية cb_سرآیند *cbh = پیدا_کن_cb_سرآیند(0, 0x1000);
    إذا (!cbh)
        أرجع عدم;
    بنية cb_forward *cbf = پیدا_کن_cb_subtable(cbh, CB_برچسب_FORWARD);
    إذا (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = پیدا_کن_cb_سرآیند(GET_FARVAR(0, cbf->forward), 0x100);
        إذا (!cbh)
            أرجع عدم;
    }
    أرجع cbh;
}

ساكن بنية cb_حافظه *CBMem_جدول;
ثابت حرف *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
فراغ
coreboot_preinit(فراغ)
{
    إذا (!CONFIG_COREBOOT)
        أرجع;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    بنية cb_سرآیند *cbh = پیدا_کن_cb_جدول();
    إذا (!cbh)
        اذهب fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    بنية cb_حافظه *cbm = CBMem_جدول = پیدا_کن_cb_subtable(cbh, CB_برچسب_حافظه);
    إذا (!cbm)
        اذهب fail;

    صحيح i, شمار = MEM_RANGE_شمار(cbm);
    لكل (i=0; i<شمار; i++) {
        بنية cb_حافظه_range *m = &cbm->map[i];
        u32 type = m->type;
        إذا (type == CB_MEM_جدول)
            type = E820_RESERVED;
        e820_جمع_کن(m->شروع, m->اندازه, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_جمع_کن(0, 16*1024, E820_RAM);

    بنية cb_cbmem_ref *cbref = پیدا_کن_cb_subtable(cbh, CB_برچسب_CBMEM_CONSOLE);
    إذا (cbref) {
        cbcon = (فراغ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    بنية cb_mainboard *cbmb = پیدا_کن_cb_subtable(cbh, CB_برچسب_MAINBOARD);
    إذا (cbmb) {
        CBvendor = &cbmb->رشتهها[cbmb->vendor_idx];
        CBpart = &cbmb->رشتهها[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    أرجع;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_جمع_کن(0, 16*1024*1024, E820_RAM);
    أرجع;
}

فراغ coreboot_debug_putc(حرف c)
{
    إذا (!CONFIG_DEBUG_COREBOOT)
        أرجع;
    إذا (!cbcon)
        أرجع;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_ماسک;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_ماسک;
    إذا (cursor >= cbcon->اندازه)
        أرجع; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    إذا (cursor >= cbcon->اندازه) {
        cursor = 0;
        flags |= CBMC_سرریز;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
ساكن فراغ
پیمایش_tables(u32 شروع, u32 اندازه)
{
    فراغ *p = (فراغ*)ALIGN(شروع, 16);
    فراغ *انتها = (فراغ*)شروع + اندازه;
    لكل (; p<انتها; p += 16)
        copy_جدول(p);
}

فراغ
coreboot_platform_راهاندازی(فراغ)
{
    إذا (!CONFIG_COREBOOT)
        أرجع;
    pci_probe_devices();

    بنية cb_حافظه *cbm = CBMem_جدول;
    إذا (!cbm)
        أرجع;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    صحيح i, شمار = MEM_RANGE_شمار(cbm);
    لكل (i=0; i<شمار; i++) {
        بنية cb_حافظه_range *m = &cbm->map[i];
        إذا (m->type == CB_MEM_جدول)
            پیمایش_tables(m->شروع, m->اندازه);
    }

    پیدا_کن_acpi_features();
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
    صحيح ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_اندازه);
    إذا (ret != LZMA_نتیجه_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        أرجع -1;
    }
    u8 scratch[15980];
    صحيح need = (Lzma_Get_رقم_Probs(&state.Properties) * حجم(CProb));
    إذا (need > حجم(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (بیعلامت صحيح)حجم(scratch));
        أرجع -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_اندازه);
    إذا (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        أرجع -1;
    }
    u32 درون_Processed, بیرون_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_اندازه + 8, srclen
                     , &درون_Processed, dst, dstlen, &بیرون_Processed);
    إذا (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        أرجع -1;
    }
    أرجع dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#تعریف_کن CBFS_سرآیند_MAGIC 0x4F524243
#تعریف_کن CBFS_VERSION1 0x31313131

بنية cbfs_سرآیند {
    u32 magic;
    u32 نسخه_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 جابه_جایی_2;
    u32 pad[2];
} PACKED;

#تعریف_کن CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

بنية cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 جابه_جایی_2;
    حرف filename[0];
} PACKED;

بنية cbfs_romfile_s {
    بنية romfile_s file;
    بنية cbfs_file *fhdr;
    فراغ *داده;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ساكن صحيح
cbfs_copyfile(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع -1;

    بنية cbfs_romfile_s *cfile;
    cfile = container_از(file, بنية cbfs_romfile_s, file);
    u32 اندازه = cfile->rawsize;
    فراغ *src = cfile->داده;
    إذا (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        فراغ *موقت = malloc_tmphigh(اندازه);
        إذا (!موقت) {
            warn_noalloc();
            أرجع -1;
        }
        iomemcpy(موقت, src, اندازه);
        صحيح ret = ulzma(dst, maxlen, موقت, اندازه);
        yield();
        حرر(موقت);
        أرجع ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", اندازه, src, maxlen, dst);
    إذا (اندازه > maxlen) {
        warn_noalloc();
        أرجع -1;
    }
    iomemcpy(dst, src, اندازه);
    أرجع اندازه;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ساكن فراغ
process_links_file(فراغ)
{
    حرف *links = romfile_loadfile("links", عدم), *بعدی = links;
    طالما (بعدی) {
        // Parse out linkname and destname
        حرف *linkname = بعدی;
        بعدی = strchr(linkname, '\n');
        إذا (بعدی)
            *بعدی++ = '\0';
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
        بنية romfile_s *ufile = romfile_پیدا_کن(destname);
        إذا (!ufile)
            استمر;
        بنية cbfs_romfile_s *cufile
            = container_از(ufile, بنية cbfs_romfile_s, file);
        بنية cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        إذا (!cfile) {
            warn_noalloc();
            اكسر;
        }
        کپی_حافظه(cfile, cufile, حجم(*cfile));
        strtcpy(cfile->file.name, linkname, حجم(cfile->file.name));
        romfile_جمع_کن(&cfile->file);
    }
    حرر(links);
}

فراغ
coreboot_cbfs_init(فراغ)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع;

    بنية cbfs_سرآیند *hdr = *(فراغ **)(CONFIG_CBFS_LOCATION - 4);
    إذا ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        أرجع;
    }
    إذا (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (فراغ*)hdr + CONFIG_CBFS_LOCATION;
    إذا (hdr->magic != cpu_مقصد_be32(CBFS_سرآیند_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_مقصد_be32(CBFS_سرآیند_MAGIC));
        أرجع;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_مقصد_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    بنية cbfs_file *fhdr = (فراغ*)romstart + be32_مقصد_cpu(hdr->جابه_جایی_2);
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
        cfile->file.اندازه = cfile->rawsize = be32_مقصد_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->داده = (فراغ*)fhdr + be32_مقصد_cpu(fhdr->جابه_جایی_2);
        صحيح len = طول_رشته(cfile->file.name);
        إذا (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.اندازه = *(u32*)(cfile->داده + LZMA_PROPERTIES_اندازه);
        }
        romfile_جمع_کن(&cfile->file);

        fhdr = (فراغ*)ALIGN((u32)cfile->داده + cfile->rawsize
                            , be32_مقصد_cpu(hdr->align));
    }

    process_links_file();
}

بنية cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 جابه_جایی_2;
    u64 بارگیری_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#تعریف_کن PAYLOAD_SEGMENT_BSS    0x20535342
#تعریف_کن PAYLOAD_SEGMENT_مدخل  0x52544E45

#تعریف_کن CBFS_COMPRESS_NONE  0
#تعریف_کن CBFS_COMPRESS_LZMA  1

بنية cbfs_payload {
    بنية cbfs_payload_segment segments[1];
};

فراغ
cbfs_پیاپی_payload(بنية cbfs_file *fhdr)
{
    إذا (!CONFIG_COREBOOT_FLASH || !fhdr)
        أرجع;
    dprintf(1, "Run %s\n", fhdr->filename);
    بنية cbfs_payload *pay = (فراغ*)fhdr + be32_مقصد_cpu(fhdr->جابه_جایی_2);
    بنية cbfs_payload_segment *seg = pay->segments;
    لكل (;;) {
        فراغ *src = (فراغ*)pay + be32_مقصد_cpu(seg->جابه_جایی_2);
        فراغ *dest = (فراغ*)(u32)be64_مقصد_cpu(seg->بارگیری_addr);
        u32 src_len = be32_مقصد_cpu(seg->len);
        u32 dest_len = be32_مقصد_cpu(seg->mem_len);
        بدل (seg->type) {
        حالة PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            اكسر;
        حالة PAYLOAD_SEGMENT_مدخل: {
            dprintf(1, "Calling addr %p\n", dest);
            فراغ (*func)(فراغ) = dest;
            func();
            أرجع;
        }
        افتراضي:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            إذا (seg->compression == cpu_مقصد_be32(CBFS_COMPRESS_NONE)) {
                إذا (src_len > dest_len)
                    src_len = dest_len;
                کپی_حافظه(dest, src, src_len);
            } وإلا إذا (CONFIG_LZMA
                       && seg->compression == cpu_مقصد_be32(CBFS_COMPRESS_LZMA)) {
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
cbfs_payload_راهاندازی(فراغ)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع;
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix("img/", file);
        إذا (!file)
            اكسر;
        بنية cbfs_romfile_s *cfile;
        cfile = container_از(file, بنية cbfs_romfile_s, file);
        ثابت حرف *filename = file->name;
        حرف *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_جمع_کن_cbfs(cfile->fhdr, desc, bootprio_پیدا_کن_named_rom(filename, 0));
    }
}
