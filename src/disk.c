// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ata.h" // ATA_CB_DC
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_eoi2
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // debug_enter
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // call16_int
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
__disk_ret(ਬਣਤਰ bregs *regs, u32 linecode, ਅਚਲ ਅੱਖਰ *fname)
{
    u8 ਕੋਡ = linecode;
    ਜੇ (regs->dl < EXTSTART_HD)
        ਸੈੱਟ_BDA(floppy_ਆਖਰੀ_ਸਥਿਤੀ, ਕੋਡ);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_BDA(disk_ਆਖਰੀ_ਸਥਿਤੀ, ਕੋਡ);
    ਜੇ (ਕੋਡ)
        __set_code_invalid(regs, linecode, fname);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
__disk_ret_unimplemented(ਬਣਤਰ bregs *regs, u32 linecode, ਅਚਲ ਅੱਖਰ *fname)
{
    u8 ਕੋਡ = linecode;
    ਜੇ (regs->dl < EXTSTART_HD)
        ਸੈੱਟ_BDA(floppy_ਆਖਰੀ_ਸਥਿਤੀ, ਕੋਡ);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_BDA(disk_ਆਖਰੀ_ਸਥਿਤੀ, ਕੋਡ);
    __set_code_unimplemented(regs, linecode, fname);
}

ਸਥਿਰ ਖਾਲੀ
__disk_stub(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disk_ret(regs, ਕੋਡ) \
    __disk_ret((regs), (ਕੋਡ) | (__LINE__ << 8), __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disk_ret_unimplemented(regs, ਕੋਡ) \
    __disk_ret_unimplemented((regs), (ਕੋਡ) | (__LINE__ << 8), __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
ਸਥਿਰ ਬਣਤਰ chs_s
getLCHS(ਬਣਤਰ drive_s *drive_fl)
{
    ਬਣਤਰ chs_s res = { };
    ਜੇ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_ਨੀਵਾਂ(CDEmu.chs.sptcyl);
        res.cylinder = GET_ਨੀਵਾਂ(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.ਸਿਰ = GET_ਨੀਵਾਂ(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        ਵਾਪਸ res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.ਸਿਰ = GET_FLATPTR(drive_fl->lchs.ਸਿਰ);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    ਵਾਪਸ res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ਸਥਿਰ ਪੂਰਨ
__send_disk_op(ਬਣਤਰ disk_op_s *op_far, u16 op_seg)
{
    ਬਣਤਰ disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, ਆਕਾਰ(dop));

    ਪੂਰਨ ਸਥਿਤੀ = process_op(&dop);

    // Update count with total sectors transferred.
    ਸੈੱਟ_FARVAR(op_seg, op_far->ਗਿਣਤੀ_2, dop.ਗਿਣਤੀ_2);

    ਵਾਪਸ ਸਥਿਤੀ;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ਸਥਿਰ ਪੂਰਨ
send_disk_op(ਬਣਤਰ disk_op_s *op)
{
    ASSERT16();
    ਜੇ (! CONFIG_DRIVES)
        ਵਾਪਸ -1;
    ਜੇ (!CONFIG_ਇੰਦਰਾਜ_EXTRASTACK)
        // Jump on to extra stack
        ਵਾਪਸ ਢੇਰ_hop(__send_disk_op, op, GET_SEG(SS));
    ਵਾਪਸ process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ਸਥਿਰ ਖਾਲੀ noinline
basic_access(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl, u16 command)
{
    ਬਣਤਰ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 ਗਿਣਤੀ_2 = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 ਸਿਰ = regs->dh;

    ਜੇ (ਗਿਣਤੀ_2 > 128 || ਗਿਣਤੀ_2 == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }
    dop.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;

    ਬਣਤਰ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ਸਿਰ, nls=chs.sector;

    // sanity check on cyl heads, sec
    ਜੇ (cylinder >= nlc || ਸਿਰ >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)ਸਿਰ) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    ਪੂਰਨ ਸਥਿਤੀ = send_disk_op(&dop);

    regs->al = dop.ਗਿਣਤੀ_2;

    disk_ret(regs, ਸਥਿਤੀ);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ਸਥਿਰ ਖਾਲੀ noinline
extended_access(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl, u16 command)
{
    ਬਣਤਰ disk_op_s dop;
    ਬਣਤਰ int13ext_s *param_far = (ਬਣਤਰ int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    ਜੇ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }

    dop.buf_fl = SEGOFF_ਮੰਜ਼ਿਲ_FLATPTR(GET_FARVAR(regs->ds, param_far->ਅੰਕੜੇ));
    dop.ਗਿਣਤੀ_2 = GET_FARVAR(regs->ds, param_far->ਗਿਣਤੀ_2);
    ਜੇ (! dop.ਗਿਣਤੀ_2) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        ਵਾਪਸ;
    }

    ਪੂਰਨ ਸਥਿਤੀ = send_disk_op(&dop);

    ਸੈੱਟ_FARVAR(regs->ds, param_far->ਗਿਣਤੀ_2, dop.ਗਿਣਤੀ_2);

    disk_ret(regs, ਸਥਿਤੀ);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ਸਥਿਰ ਖਾਲੀ
disk_1300(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਬਣਤਰ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.ਗਿਣਤੀ_2 = 0;
    ਪੂਰਨ ਸਥਿਤੀ = send_disk_op(&dop);
    disk_ret(regs, ਸਥਿਤੀ);
}

// read disk status
ਸਥਿਰ ਖਾਲੀ
disk_1301(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    u8 v;
    ਜੇ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_ਆਖਰੀ_ਸਥਿਤੀ);
    ਨਹੀਂਤਾਂ
        v = GET_BDA(disk_ਆਖਰੀ_ਸਥਿਤੀ);
    regs->ah = v;
    ਸੈੱਟ_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ਸਥਿਰ ਖਾਲੀ
disk_1302(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ਸਥਿਰ ਖਾਲੀ
disk_1303(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ਸਥਿਰ ਖਾਲੀ
disk_1304(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ਸਥਿਰ ਖਾਲੀ noinline
disk_1305(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    debug_stub(regs);

    ਬਣਤਰ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ਸਿਰ, nls=chs.sector;

    u8 ਗਿਣਤੀ_2 = regs->al;
    u8 cylinder = regs->ch;
    u8 ਸਿਰ = regs->dh;

    ਜੇ (cylinder >= nlc || ਸਿਰ >= nlh || ਗਿਣਤੀ_2 == 0 || ਗਿਣਤੀ_2 > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }

    ਬਣਤਰ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)ਸਿਰ) * (u32)nls;
    dop.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    ਪੂਰਨ ਸਥਿਤੀ = send_disk_op(&dop);
    disk_ret(regs, ਸਥਿਤੀ);
}

// read disk drive parameters
ਸਥਿਰ ਖਾਲੀ noinline
disk_1308(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    // Get logical geometry from table
    ਬਣਤਰ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ਸਿਰ, nls=chs.sector;
    nlc--;
    nlh--;
    u8 ਗਿਣਤੀ_2;
    ਜੇ (regs->dl < EXTSTART_HD) {
        // Floppy
        ਗਿਣਤੀ_2 = GET_GLOBAL(Floppy_ਗਿਣਤੀ);

        ਜੇ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_ਨੀਵਾਂ(CDEmu.media) * 2;
        ਨਹੀਂਤਾਂ
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } ਨਹੀਂਤਾਂ ਜੇ (regs->dl < EXTSTART_CD) {
        // Hard drive
        ਗਿਣਤੀ_2 = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } ਨਹੀਂਤਾਂ {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }

    ਜੇ (CONFIG_CDROM_EMU && GET_ਨੀਵਾਂ(CDEmu.media)) {
        u8 emudrive = GET_ਨੀਵਾਂ(CDEmu.emulated_drive);
        ਜੇ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            ਗਿਣਤੀ_2++;
        ਜੇ (regs->dl < EXTSTART_HD && ਗਿਣਤੀ_2 > 2)
            // Max of two floppy drives.
            ਗਿਣਤੀ_2 = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = ਗਿਣਤੀ_2;
}

// initialize drive parameters
ਸਥਿਰ ਖਾਲੀ
disk_1309(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ਸਥਿਰ ਖਾਲੀ
disk_130c(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ਸਥਿਰ ਖਾਲੀ
disk_130d(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ਸਥਿਰ ਖਾਲੀ
disk_1310(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    // should look at 40:8E also???

    ਬਣਤਰ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.ਗਿਣਤੀ_2 = 0;
    ਪੂਰਨ ਸਥਿਤੀ = send_disk_op(&dop);
    disk_ret(regs, ਸਥਿਤੀ);
}

// recalibrate
ਸਥਿਰ ਖਾਲੀ
disk_1311(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ਸਥਿਰ ਖਾਲੀ
disk_1314(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ਸਥਿਰ ਖਾਲੀ noinline
disk_1315(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    ਜੇ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        ਵਾਪਸ;
    }
    // Hard drive

    // Get logical geometry from table
    ਬਣਤਰ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ਸਿਰ, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ਸਥਿਰ ਖਾਲੀ
disk_1316(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਜੇ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ਸਥਿਰ ਖਾਲੀ
disk_1341(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ਸਥਿਰ ਖਾਲੀ
disk_1342(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ਸਥਿਰ ਖਾਲੀ
disk_1343(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ਸਥਿਰ ਖਾਲੀ
disk_1344(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

//ਤਾਲਾ
ਸਥਿਰ ਖਾਲੀ
disk_134500(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਪੂਰਨ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ਨੀਵਾਂ(CDRom_locks[cdid]);
    ਜੇ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        ਵਾਪਸ;
    }
    ਸੈੱਟ_ਨੀਵਾਂ(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ਸਥਿਰ ਖਾਲੀ
disk_134501(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਪੂਰਨ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ਨੀਵਾਂ(CDRom_locks[cdid]);
    ਜੇ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        ਵਾਪਸ;
    }
    locks--;
    ਸੈੱਟ_ਨੀਵਾਂ(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

//ਸਥਿਤੀ
ਸਥਿਰ ਖਾਲੀ
disk_134502(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਪੂਰਨ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ਨੀਵਾਂ(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ਸਥਿਰ ਖਾਲੀ
disk_1345XX(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ਸਥਿਰ ਖਾਲੀ
disk_1345(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਜੇ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ਵਾਪਸ;
    }

    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x00: disk_134500(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x01: disk_134501(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x02: disk_134502(regs, drive_fl); ਰੋਕੋ;
    ਮੂਲ:   disk_1345XX(regs, drive_fl); ਰੋਕੋ;
    }
}

// IBM/MS eject media
ਸਥਿਰ ਖਾਲੀ noinline
disk_1346(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਜੇ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        ਵਾਪਸ;
    }

    ਪੂਰਨ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ਨੀਵਾਂ(CDRom_locks[cdid]);
    ਜੇ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ਵਾਪਸ;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    ਜੇ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ਵਾਪਸ;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ਸਥਿਰ ਖਾਲੀ
disk_1347(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ਸਥਿਰ ਖਾਲੀ
disk_1348(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਪੂਰਨ ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ਸਥਿਰ ਖਾਲੀ
disk_1349(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਜੇ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ਵਾਪਸ;
    }
    ਸੈੱਟ_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ਸਥਿਰ ਖਾਲੀ
disk_134e01(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ਸਥਿਰ ਖਾਲੀ
disk_134e03(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ਸਥਿਰ ਖਾਲੀ
disk_134e04(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ਸਥਿਰ ਖਾਲੀ
disk_134e06(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ਸਥਿਰ ਖਾਲੀ
disk_134eXX(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ਸਥਿਰ ਖਾਲੀ
disk_134e(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x01: disk_134e01(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x03: disk_134e03(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x04: disk_134e04(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x06: disk_134e06(regs, drive_fl); ਰੋਕੋ;
    ਮੂਲ:   disk_134eXX(regs, drive_fl); ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
disk_13XX(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ਸਥਿਰ ਖਾਲੀ
disk_13(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    ਸੈੱਟ_BDA(disk_ਰੁਕਾਵਟ_flag, 0);

    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00: disk_1300(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x01: disk_1301(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x02: disk_1302(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x03: disk_1303(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x04: disk_1304(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x05: disk_1305(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x08: disk_1308(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x09: disk_1309(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x0c: disk_130c(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x0d: disk_130d(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x10: disk_1310(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x11: disk_1311(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x14: disk_1314(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x15: disk_1315(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x16: disk_1316(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x41: disk_1341(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x42: disk_1342(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x43: disk_1343(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x44: disk_1344(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x45: disk_1345(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x46: disk_1346(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x47: disk_1347(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x48: disk_1348(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x49: disk_1349(regs, drive_fl); ਰੋਕੋ;
    ਹਾਲਤ 0x4e: disk_134e(regs, drive_fl); ਰੋਕੋ;
    ਮੂਲ:   disk_13XX(regs, drive_fl); ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
floppy_13(ਬਣਤਰ bregs *regs, ਬਣਤਰ drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00:
    ਹਾਲਤ 0x01:
    ਹਾਲਤ 0x02:
    ਹਾਲਤ 0x03:
    ਹਾਲਤ 0x04:
    ਹਾਲਤ 0x05:
    ਹਾਲਤ 0x08:
    ਹਾਲਤ 0x15:
    ਹਾਲਤ 0x16:
        disk_13(regs, drive_fl);
        ਰੋਕੋ;
    ਮੂਲ:   disk_13XX(regs, drive_fl); ਰੋਕੋ;
    }
}

// ElTorito - Terminate disk emu
ਸਥਿਰ ਖਾਲੀ
cdemu_134b(ਬਣਤਰ bregs *regs)
{
    memcpy_far(regs->ds, (ਖਾਲੀ*)(regs->si+0), SEG_ਨੀਵਾਂ, &CDEmu, ਆਕਾਰ(CDEmu));

    // If we have to terminate emulation
    ਜੇ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        ਸੈੱਟ_ਨੀਵਾਂ(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
handle_legacy_disk(ਬਣਤਰ bregs *regs, u8 extdrive)
{
    ਜੇ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        ਵਾਪਸ;
    }

    ਜੇ (extdrive < EXTSTART_HD) {
        ਬਣਤਰ drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        ਜੇ (!drive_fl)
            ਜਾਓ fail;
        floppy_13(regs, drive_fl);
        ਵਾਪਸ;
    }

    ਬਣਤਰ drive_s *drive_fl;
    ਜੇ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    ਨਹੀਂਤਾਂ
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    ਜੇ (!drive_fl)
        ਜਾਓ fail;
    disk_13(regs, drive_fl);
    ਵਾਪਸ;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

ਖਾਲੀ VISIBLE16
handle_40(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
ਖਾਲੀ VISIBLE16
handle_13(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    ਜੇ (CONFIG_CDROM_EMU) {
        ਜੇ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            ਵਾਪਸ;
        }
        ਜੇ (GET_ਨੀਵਾਂ(CDEmu.media)) {
            u8 emudrive = GET_ਨੀਵਾਂ(CDEmu.emulated_drive);
            ਜੇ (extdrive == emudrive) {
                // Access to an emulated drive.
                ਬਣਤਰ drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                ਜੇ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    ਵਾਪਸ;
                }
                disk_13(regs, cdemu_gf);
                ਵਾਪਸ;
            }
            ਜੇ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
ਖਾਲੀ VISIBLE16
handle_76(ਖਾਲੀ)
{
    debug_isr(DEBUG_ISR_76);
    ਸੈੱਟ_BDA(disk_ਰੁਕਾਵਟ_flag, 0xff);
    pic_eoi2();
}
