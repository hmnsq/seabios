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
__disk_ret(ರಚನೆ bregs *regs, u32 linecode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    u8 ಸಂಕೇತ_2 = linecode;
    ಆದರೆ (regs->dl < EXTSTART_HD)
        ಗಣ_BDA(floppy_ಕೊನೆಯ_ಸ್ಥಿತಿ, ಸಂಕೇತ_2);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_BDA(disk_ಕೊನೆಯ_ಸ್ಥಿತಿ, ಸಂಕೇತ_2);
    ಆದರೆ (ಸಂಕೇತ_2)
        __set_code_invalid(regs, linecode, fname);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
__disk_ret_unimplemented(ರಚನೆ bregs *regs, u32 linecode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    u8 ಸಂಕೇತ_2 = linecode;
    ಆದರೆ (regs->dl < EXTSTART_HD)
        ಗಣ_BDA(floppy_ಕೊನೆಯ_ಸ್ಥಿತಿ, ಸಂಕೇತ_2);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_BDA(disk_ಕೊನೆಯ_ಸ್ಥಿತಿ, ಸಂಕೇತ_2);
    __set_code_unimplemented(regs, linecode, fname);
}

ಸ್ಥಿರ ಶೂನ್ಯ
__disk_stub(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#ವ್ಯಾಖ್ಯಾನಿಸು disk_ret(regs, ಸಂಕೇತ_2) \
    __disk_ret((regs), (ಸಂಕೇತ_2) | (__LINE__ << 8), __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು disk_ret_unimplemented(regs, ಸಂಕೇತ_2) \
    __disk_ret_unimplemented((regs), (ಸಂಕೇತ_2) | (__LINE__ << 8), __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
ಸ್ಥಿರ ರಚನೆ chs_s
getLCHS(ರಚನೆ drive_s *drive_fl)
{
    ರಚನೆ chs_s res = { };
    ಆದರೆ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_ಕೆಳಗಿನ(CDEmu.chs.sptcyl);
        res.cylinder = GET_ಕೆಳಗಿನ(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.ತಲೆ = GET_ಕೆಳಗಿನ(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        ಹಿಂತಿರುಗಿ res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.ತಲೆ = GET_FLATPTR(drive_fl->lchs.ತಲೆ);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    ಹಿಂತಿರುಗಿ res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
__send_disk_op(ರಚನೆ disk_op_s *op_far, u16 op_seg)
{
    ರಚನೆ disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, ಗಾತ್ರ(dop));

    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = process_op(&dop);

    // Update count with total sectors transferred.
    ಗಣ_FARVAR(op_seg, op_far->ಎಣಿಕೆ_2, dop.ಎಣಿಕೆ_2);

    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
send_disk_op(ರಚನೆ disk_op_s *op)
{
    ASSERT16();
    ಆದರೆ (! CONFIG_DRIVES)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!CONFIG_ನಮೂದು_EXTRASTACK)
        // Jump on to extra stack
        ಹಿಂತಿರುಗಿ ರಾಶಿ_hop(__send_disk_op, op, GET_SEG(SS));
    ಹಿಂತಿರುಗಿ process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ಸ್ಥಿರ ಶೂನ್ಯ noinline
basic_access(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl, u16 command)
{
    ರಚನೆ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 ಎಣಿಕೆ_2 = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 ತಲೆ = regs->dh;

    ಆದರೆ (ಎಣಿಕೆ_2 > 128 || ಎಣಿಕೆ_2 == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }
    dop.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;

    ರಚನೆ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ತಲೆ, nls=chs.sector;

    // sanity check on cyl heads, sec
    ಆದರೆ (cylinder >= nlc || ತಲೆ >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)ತಲೆ) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = send_disk_op(&dop);

    regs->al = dop.ಎಣಿಕೆ_2;

    disk_ret(regs, ಸ್ಥಿತಿ_2);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ಸ್ಥಿರ ಶೂನ್ಯ noinline
extended_access(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl, u16 command)
{
    ರಚನೆ disk_op_s dop;
    ರಚನೆ int13ext_s *param_far = (ರಚನೆ int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    ಆದರೆ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }

    dop.buf_fl = SEGOFF_ಗಮ್ಯ_FLATPTR(GET_FARVAR(regs->ds, param_far->ದತ್ತಾಂಶ));
    dop.ಎಣಿಕೆ_2 = GET_FARVAR(regs->ds, param_far->ಎಣಿಕೆ_2);
    ಆದರೆ (! dop.ಎಣಿಕೆ_2) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = send_disk_op(&dop);

    ಗಣ_FARVAR(regs->ds, param_far->ಎಣಿಕೆ_2, dop.ಎಣಿಕೆ_2);

    disk_ret(regs, ಸ್ಥಿತಿ_2);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1300(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ರಚನೆ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.ಎಣಿಕೆ_2 = 0;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = send_disk_op(&dop);
    disk_ret(regs, ಸ್ಥಿತಿ_2);
}

// read disk status
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1301(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    u8 v;
    ಆದರೆ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_ಕೊನೆಯ_ಸ್ಥಿತಿ);
    ಇಲ್ಲದಿದ್ದರೆ
        v = GET_BDA(disk_ಕೊನೆಯ_ಸ್ಥಿತಿ);
    regs->ah = v;
    ಗಣ_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1302(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1303(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1304(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1305(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    debug_stub(regs);

    ರಚನೆ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ತಲೆ, nls=chs.sector;

    u8 ಎಣಿಕೆ_2 = regs->al;
    u8 cylinder = regs->ch;
    u8 ತಲೆ = regs->dh;

    ಆದರೆ (cylinder >= nlc || ತಲೆ >= nlh || ಎಣಿಕೆ_2 == 0 || ಎಣಿಕೆ_2 > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }

    ರಚನೆ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)ತಲೆ) * (u32)nls;
    dop.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = send_disk_op(&dop);
    disk_ret(regs, ಸ್ಥಿತಿ_2);
}

// read disk drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1308(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    // Get logical geometry from table
    ರಚನೆ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ತಲೆ, nls=chs.sector;
    nlc--;
    nlh--;
    u8 ಎಣಿಕೆ_2;
    ಆದರೆ (regs->dl < EXTSTART_HD) {
        // Floppy
        ಎಣಿಕೆ_2 = GET_GLOBAL(Floppy_ಎಣಿಕೆ);

        ಆದರೆ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_ಕೆಳಗಿನ(CDEmu.media) * 2;
        ಇಲ್ಲದಿದ್ದರೆ
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (regs->dl < EXTSTART_CD) {
        // Hard drive
        ಎಣಿಕೆ_2 = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (CONFIG_CDROM_EMU && GET_ಕೆಳಗಿನ(CDEmu.media)) {
        u8 emudrive = GET_ಕೆಳಗಿನ(CDEmu.emulated_drive);
        ಆದರೆ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            ಎಣಿಕೆ_2++;
        ಆದರೆ (regs->dl < EXTSTART_HD && ಎಣಿಕೆ_2 > 2)
            // Max of two floppy drives.
            ಎಣಿಕೆ_2 = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = ಎಣಿಕೆ_2;
}

// initialize drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1309(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ಸ್ಥಿರ ಶೂನ್ಯ
disk_130c(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ಸ್ಥಿರ ಶೂನ್ಯ
disk_130d(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1310(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    // should look at 40:8E also???

    ರಚನೆ disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.ಎಣಿಕೆ_2 = 0;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = send_disk_op(&dop);
    disk_ret(regs, ಸ್ಥಿತಿ_2);
}

// recalibrate
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1311(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1314(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1315(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    ಆದರೆ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        ಹಿಂತಿರುಗಿ;
    }
    // Hard drive

    // Get logical geometry from table
    ರಚನೆ chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.ತಲೆ, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_1316(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಆದರೆ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1341(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1342(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1343(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1344(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

//ಬೀಗ
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134500(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಂಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಆದರೆ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        ಹಿಂತಿರುಗಿ;
    }
    ಗಣ_ಕೆಳಗಿನ(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134501(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಂಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಆದರೆ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        ಹಿಂತಿರುಗಿ;
    }
    locks--;
    ಗಣ_ಕೆಳಗಿನ(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

//ಸ್ಥಿತಿ
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134502(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಂಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_1345XX(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1345(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಆದರೆ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x00: disk_134500(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: disk_134501(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: disk_134502(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   disk_1345XX(regs, drive_fl); ನಿಲ್ಲಿಸು;
    }
}

// IBM/MS eject media
ಸ್ಥಿರ ಶೂನ್ಯ noinline
disk_1346(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಆದರೆ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ಕೆಳಗಿನ(CDRom_locks[cdid]);
    ಆದರೆ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ಹಿಂತಿರುಗಿ;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    ಆದರೆ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ಹಿಂತಿರುಗಿ;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1347(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1348(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಪೂರ್ಣಾಂಕ ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ಸ್ಥಿರ ಶೂನ್ಯ
disk_1349(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಆದರೆ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ಹಿಂತಿರುಗಿ;
    }
    ಗಣ_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e01(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e03(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e04(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e06(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_134eXX(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ಸ್ಥಿರ ಶೂನ್ಯ
disk_134e(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x01: disk_134e01(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: disk_134e03(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x04: disk_134e04(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: disk_134e06(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   disk_134eXX(regs, drive_fl); ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_13XX(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ಸ್ಥಿರ ಶೂನ್ಯ
disk_13(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    ಗಣ_BDA(disk_ಅಡಚಣೆ_flag, 0);

    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00: disk_1300(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: disk_1301(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: disk_1302(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: disk_1303(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x04: disk_1304(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x05: disk_1305(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x08: disk_1308(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x09: disk_1309(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0c: disk_130c(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0d: disk_130d(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x10: disk_1310(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x11: disk_1311(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x14: disk_1314(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x15: disk_1315(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x16: disk_1316(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x41: disk_1341(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x42: disk_1342(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x43: disk_1343(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x44: disk_1344(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x45: disk_1345(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x46: disk_1346(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x47: disk_1347(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x48: disk_1348(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x49: disk_1349(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x4e: disk_134e(regs, drive_fl); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   disk_13XX(regs, drive_fl); ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
floppy_13(ರಚನೆ bregs *regs, ರಚನೆ drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00:
    ಸ್ಥಿತಿ 0x01:
    ಸ್ಥಿತಿ 0x02:
    ಸ್ಥಿತಿ 0x03:
    ಸ್ಥಿತಿ 0x04:
    ಸ್ಥಿತಿ 0x05:
    ಸ್ಥಿತಿ 0x08:
    ಸ್ಥಿತಿ 0x15:
    ಸ್ಥಿತಿ 0x16:
        disk_13(regs, drive_fl);
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   disk_13XX(regs, drive_fl); ನಿಲ್ಲಿಸು;
    }
}

// ElTorito - Terminate disk emu
ಸ್ಥಿರ ಶೂನ್ಯ
cdemu_134b(ರಚನೆ bregs *regs)
{
    memcpy_far(regs->ds, (ಶೂನ್ಯ*)(regs->si+0), SEG_ಕೆಳಗಿನ, &CDEmu, ಗಾತ್ರ(CDEmu));

    // If we have to terminate emulation
    ಆದರೆ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        ಗಣ_ಕೆಳಗಿನ(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
handle_legacy_disk(ರಚನೆ bregs *regs, u8 extdrive)
{
    ಆದರೆ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (extdrive < EXTSTART_HD) {
        ರಚನೆ drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        ಆದರೆ (!drive_fl)
            ಹೋಗು fail;
        floppy_13(regs, drive_fl);
        ಹಿಂತಿರುಗಿ;
    }

    ರಚನೆ drive_s *drive_fl;
    ಆದರೆ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    ಇಲ್ಲದಿದ್ದರೆ
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    ಆದರೆ (!drive_fl)
        ಹೋಗು fail;
    disk_13(regs, drive_fl);
    ಹಿಂತಿರುಗಿ;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

ಶೂನ್ಯ VISIBLE16
handle_40(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
ಶೂನ್ಯ VISIBLE16
handle_13(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    ಆದರೆ (CONFIG_CDROM_EMU) {
        ಆದರೆ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            ಹಿಂತಿರುಗಿ;
        }
        ಆದರೆ (GET_ಕೆಳಗಿನ(CDEmu.media)) {
            u8 emudrive = GET_ಕೆಳಗಿನ(CDEmu.emulated_drive);
            ಆದರೆ (extdrive == emudrive) {
                // Access to an emulated drive.
                ರಚನೆ drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                ಆದರೆ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    ಹಿಂತಿರುಗಿ;
                }
                disk_13(regs, cdemu_gf);
                ಹಿಂತಿರುಗಿ;
            }
            ಆದರೆ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
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
    ಗಣ_BDA(disk_ಅಡಚಣೆ_flag, 0xff);
    pic_eoi2();
}
