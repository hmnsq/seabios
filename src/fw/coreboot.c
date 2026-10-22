// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // MAXDESCSIZE
#شامل "byteorder.h" // be32_to_cpu
#شامل "config.h" // CONFIG_*
#شامل "e820map.h" // e820_add
#شامل "hw/pcidevice.h" // pci_probe_devices
#شامل "lzmadecode.h" // LzmaDecode
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "paravirt.h" // PlatformRunningOn
#شامل "romfile.h" // romfile_findprefix
#شامل "stacks.h" // yield
#شامل "string.h" // memset
#شامل "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ساخت cb_سرخی {
    u32 signature;
    u32 سرخی_بائٹیں;
    u32 سرخی_checksum;
    u32 جدول_بائٹیں;
    u32 جدول_checksum;
    u32 جدول_entries;
};

#تعریف_کرو CB_SIGNATURE 0x4f49424C // "LBIO"

ساخت cb_حافظہ_range {
    u64 آغاز;
    u64 سائز;
    u32 type;
};

#تعریف_کرو CB_MEM_جدول    16

ساخت cb_حافظہ {
    u32 لیبل;
    u32 سائز;
    ساخت cb_حافظہ_range map[0];
};

#تعریف_کرو CB_لیبل_حافظہ 0x01

#تعریف_کرو MEM_RANGE_گنتی(_rec) \
        (((_rec)->سائز - حجم(*(_rec))) / حجم((_rec)->map[0]))

ساخت cb_mainboard {
    u32 لیبل;
    u32 سائز;
    u8  vendor_idx;
    u8  part_idx;
    حرف  لڑیاں[0];
};

#تعریف_کرو CB_لیبل_MAINBOARD 0x0003

ساخت cb_forward {
    u32 لیبل;
    u32 سائز;
    u64 forward;
};

#تعریف_کرو CB_لیبل_FORWARD 0x11

ساخت cb_cbmem_ref {
    u32 لیبل;
    u32 سائز;
    u64 cbmem_addr;
};

#تعریف_کرو CB_لیبل_CBMEM_CONSOLE 0x17

ساخت cbmem_console {
    u32 سائز;
    u32 cursor;
    u8  body[0];
} PACKED;
#تعریف_کرو CBMC_CURSOR_نقاب ((1 << 28) - 1)
#تعریف_کرو CBMC_حد_سے_تجاوز (1 << 31)
ساکن ساخت cbmem_console *cbcon = تہی;

ساکن u16
ipchksum(حرف *buf, صحیح گنتی)
{
    u16 *p = (u16*)buf;
    u32 مجموع = 0;
    جب (گنتی > 1) {
        مجموع += GET_FARVAR(0, *p);
        p++;
        گنتی -= 2;
    }
    اگر (گنتی)
        مجموع += GET_FARVAR(0, *(u8*)p);
    مجموع = (مجموع >> 16) + (مجموع & 0xffff);
    مجموع += (مجموع >> 16);
    واپس ~مجموع;
}

