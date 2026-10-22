// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // SET_BDA
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/ata.h" // ATA_CB_DC
#অন্তর্ভাব "hw/pic.h" // pic_eoi2
#অন্তর্ভাব "output.h" // debug_enter
#অন্তর্ভাব "stacks.h" // call16_int
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

স্থির শূন্য
__disk_ret(সংরচনা bregs *regs, u32 linecode, ধ্রুব বর্ণ *fname)
{
    u8 code = linecode;
    যদি (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    অন্যথা
        SET_BDA(disk_last_status, code);
    যদি (code)
        __set_code_invalid(regs, linecode, fname);
    অন্যথা
        set_code_success(regs);
}

স্থির শূন্য
__disk_ret_unimplemented(সংরচনা bregs *regs, u32 linecode, ধ্রুব বর্ণ *fname)
{
    u8 code = linecode;
    যদি (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    অন্যথা
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

স্থির শূন্য
__disk_stub(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname)
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
স্থির সংরচনা chs_s
getLCHS(সংরচনা drive_s *drive_fl)
{
    সংরচনা chs_s res = { };
    যদি (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_নিচের(CDEmu.chs.sptcyl);
        res.cylinder = GET_নিচের(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_নিচের(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        প্রত্যাগম res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    প্রত্যাগম res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
স্থির পূর্ণাঙ্ক
__send_disk_op(সংরচনা disk_op_s *op_far, u16 op_seg)
{
    সংরচনা disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, পরিমাণ(dop));

    পূর্ণাঙ্ক status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->গুনতি, dop.গুনতি);

    প্রত্যাগম status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
স্থির পূর্ণাঙ্ক
send_disk_op(সংরচনা disk_op_s *op)
{
    ASSERT16();
    যদি (! CONFIG_DRIVES)
        প্রত্যাগম -1;
    যদি (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        প্রত্যাগম স্তূপ_hop(__send_disk_op, op, GET_SEG(SS));
    প্রত্যাগম process_op(op);
}

// Perform read/write/verify using old-style chs accesses
স্থির শূন্য noinline
basic_access(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl, u16 command)
{
    সংরচনা disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 গুনতি = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    যদি (গুনতি > 128 || গুনতি == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }
    dop.গুনতি = গুনতি;

    সংরচনা chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    যদি (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    পূর্ণাঙ্ক status = send_disk_op(&dop);

    regs->al = dop.গুনতি;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
স্থির শূন্য noinline
extended_access(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl, u16 command)
{
    সংরচনা disk_op_s dop;
    সংরচনা int13ext_s *param_far = (সংরচনা int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    যদি (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.গুনতি = GET_FARVAR(regs->ds, param_far->গুনতি);
    যদি (! dop.গুনতি) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        প্রত্যাগম;
    }

    পূর্ণাঙ্ক status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->গুনতি, dop.গুনতি);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
স্থির শূন্য
disk_1300(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    সংরচনা disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.গুনতি = 0;
    পূর্ণাঙ্ক status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
স্থির শূন্য
disk_1301(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    u8 v;
    যদি (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    অন্যথা
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
স্থির শূন্য
disk_1302(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
স্থির শূন্য
disk_1303(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
স্থির শূন্য
disk_1304(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
স্থির শূন্য noinline
disk_1305(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    debug_stub(regs);

    সংরচনা chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 গুনতি = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    যদি (cylinder >= nlc || head >= nlh || গুনতি == 0 || গুনতি > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }

    সংরচনা disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.গুনতি = গুনতি;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    পূর্ণাঙ্ক status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
স্থির শূন্য noinline
disk_1308(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    // Get logical geometry from table
    সংরচনা chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 গুনতি;
    যদি (regs->dl < EXTSTART_HD) {
        // Floppy
        গুনতি = GET_GLOBAL(Floppy_গুনতি);

        যদি (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_নিচের(CDEmu.media) * 2;
        অন্যথা
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } অন্যথা যদি (regs->dl < EXTSTART_CD) {
        // Hard drive
        গুনতি = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } অন্যথা {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }

    যদি (CONFIG_CDROM_EMU && GET_নিচের(CDEmu.media)) {
        u8 emudrive = GET_নিচের(CDEmu.emulated_drive);
        যদি (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            গুনতি++;
        যদি (regs->dl < EXTSTART_HD && গুনতি > 2)
            // Max of two floppy drives.
            গুনতি = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = গুনতি;
}

// initialize drive parameters
স্থির শূন্য
disk_1309(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
স্থির শূন্য
disk_130c(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
স্থির শূন্য
disk_130d(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
স্থির শূন্য
disk_1310(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    // should look at 40:8E also???

    সংরচনা disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.গুনতি = 0;
    পূর্ণাঙ্ক status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
স্থির শূন্য
disk_1311(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
স্থির শূন্য
disk_1314(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
স্থির শূন্য noinline
disk_1315(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    যদি (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        প্রত্যাগম;
    }
    // Hard drive

    // Get logical geometry from table
    সংরচনা chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

স্থির শূন্য
disk_1316(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    যদি (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
স্থির শূন্য
disk_1341(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
স্থির শূন্য
disk_1342(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
স্থির শূন্য
disk_1343(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
স্থির শূন্য
disk_1344(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
স্থির শূন্য
disk_134500(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    পূর্ণাঙ্ক cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_নিচের(CDRom_locks[cdid]);
    যদি (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        প্রত্যাগম;
    }
    SET_নিচের(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
স্থির শূন্য
disk_134501(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    পূর্ণাঙ্ক cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_নিচের(CDRom_locks[cdid]);
    যদি (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        প্রত্যাগম;
    }
    locks--;
    SET_নিচের(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
স্থির শূন্য
disk_134502(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    পূর্ণাঙ্ক cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_নিচের(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

স্থির শূন্য
disk_1345XX(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
স্থির শূন্য
disk_1345(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    যদি (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        প্রত্যাগম;
    }

    বিকল্প (regs->al) {
    পক্ষ 0x00: disk_134500(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x01: disk_134501(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x02: disk_134502(regs, drive_fl); ভঙ্গ;
    স্বভাব:   disk_1345XX(regs, drive_fl); ভঙ্গ;
    }
}

// IBM/MS eject media
স্থির শূন্য noinline
disk_1346(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    যদি (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        প্রত্যাগম;
    }

    পূর্ণাঙ্ক cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_নিচের(CDRom_locks[cdid]);
    যদি (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        প্রত্যাগম;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    যদি (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        প্রত্যাগম;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
স্থির শূন্য
disk_1347(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
স্থির শূন্য
disk_1348(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    পূর্ণাঙ্ক ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
স্থির শূন্য
disk_1349(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    যদি (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        প্রত্যাগম;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

স্থির শূন্য
disk_134e01(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

স্থির শূন্য
disk_134e03(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

স্থির শূন্য
disk_134e04(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

স্থির শূন্য
disk_134e06(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

স্থির শূন্য
disk_134eXX(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
স্থির শূন্য
disk_134e(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    বিকল্প (regs->al) {
    পক্ষ 0x01: disk_134e01(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x03: disk_134e03(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x04: disk_134e04(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x06: disk_134e06(regs, drive_fl); ভঙ্গ;
    স্বভাব:   disk_134eXX(regs, drive_fl); ভঙ্গ;
    }
}

স্থির শূন্য
disk_13XX(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

স্থির শূন্য
disk_13(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    বিকল্প (regs->ah) {
    পক্ষ 0x00: disk_1300(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x01: disk_1301(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x02: disk_1302(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x03: disk_1303(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x04: disk_1304(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x05: disk_1305(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x08: disk_1308(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x09: disk_1309(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x0c: disk_130c(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x0d: disk_130d(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x10: disk_1310(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x11: disk_1311(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x14: disk_1314(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x15: disk_1315(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x16: disk_1316(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x41: disk_1341(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x42: disk_1342(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x43: disk_1343(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x44: disk_1344(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x45: disk_1345(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x46: disk_1346(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x47: disk_1347(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x48: disk_1348(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x49: disk_1349(regs, drive_fl); ভঙ্গ;
    পক্ষ 0x4e: disk_134e(regs, drive_fl); ভঙ্গ;
    স্বভাব:   disk_13XX(regs, drive_fl); ভঙ্গ;
    }
}

স্থির শূন্য
floppy_13(সংরচনা bregs *regs, সংরচনা drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    বিকল্প (regs->ah) {
    পক্ষ 0x00:
    পক্ষ 0x01:
    পক্ষ 0x02:
    পক্ষ 0x03:
    পক্ষ 0x04:
    পক্ষ 0x05:
    পক্ষ 0x08:
    পক্ষ 0x15:
    পক্ষ 0x16:
        disk_13(regs, drive_fl);
        ভঙ্গ;
    স্বভাব:   disk_13XX(regs, drive_fl); ভঙ্গ;
    }
}

// ElTorito - Terminate disk emu
স্থির শূন্য
cdemu_134b(সংরচনা bregs *regs)
{
    memcpy_far(regs->ds, (শূন্য*)(regs->si+0), SEG_নিচের, &CDEmu, পরিমাণ(CDEmu));

    // If we have to terminate emulation
    যদি (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_নিচের(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

স্থির শূন্য
handle_legacy_disk(সংরচনা bregs *regs, u8 extdrive)
{
    যদি (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        প্রত্যাগম;
    }

    যদি (extdrive < EXTSTART_HD) {
        সংরচনা drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        যদি (!drive_fl)
            গচ্ছ fail;
        floppy_13(regs, drive_fl);
        প্রত্যাগম;
    }

    সংরচনা drive_s *drive_fl;
    যদি (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    অন্যথা
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    যদি (!drive_fl)
        গচ্ছ fail;
    disk_13(regs, drive_fl);
    প্রত্যাগম;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

শূন্য VISIBLE16
handle_40(সংরচনা bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
শূন্য VISIBLE16
handle_13(সংরচনা bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    যদি (CONFIG_CDROM_EMU) {
        যদি (regs->ah == 0x4b) {
            cdemu_134b(regs);
            প্রত্যাগম;
        }
        যদি (GET_নিচের(CDEmu.media)) {
            u8 emudrive = GET_নিচের(CDEmu.emulated_drive);
            যদি (extdrive == emudrive) {
                // Access to an emulated drive.
                সংরচনা drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                যদি (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    প্রত্যাগম;
                }
                disk_13(regs, cdemu_gf);
                প্রত্যাগম;
            }
            যদি (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
শূন্য VISIBLE16
handle_76(শূন্য)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
