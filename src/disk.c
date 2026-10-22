// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/ata.h" // ATA_CB_DC
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_eoi2
#ಅನ್ತರ್ಭಾವ "output.h" // debug_enter
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
__disk_ret(ಸಂರಚನಾ bregs *regs, u32 linecode, ಧ್ರುವ ವರ್ಣ *fname)
{
    u8 code = linecode;
    ಯದಿ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    ಅನ್ಯಥಾ
        SET_BDA(disk_last_status, code);
    ಯದಿ (code)
        __set_code_invalid(regs, linecode, fname);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
__disk_ret_unimplemented(ಸಂರಚನಾ bregs *regs, u32 linecode, ಧ್ರುವ ವರ್ಣ *fname)
{
    u8 code = linecode;
    ಯದಿ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    ಅನ್ಯಥಾ
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

ಸ್ಥಿರ ಶೂನ್ಯ
__disk_stub(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#define disk_ret(regs, code) \
    __disk_ret((regs), (code) | (__LINE__ << 8), __func__)
#define disk_ret_unimplemented(regs, code) \
    __disk_ret_unimplemented((regs), (code) | (__LINE__ << 8), __func__)
#define DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
ಸ್ಥಿರ ಸಂರಚನಾ chs_s
getLCHS(ಸಂರಚನಾ drive_s *drive_fl)
{
    ಸಂರಚನಾ chs_s res = { };
    ಯದಿ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_ಕೆಳಗಿನ(CDEmu.chs.sptcyl);
        res.cylinder = GET_ಕೆಳಗಿನ(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_ಕೆಳಗಿನ(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        ಪ್ರತ್ಯಾಗಮ res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    ಪ್ರತ್ಯಾಗಮ res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
__send_disk_op(ಸಂರಚನಾ disk_op_s *op_far, u16 op_seg)
{
    ಸಂರಚನಾ disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, ಪರಿಮಾಣ(dop));

    ಪೂರ್ಣಾಙ್ಕ status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->ಎಣಿಕೆ, dop.ಎಣಿಕೆ);

    ಪ್ರತ್ಯಾಗಮ status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
send_disk_op(ಸಂರಚನಾ disk_op_s *op)
{
    ASSERT16();
    ಯದಿ (! CONFIG_DRIVES)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        ಪ್ರತ್ಯಾಗಮ ರಾಶಿ_hop(__send_disk_op, op, GET_SEG(SS));
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ಸ್ಥಿರ ಶೂನ್ಯ noinline
basic_access(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl, u16 command)
{
    ಸಂರಚನಾ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 ಎಣಿಕೆ = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    ಯದಿ (ಎಣಿಕೆ > 128 || ಎಣಿಕೆ == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }
    dop.ಎಣಿಕೆ = ಎಣಿಕೆ;

    ಸಂರಚನಾ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    ಯದಿ (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    ಪೂರ್ಣಾಙ್ಕ status = send_disk_op(&dop);

    regs->al = dop.ಎಣಿಕೆ;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ಸ್ಥಿರ ಶೂನ್ಯ noinline
extended_access(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl, u16 command)
{
    ಸಂರಚನಾ disk_op_s dop;
    ಸಂರಚನಾ int13ext_s *param_far = (ಸಂರಚನಾ int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    ಯದಿ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.ಎಣಿಕೆ = GET_FARVAR(regs->ds, param_far->ಎಣಿಕೆ);
    ಯದಿ (! dop.ಎಣಿಕೆ) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->ಎಣಿಕೆ, dop.ಎಣಿಕೆ);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1300(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಸಂರಚನಾ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.ಎಣಿಕೆ = 0;
    ಪೂರ್ಣಾಙ್ಕ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1301(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    u8 v;
    ಯದಿ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    ಅನ್ಯಥಾ
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1302(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1303(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1304(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1305(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    debug_stub(regs);

    ಸಂರಚನಾ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 ಎಣಿಕೆ = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    ಯದಿ (cylinder >= nlc || head >= nlh || ಎಣಿಕೆ == 0 || ಎಣಿಕೆ > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಸಂರಚನಾ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.ಎಣಿಕೆ = ಎಣಿಕೆ;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    ಪೂರ್ಣಾಙ್ಕ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1308(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    // Get logical geometry from table
    ಸಂರಚನಾ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 ಎಣಿಕೆ;
    ಯದಿ (regs->dl < EXTSTART_HD) {
        // Floppy
        ಎಣಿಕೆ = GET_GLOBAL(Floppy_ಎಣಿಕೆ);

        ಯದಿ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_ಕೆಳಗಿನ(CDEmu.media) * 2;
        ಅನ್ಯಥಾ
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } ಅನ್ಯಥಾ ಯದಿ (regs->dl < EXTSTART_CD) {
        // Hard drive
        ಎಣಿಕೆ = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } ಅನ್ಯಥಾ {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (CONFIG_CDROM_EMU && GET_ಕೆಳಗಿನ(CDEmu.media)) {
        u8 emudrive = GET_ಕೆಳಗಿನ(CDEmu.emulated_drive);
        ಯದಿ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            ಎಣಿಕೆ++;
        ಯದಿ (regs->dl < EXTSTART_HD && ಎಣಿಕೆ > 2)
            // Max of two floppy drives.
            ಎಣಿಕೆ = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = ಎಣಿಕೆ;
}

// initialize drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1309(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ಸ್ಥಿರ ಶೂನ್ಯ
disk_130c(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ಸ್ಥಿರ ಶೂನ್ಯ
disk_130d(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1310(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    // should look at 40:8E also???

    ಸಂರಚನಾ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.ಎಣಿಕೆ = 0;
    ಪೂರ್ಣಾಙ್ಕ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1311(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1314(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1315(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    ಯದಿ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        ಪ್ರತ್ಯಾಗಮ;
    }
    // Hard drive

    // Get logical geometry from table
    ಸಂರಚನಾ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_1316(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಯದಿ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1341(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1342(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1343(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1344(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134500(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಙ್ಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಯದಿ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        ಪ್ರತ್ಯಾಗಮ;
    }
    SET_ಕೆಳಗಿನ(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134501(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಙ್ಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಯದಿ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        ಪ್ರತ್ಯಾಗಮ;
    }
    locks--;
    SET_ಕೆಳಗಿನ(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134502(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಙ್ಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_1345XX(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1345(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಯದಿ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x00: disk_134500(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: disk_134501(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: disk_134502(regs, drive_fl); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   disk_1345XX(regs, drive_fl); ಭಙ್ಗ;
    }
}

// IBM/MS eject media
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1346(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಯದಿ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಯದಿ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ಪ್ರತ್ಯಾಗಮ;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    ಯದಿ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ಪ್ರತ್ಯಾಗಮ;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1347(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1348(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಙ್ಕ ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1349(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ಯದಿ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ಪ್ರತ್ಯಾಗಮ;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e01(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e03(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e04(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e06(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134eXX(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x01: disk_134e01(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: disk_134e03(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x04: disk_134e04(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x06: disk_134e06(regs, drive_fl); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   disk_134eXX(regs, drive_fl); ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_13XX(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_13(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    ವಿಕಲ್ಪ (regs->ah) {
    ಪಕ್ಷ 0x00: disk_1300(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: disk_1301(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: disk_1302(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: disk_1303(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x04: disk_1304(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x05: disk_1305(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x08: disk_1308(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x09: disk_1309(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0c: disk_130c(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0d: disk_130d(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x10: disk_1310(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x11: disk_1311(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x14: disk_1314(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x15: disk_1315(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x16: disk_1316(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x41: disk_1341(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x42: disk_1342(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x43: disk_1343(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x44: disk_1344(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x45: disk_1345(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x46: disk_1346(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x47: disk_1347(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x48: disk_1348(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x49: disk_1349(regs, drive_fl); ಭಙ್ಗ;
    ಪಕ್ಷ 0x4e: disk_134e(regs, drive_fl); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   disk_13XX(regs, drive_fl); ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
floppy_13(ಸಂರಚನಾ bregs *regs, ಸಂರಚನಾ drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    ವಿಕಲ್ಪ (regs->ah) {
    ಪಕ್ಷ 0x00:
    ಪಕ್ಷ 0x01:
    ಪಕ್ಷ 0x02:
    ಪಕ್ಷ 0x03:
    ಪಕ್ಷ 0x04:
    ಪಕ್ಷ 0x05:
    ಪಕ್ಷ 0x08:
    ಪಕ್ಷ 0x15:
    ಪಕ್ಷ 0x16:
        disk_13(regs, drive_fl);
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:   disk_13XX(regs, drive_fl); ಭಙ್ಗ;
    }
}

// ElTorito - Terminate disk emu
ಸ್ಥಿರ ಶೂನ್ಯ
cdemu_134b(ಸಂರಚನಾ bregs *regs)
{
    memcpy_far(regs->ds, (ಶೂನ್ಯ*)(regs->si+0), SEG_ಕೆಳಗಿನ, &CDEmu, ಪರಿಮಾಣ(CDEmu));

    // If we have to terminate emulation
    ಯದಿ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_ಕೆಳಗಿನ(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
handle_legacy_disk(ಸಂರಚನಾ bregs *regs, u8 extdrive)
{
    ಯದಿ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (extdrive < EXTSTART_HD) {
        ಸಂರಚನಾ drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        ಯದಿ (!drive_fl)
            ಗಚ್ಛ fail;
        floppy_13(regs, drive_fl);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಸಂರಚನಾ drive_s *drive_fl;
    ಯದಿ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    ಅನ್ಯಥಾ
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    ಯದಿ (!drive_fl)
        ಗಚ್ಛ fail;
    disk_13(regs, drive_fl);
    ಪ್ರತ್ಯಾಗಮ;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

ಶೂನ್ಯ VISIBLE16
handle_40(ಸಂರಚನಾ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
ಶೂನ್ಯ VISIBLE16
handle_13(ಸಂರಚನಾ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    ಯದಿ (CONFIG_CDROM_EMU) {
        ಯದಿ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            ಪ್ರತ್ಯಾಗಮ;
        }
        ಯದಿ (GET_ಕೆಳಗಿನ(CDEmu.media)) {
            u8 emudrive = GET_ಕೆಳಗಿನ(CDEmu.emulated_drive);
            ಯದಿ (extdrive == emudrive) {
                // Access to an emulated drive.
                ಸಂರಚನಾ drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                ಯದಿ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    ಪ್ರತ್ಯಾಗಮ;
                }
                disk_13(regs, cdemu_gf);
                ಪ್ರತ್ಯಾಗಮ;
            }
            ಯದಿ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
ಶೂನ್ಯ VISIBLE16
handle_76(ಶೂನ್ಯ)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