// Try to locate the coreboot header in a given address range.
ساکن ساخت cb_سرخی *
ڈھونڈو_cb_سرخی(u32 addr, صحیح len)
{
    u32 انتہا = addr + len;
    برائے (; addr < انتہا; addr += 16) {
        ساخت cb_سرخی *cbh = (خالی*)addr;
        اگر (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            جاری;
        u32 tsize = GET_FARVAR(0, cbh->جدول_بائٹیں);
        اگر (! tsize)
            جاری;
        اگر (ipchksum((خالی*)addr, حجم(*cbh)) != 0)
            جاری;
        اگر (ipchksum((خالی*)addr + حجم(*cbh), tsize)
            != GET_FARVAR(0, cbh->جدول_checksum))
            جاری;
        واپس cbh;
    }
    واپس تہی;
}

// Try to find the coreboot memory table in the given coreboot table.
خالی *
ڈھونڈو_cb_subtable(ساخت cb_سرخی *cbh, u32 لیبل)
{
    حرف *tbl = (حرف *)cbh + حجم(*cbh);
    u32 گنتی = GET_FARVAR(0, cbh->جدول_entries);
    صحیح i;
    برائے (i=0; i<گنتی; i++) {
        ساخت cb_حافظہ *cbm = (خالی*)tbl;
        tbl += GET_FARVAR(0, cbm->سائز);
        اگر (GET_FARVAR(0, cbm->لیبل) == لیبل)
            واپس cbm;
    }
    واپس تہی;
}

ساخت cb_سرخی *
ڈھونڈو_cb_جدول(خالی)
{
    ساخت cb_سرخی *cbh = ڈھونڈو_cb_سرخی(0, 0x1000);
    اگر (!cbh)
        واپس تہی;
    ساخت cb_forward *cbf = ڈھونڈو_cb_subtable(cbh, CB_لیبل_FORWARD);
    اگر (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = ڈھونڈو_cb_سرخی(GET_FARVAR(0, cbf->forward), 0x100);
        اگر (!cbh)
            واپس تہی;
    }
    واپس cbh;
}

ساکن ساخت cb_حافظہ *CBMem_جدول;
ثابت حرف *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
خالی
coreboot_preinit(خالی)
{
    اگر (!CONFIG_COREBOOT)
        واپس;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ساخت cb_سرخی *cbh = ڈھونڈو_cb_جدول();
    اگر (!cbh)
        جاؤ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ساخت cb_حافظہ *cbm = CBMem_جدول = ڈھونڈو_cb_subtable(cbh, CB_لیبل_حافظہ);
    اگر (!cbm)
        جاؤ fail;

    صحیح i, گنتی = MEM_RANGE_گنتی(cbm);
    برائے (i=0; i<گنتی; i++) {
        ساخت cb_حافظہ_range *m = &cbm->map[i];
        u32 type = m->type;
        اگر (type == CB_MEM_جدول)
            type = E820_RESERVED;
        e820_جوڑو(m->آغاز, m->سائز, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_جوڑو(0, 16*1024, E820_RAM);

    ساخت cb_cbmem_ref *cbref = ڈھونڈو_cb_subtable(cbh, CB_لیبل_CBMEM_CONSOLE);
    اگر (cbref) {
        cbcon = (خالی*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ساخت cb_mainboard *cbmb = ڈھونڈو_cb_subtable(cbh, CB_لیبل_MAINBOARD);
    اگر (cbmb) {
        CBvendor = &cbmb->لڑیاں[cbmb->vendor_idx];
        CBpart = &cbmb->لڑیاں[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    واپس;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_جوڑو(0, 16*1024*1024, E820_RAM);
    واپس;
}

خالی coreboot_debug_putc(حرف c)
{
    اگر (!CONFIG_DEBUG_COREBOOT)
        واپس;
    اگر (!cbcon)
        واپس;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_نقاب;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_نقاب;
    اگر (cursor >= cbcon->سائز)
        واپس; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    اگر (cursor >= cbcon->سائز) {
        cursor = 0;
        flags |= CBMC_حد_سے_تجاوز;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
ساکن خالی
جائزہ_tables(u32 آغاز, u32 سائز)
{
    خالی *p = (خالی*)ALIGN(آغاز, 16);
    خالی *انتہا = (خالی*)آغاز + سائز;
    برائے (; p<انتہا; p += 16)
        copy_جدول(p);
}

خالی
coreboot_platform_تیاری(خالی)
{
    اگر (!CONFIG_COREBOOT)
        واپس;
    pci_probe_devices();

    ساخت cb_حافظہ *cbm = CBMem_جدول;
    اگر (!cbm)
        واپس;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    صحیح i, گنتی = MEM_RANGE_گنتی(cbm);
    برائے (i=0; i<گنتی; i++) {
        ساخت cb_حافظہ_range *m = &cbm->map[i];
        اگر (m->type == CB_MEM_جدول)
            جائزہ_tables(m->آغاز, m->سائز);
    }

    ڈھونڈو_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ساکن صحیح
ulzma(u8 *dst, u32 maxlen, ثابت u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    صحیح ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_سائز);
    اگر (ret != LZMA_نتیجہ_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        واپس -1;
    }
    u8 scratch[15980];
    صحیح need = (Lzma_Get_عدد_Probs(&state.Properties) * حجم(CProb));
    اگر (need > حجم(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (غیرعلامتی صحیح)حجم(scratch));
        واپس -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_سائز);
    اگر (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        واپس -1;
    }
    u32 اندر_Processed, برآمد_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_سائز + 8, srclen
                     , &اندر_Processed, dst, dstlen, &برآمد_Processed);
    اگر (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        واپس -1;
    }
    واپس dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#تعریف_کرو CBFS_سرخی_MAGIC 0x4F524243
#تعریف_کرو CBFS_VERSION1 0x31313131

ساخت cbfs_سرخی {
    u32 magic;
    u32 نسخہ_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 انحراف_2;
    u32 pad[2];
} PACKED;

#تعریف_کرو CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ساخت cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 انحراف_2;
    حرف filename[0];
} PACKED;

ساخت cbfs_romfile_s {
    ساخت romfile_s file;
    ساخت cbfs_file *fhdr;
    خالی *ڈیٹا;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ساکن صحیح
cbfs_copyfile(ساخت romfile_s *file, خالی *dst, u32 maxlen)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        واپس -1;

    ساخت cbfs_romfile_s *cfile;
    cfile = container_کا(file, ساخت cbfs_romfile_s, file);
    u32 سائز = cfile->rawsize;
    خالی *src = cfile->ڈیٹا;
    اگر (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        خالی *عارضی = malloc_tmphigh(سائز);
        اگر (!عارضی) {
            warn_noalloc();
            واپس -1;
        }
        iomemcpy(عارضی, src, سائز);
        صحیح ret = ulzma(dst, maxlen, عارضی, سائز);
        yield();
        آزاد(عارضی);
        واپس ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", سائز, src, maxlen, dst);
    اگر (سائز > maxlen) {
        warn_noalloc();
        واپس -1;
    }
    iomemcpy(dst, src, سائز);
    واپس سائز;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ساکن خالی
process_links_file(خالی)
{
    حرف *links = romfile_loadfile("links", تہی), *اگلا = links;
    جب (اگلا) {
        // Parse out linkname and destname
        حرف *linkname = اگلا;
        اگلا = strchr(linkname, '\n');
        اگر (اگلا)
            *اگلا++ = '\0';
        حرف *comment = strchr(linkname, '#');
        اگر (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        حرف *destname = strchr(linkname, ' ');
        اگر (!destname)
            جاری;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ساخت romfile_s *ufile = romfile_ڈھونڈو(destname);
        اگر (!ufile)
            جاری;
        ساخت cbfs_romfile_s *cufile
            = container_کا(ufile, ساخت cbfs_romfile_s, file);
        ساخت cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            توڑو;
        }
        حافظہ_نقل(cfile, cufile, حجم(*cfile));
        strtcpy(cfile->file.name, linkname, حجم(cfile->file.name));
        romfile_جوڑو(&cfile->file);
    }
    آزاد(links);
}

خالی
coreboot_cbfs_init(خالی)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        واپس;

    ساخت cbfs_سرخی *hdr = *(خالی **)(CONFIG_CBFS_LOCATION - 4);
    اگر ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        واپس;
    }
    اگر (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (خالی*)hdr + CONFIG_CBFS_LOCATION;
    اگر (hdr->magic != cpu_مقصد_be32(CBFS_سرخی_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_مقصد_be32(CBFS_سرخی_MAGIC));
        واپس;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_مقصد_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ساخت cbfs_file *fhdr = (خالی*)romstart + be32_مقصد_cpu(hdr->انحراف_2);
    برائے (;;) {
        اگر ((u32)fhdr - romstart > romsize)
            توڑو;
        u64 magic = fhdr->magic;
        اگر (magic != CBFS_FILE_MAGIC)
            توڑو;
        ساخت cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            توڑو;
        }
        memset(cfile, 0, حجم(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, حجم(cfile->file.name));
        cfile->file.سائز = cfile->rawsize = be32_مقصد_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->ڈیٹا = (خالی*)fhdr + be32_مقصد_cpu(fhdr->انحراف_2);
        صحیح len = لڑی_لمبائی(cfile->file.name);
        اگر (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.سائز = *(u32*)(cfile->ڈیٹا + LZMA_PROPERTIES_سائز);
        }
        romfile_جوڑو(&cfile->file);

        fhdr = (خالی*)ALIGN((u32)cfile->ڈیٹا + cfile->rawsize
                            , be32_مقصد_cpu(hdr->align));
    }

    process_links_file();
}

ساخت cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 انحراف_2;
    u64 بھرو_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#تعریف_کرو PAYLOAD_SEGMENT_BSS    0x20535342
#تعریف_کرو PAYLOAD_SEGMENT_اندراج  0x52544E45

#تعریف_کرو CBFS_COMPRESS_NONE  0
#تعریف_کرو CBFS_COMPRESS_LZMA  1

ساخت cbfs_payload {
    ساخت cbfs_payload_segment segments[1];
};

خالی
cbfs_تسلسل_payload(ساخت cbfs_file *fhdr)
{
    اگر (!CONFIG_COREBOOT_FLASH || !fhdr)
        واپس;
    dprintf(1, "Run %s\n", fhdr->filename);
    ساخت cbfs_payload *pay = (خالی*)fhdr + be32_مقصد_cpu(fhdr->انحراف_2);
    ساخت cbfs_payload_segment *seg = pay->segments;
    برائے (;;) {
        خالی *src = (خالی*)pay + be32_مقصد_cpu(seg->انحراف_2);
        خالی *dest = (خالی*)(u32)be64_مقصد_cpu(seg->بھرو_addr);
        u32 src_len = be32_مقصد_cpu(seg->len);
        u32 dest_len = be32_مقصد_cpu(seg->mem_len);
        انتخاب (seg->type) {
        حالت PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            توڑو;
        حالت PAYLOAD_SEGMENT_اندراج: {
            dprintf(1, "Calling addr %p\n", dest);
            خالی (*func)(خالی) = dest;
            func();
            واپس;
        }
        طےشدہ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            اگر (seg->compression == cpu_مقصد_be32(CBFS_COMPRESS_NONE)) {
                اگر (src_len > dest_len)
                    src_len = dest_len;
                حافظہ_نقل(dest, src, src_len);
            } ورنہ اگر (CONFIG_LZMA
                       && seg->compression == cpu_مقصد_be32(CBFS_COMPRESS_LZMA)) {
                صحیح ret = ulzma(dest, dest_len, src, src_len);
                اگر (ret < 0)
                    واپس;
                src_len = ret;
            } ورنہ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                واپس;
            }
            اگر (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            توڑو;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
خالی
cbfs_payload_تیاری(خالی)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        واپس;
    ساخت romfile_s *file = تہی;
    برائے (;;) {
        file = romfile_findprefix("img/", file);
        اگر (!file)
            توڑو;
        ساخت cbfs_romfile_s *cfile;
        cfile = container_کا(file, ساخت cbfs_romfile_s, file);
        ثابت حرف *filename = file->name;
        حرف *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_جوڑو_cbfs(cfile->fhdr, desc, bootprio_ڈھونڈو_named_rom(filename, 0));
    }
}
