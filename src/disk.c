// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "hw/ata.h" // ATA_CB_DC
#شامل "hw/pic.h" // pic_eoi2
#شامل "output.h" // debug_enter
#شامل "stacks.h" // call16_int
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

ایستا تهی
__disk_ret(ساختار bregs *regs, u32 linecode, ثابت نویسه *fname)
{
    u8 code = linecode;
    اگر (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    وگرنه
        SET_BDA(disk_last_status, code);
    اگر (code)
        __set_code_invalid(regs, linecode, fname);
    وگرنه
        set_code_success(regs);
}

ایستا تهی
__disk_ret_unimplemented(ساختار bregs *regs, u32 linecode, ثابت نویسه *fname)
{
    u8 code = linecode;
    اگر (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    وگرنه
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

ایستا تهی
__disk_stub(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
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
ایستا ساختار chs_s
getLCHS(ساختار drive_s *drive_fl)
{
    ساختار chs_s res = { };
    اگر (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_کم(CDEmu.chs.sptcyl);
        res.cylinder = GET_کم(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_کم(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        بازگشت res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    بازگشت res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ایستا صحیح
__send_disk_op(ساختار disk_op_s *op_far, u16 op_seg)
{
    ساختار disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, اندازه(dop));

    صحیح status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->شمار, dop.شمار);

    بازگشت status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ایستا صحیح
send_disk_op(ساختار disk_op_s *op)
{
    ASSERT16();
    اگر (! CONFIG_DRIVES)
        بازگشت -1;
    اگر (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        بازگشت پشته_hop(__send_disk_op, op, GET_SEG(SS));
    بازگشت process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ایستا تهی noinline
basic_access(ساختار bregs *regs, ساختار drive_s *drive_fl, u16 command)
{
    ساختار disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 شمار = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    اگر (شمار > 128 || شمار == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }
    dop.شمار = شمار;

    ساختار chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    اگر (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    صحیح status = send_disk_op(&dop);

    regs->al = dop.شمار;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ایستا تهی noinline
extended_access(ساختار bregs *regs, ساختار drive_s *drive_fl, u16 command)
{
    ساختار disk_op_s dop;
    ساختار int13ext_s *param_far = (ساختار int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    اگر (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.شمار = GET_FARVAR(regs->ds, param_far->شمار);
    اگر (! dop.شمار) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        بازگشت;
    }

    صحیح status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->شمار, dop.شمار);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ایستا تهی
disk_1300(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    ساختار disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.شمار = 0;
    صحیح status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
ایستا تهی
disk_1301(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    u8 v;
    اگر (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    وگرنه
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ایستا تهی
disk_1302(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ایستا تهی
disk_1303(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ایستا تهی
disk_1304(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ایستا تهی noinline
disk_1305(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    debug_stub(regs);

    ساختار chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 شمار = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    اگر (cylinder >= nlc || head >= nlh || شمار == 0 || شمار > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }

    ساختار disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.شمار = شمار;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    صحیح status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
ایستا تهی noinline
disk_1308(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    // Get logical geometry from table
    ساختار chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 شمار;
    اگر (regs->dl < EXTSTART_HD) {
        // Floppy
        شمار = GET_GLOBAL(Floppy_شمار);

        اگر (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_کم(CDEmu.media) * 2;
        وگرنه
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } وگرنه اگر (regs->dl < EXTSTART_CD) {
        // Hard drive
        شمار = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } وگرنه {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }

    اگر (CONFIG_CDROM_EMU && GET_کم(CDEmu.media)) {
        u8 emudrive = GET_کم(CDEmu.emulated_drive);
        اگر (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            شمار++;
        اگر (regs->dl < EXTSTART_HD && شمار > 2)
            // Max of two floppy drives.
            شمار = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = شمار;
}

// initialize drive parameters
ایستا تهی
disk_1309(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ایستا تهی
disk_130c(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ایستا تهی
disk_130d(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ایستا تهی
disk_1310(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    // should look at 40:8E also???

    ساختار disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.شمار = 0;
    صحیح status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
ایستا تهی
disk_1311(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ایستا تهی
disk_1314(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ایستا تهی noinline
disk_1315(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    اگر (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        بازگشت;
    }
    // Hard drive

    // Get logical geometry from table
    ساختار chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ایستا تهی
disk_1316(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    اگر (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ایستا تهی
disk_1341(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ایستا تهی
disk_1342(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ایستا تهی
disk_1343(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ایستا تهی
disk_1344(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
ایستا تهی
disk_134500(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        بازگشت;
    }
    SET_کم(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ایستا تهی
disk_134501(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        بازگشت;
    }
    locks--;
    SET_کم(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
ایستا تهی
disk_134502(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ایستا تهی
disk_1345XX(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ایستا تهی
disk_1345(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        بازگشت;
    }

    انتخاب (regs->al) {
    حالت 0x00: disk_134500(regs, drive_fl); بشکن;
    حالت 0x01: disk_134501(regs, drive_fl); بشکن;
    حالت 0x02: disk_134502(regs, drive_fl); بشکن;
    پیشفرض:   disk_1345XX(regs, drive_fl); بشکن;
    }
}

// IBM/MS eject media
ایستا تهی noinline
disk_1346(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        بازگشت;
    }

    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        بازگشت;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    اگر (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        بازگشت;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ایستا تهی
disk_1347(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ایستا تهی
disk_1348(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    صحیح ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ایستا تهی
disk_1349(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        بازگشت;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ایستا تهی
disk_134e01(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ایستا تهی
disk_134e03(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ایستا تهی
disk_134e04(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ایستا تهی
disk_134e06(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ایستا تهی
disk_134eXX(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ایستا تهی
disk_134e(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    انتخاب (regs->al) {
    حالت 0x01: disk_134e01(regs, drive_fl); بشکن;
    حالت 0x03: disk_134e03(regs, drive_fl); بشکن;
    حالت 0x04: disk_134e04(regs, drive_fl); بشکن;
    حالت 0x06: disk_134e06(regs, drive_fl); بشکن;
    پیشفرض:   disk_134eXX(regs, drive_fl); بشکن;
    }
}

ایستا تهی
disk_13XX(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ایستا تهی
disk_13(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    انتخاب (regs->ah) {
    حالت 0x00: disk_1300(regs, drive_fl); بشکن;
    حالت 0x01: disk_1301(regs, drive_fl); بشکن;
    حالت 0x02: disk_1302(regs, drive_fl); بشکن;
    حالت 0x03: disk_1303(regs, drive_fl); بشکن;
    حالت 0x04: disk_1304(regs, drive_fl); بشکن;
    حالت 0x05: disk_1305(regs, drive_fl); بشکن;
    حالت 0x08: disk_1308(regs, drive_fl); بشکن;
    حالت 0x09: disk_1309(regs, drive_fl); بشکن;
    حالت 0x0c: disk_130c(regs, drive_fl); بشکن;
    حالت 0x0d: disk_130d(regs, drive_fl); بشکن;
    حالت 0x10: disk_1310(regs, drive_fl); بشکن;
    حالت 0x11: disk_1311(regs, drive_fl); بشکن;
    حالت 0x14: disk_1314(regs, drive_fl); بشکن;
    حالت 0x15: disk_1315(regs, drive_fl); بشکن;
    حالت 0x16: disk_1316(regs, drive_fl); بشکن;
    حالت 0x41: disk_1341(regs, drive_fl); بشکن;
    حالت 0x42: disk_1342(regs, drive_fl); بشکن;
    حالت 0x43: disk_1343(regs, drive_fl); بشکن;
    حالت 0x44: disk_1344(regs, drive_fl); بشکن;
    حالت 0x45: disk_1345(regs, drive_fl); بشکن;
    حالت 0x46: disk_1346(regs, drive_fl); بشکن;
    حالت 0x47: disk_1347(regs, drive_fl); بشکن;
    حالت 0x48: disk_1348(regs, drive_fl); بشکن;
    حالت 0x49: disk_1349(regs, drive_fl); بشکن;
    حالت 0x4e: disk_134e(regs, drive_fl); بشکن;
    پیشفرض:   disk_13XX(regs, drive_fl); بشکن;
    }
}

ایستا تهی
floppy_13(ساختار bregs *regs, ساختار drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    انتخاب (regs->ah) {
    حالت 0x00:
    حالت 0x01:
    حالت 0x02:
    حالت 0x03:
    حالت 0x04:
    حالت 0x05:
    حالت 0x08:
    حالت 0x15:
    حالت 0x16:
        disk_13(regs, drive_fl);
        بشکن;
    پیشفرض:   disk_13XX(regs, drive_fl); بشکن;
    }
}

// ElTorito - Terminate disk emu
ایستا تهی
cdemu_134b(ساختار bregs *regs)
{
    memcpy_far(regs->ds, (تهی*)(regs->si+0), SEG_کم, &CDEmu, اندازه(CDEmu));

    // If we have to terminate emulation
    اگر (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_کم(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ایستا تهی
handle_legacy_disk(ساختار bregs *regs, u8 extdrive)
{
    اگر (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        بازگشت;
    }

    اگر (extdrive < EXTSTART_HD) {
        ساختار drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        اگر (!drive_fl)
            برو fail;
        floppy_13(regs, drive_fl);
        بازگشت;
    }

    ساختار drive_s *drive_fl;
    اگر (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    وگرنه
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    اگر (!drive_fl)
        برو fail;
    disk_13(regs, drive_fl);
    بازگشت;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

تهی VISIBLE16
handle_40(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
تهی VISIBLE16
handle_13(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    اگر (CONFIG_CDROM_EMU) {
        اگر (regs->ah == 0x4b) {
            cdemu_134b(regs);
            بازگشت;
        }
        اگر (GET_کم(CDEmu.media)) {
            u8 emudrive = GET_کم(CDEmu.emulated_drive);
            اگر (extdrive == emudrive) {
                // Access to an emulated drive.
                ساختار drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                اگر (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    بازگشت;
                }
                disk_13(regs, cdemu_gf);
                بازگشت;
            }
            اگر (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
تهی VISIBLE16
handle_76(تهی)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
