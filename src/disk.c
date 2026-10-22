// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_BDA
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "hw/ata.h" // ATA_CB_DC
#ضمن "hw/pic.h" // pic_eoi2
#ضمن "output.h" // debug_enter
#ضمن "stacks.h" // call16_int
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

ساكن فراغ
__disk_ret(بنية bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 کد = linecode;
    إذا (regs->dl < EXTSTART_HD)
        مجموعه_BDA(floppy_آخرین_وضعیت, کد);
    وإلا
        مجموعه_BDA(disk_آخرین_وضعیت, کد);
    إذا (کد)
        __set_code_invalid(regs, linecode, fname);
    وإلا
        مجموعه_کد_success(regs);
}

ساكن فراغ
__disk_ret_unimplemented(بنية bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 کد = linecode;
    إذا (regs->dl < EXTSTART_HD)
        مجموعه_BDA(floppy_آخرین_وضعیت, کد);
    وإلا
        مجموعه_BDA(disk_آخرین_وضعیت, کد);
    __set_code_unimplemented(regs, linecode, fname);
}

ساكن فراغ
__disk_stub(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#تعریف_کن disk_ret(regs, کد) \
    __disk_ret((regs), (کد) | (__LINE__ << 8), __func__)
#تعریف_کن disk_ret_unimplemented(regs, کد) \
    __disk_ret_unimplemented((regs), (کد) | (__LINE__ << 8), __func__)
#تعریف_کن DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
ساكن بنية chs_s
getLCHS(بنية drive_s *drive_fl)
{
    بنية chs_s res = { };
    إذا (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_کم(CDEmu.chs.sptcyl);
        res.cylinder = GET_کم(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.سر = GET_کم(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        أرجع res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.سر = GET_FLATPTR(drive_fl->lchs.سر);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    أرجع res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ساكن صحيح
__send_disk_op(بنية disk_op_s *op_far, u16 op_seg)
{
    بنية disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, حجم(dop));

    صحيح وضعیت = process_op(&dop);

    // Update count with total sectors transferred.
    مجموعه_FARVAR(op_seg, op_far->شمار, dop.شمار);

    أرجع وضعیت;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ساكن صحيح
send_disk_op(بنية disk_op_s *op)
{
    ASSERT16();
    إذا (! CONFIG_DRIVES)
        أرجع -1;
    إذا (!CONFIG_مدخل_EXTRASTACK)
        // Jump on to extra stack
        أرجع پشته_hop(__send_disk_op, op, GET_SEG(SS));
    أرجع process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ساكن فراغ noinline
basic_access(بنية bregs *regs, بنية drive_s *drive_fl, u16 command)
{
    بنية disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 شمار = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 سر = regs->dh;

    إذا (شمار > 128 || شمار == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }
    dop.شمار = شمار;

    بنية chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    // sanity check on cyl heads, sec
    إذا (cylinder >= nlc || سر >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)سر) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    صحيح وضعیت = send_disk_op(&dop);

    regs->al = dop.شمار;

    disk_ret(regs, وضعیت);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ساكن فراغ noinline
extended_access(بنية bregs *regs, بنية drive_s *drive_fl, u16 command)
{
    بنية disk_op_s dop;
    بنية int13ext_s *param_far = (بنية int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    إذا (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }

    dop.buf_fl = SEGOFF_مقصد_FLATPTR(GET_FARVAR(regs->ds, param_far->داده));
    dop.شمار = GET_FARVAR(regs->ds, param_far->شمار);
    إذا (! dop.شمار) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        أرجع;
    }

    صحيح وضعیت = send_disk_op(&dop);

    مجموعه_FARVAR(regs->ds, param_far->شمار, dop.شمار);

    disk_ret(regs, وضعیت);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ساكن فراغ
disk_1300(بنية bregs *regs, بنية drive_s *drive_fl)
{
    بنية disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.شمار = 0;
    صحيح وضعیت = send_disk_op(&dop);
    disk_ret(regs, وضعیت);
}

// read disk status
ساكن فراغ
disk_1301(بنية bregs *regs, بنية drive_s *drive_fl)
{
    u8 v;
    إذا (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_آخرین_وضعیت);
    وإلا
        v = GET_BDA(disk_آخرین_وضعیت);
    regs->ah = v;
    مجموعه_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ساكن فراغ
disk_1302(بنية bregs *regs, بنية drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ساكن فراغ
disk_1303(بنية bregs *regs, بنية drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ساكن فراغ
disk_1304(بنية bregs *regs, بنية drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ساكن فراغ noinline
disk_1305(بنية bregs *regs, بنية drive_s *drive_fl)
{
    debug_stub(regs);

    بنية chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    u8 شمار = regs->al;
    u8 cylinder = regs->ch;
    u8 سر = regs->dh;

    إذا (cylinder >= nlc || سر >= nlh || شمار == 0 || شمار > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }

    بنية disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)سر) * (u32)nls;
    dop.شمار = شمار;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    صحيح وضعیت = send_disk_op(&dop);
    disk_ret(regs, وضعیت);
}

// read disk drive parameters
ساكن فراغ noinline
disk_1308(بنية bregs *regs, بنية drive_s *drive_fl)
{
    // Get logical geometry from table
    بنية chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;
    nlc--;
    nlh--;
    u8 شمار;
    إذا (regs->dl < EXTSTART_HD) {
        // Floppy
        شمار = GET_GLOBAL(Floppy_شمار);

        إذا (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_کم(CDEmu.media) * 2;
        وإلا
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } وإلا إذا (regs->dl < EXTSTART_CD) {
        // Hard drive
        شمار = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } وإلا {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }

    إذا (CONFIG_CDROM_EMU && GET_کم(CDEmu.media)) {
        u8 emudrive = GET_کم(CDEmu.emulated_drive);
        إذا (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            شمار++;
        إذا (regs->dl < EXTSTART_HD && شمار > 2)
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
ساكن فراغ
disk_1309(بنية bregs *regs, بنية drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ساكن فراغ
disk_130c(بنية bregs *regs, بنية drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ساكن فراغ
disk_130d(بنية bregs *regs, بنية drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ساكن فراغ
disk_1310(بنية bregs *regs, بنية drive_s *drive_fl)
{
    // should look at 40:8E also???

    بنية disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.شمار = 0;
    صحيح وضعیت = send_disk_op(&dop);
    disk_ret(regs, وضعیت);
}

// recalibrate
ساكن فراغ
disk_1311(بنية bregs *regs, بنية drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ساكن فراغ
disk_1314(بنية bregs *regs, بنية drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ساكن فراغ noinline
disk_1315(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    إذا (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        أرجع;
    }
    // Hard drive

    // Get logical geometry from table
    بنية chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ساكن فراغ
disk_1316(بنية bregs *regs, بنية drive_s *drive_fl)
{
    إذا (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ساكن فراغ
disk_1341(بنية bregs *regs, بنية drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ساكن فراغ
disk_1342(بنية bregs *regs, بنية drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ساكن فراغ
disk_1343(بنية bregs *regs, بنية drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ساكن فراغ
disk_1344(بنية bregs *regs, بنية drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

//قفل
ساكن فراغ
disk_134500(بنية bregs *regs, بنية drive_s *drive_fl)
{
    صحيح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    إذا (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        أرجع;
    }
    مجموعه_کم(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ساكن فراغ
disk_134501(بنية bregs *regs, بنية drive_s *drive_fl)
{
    صحيح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    إذا (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        أرجع;
    }
    locks--;
    مجموعه_کم(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

//وضعیت
ساكن فراغ
disk_134502(بنية bregs *regs, بنية drive_s *drive_fl)
{
    صحيح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساكن فراغ
disk_1345XX(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ساكن فراغ
disk_1345(بنية bregs *regs, بنية drive_s *drive_fl)
{
    إذا (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        أرجع;
    }

    بدل (regs->al) {
    حالة 0x00: disk_134500(regs, drive_fl); اكسر;
    حالة 0x01: disk_134501(regs, drive_fl); اكسر;
    حالة 0x02: disk_134502(regs, drive_fl); اكسر;
    افتراضي:   disk_1345XX(regs, drive_fl); اكسر;
    }
}

// IBM/MS eject media
ساكن فراغ noinline
disk_1346(بنية bregs *regs, بنية drive_s *drive_fl)
{
    إذا (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        أرجع;
    }

    صحيح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    إذا (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        أرجع;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    إذا (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        أرجع;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ساكن فراغ
disk_1347(بنية bregs *regs, بنية drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ساكن فراغ
disk_1348(بنية bregs *regs, بنية drive_s *drive_fl)
{
    صحيح ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ساكن فراغ
disk_1349(بنية bregs *regs, بنية drive_s *drive_fl)
{
    إذا (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        أرجع;
    }
    مجموعه_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ساكن فراغ
disk_134e01(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساكن فراغ
disk_134e03(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساكن فراغ
disk_134e04(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساكن فراغ
disk_134e06(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساكن فراغ
disk_134eXX(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ساكن فراغ
disk_134e(بنية bregs *regs, بنية drive_s *drive_fl)
{
    بدل (regs->al) {
    حالة 0x01: disk_134e01(regs, drive_fl); اكسر;
    حالة 0x03: disk_134e03(regs, drive_fl); اكسر;
    حالة 0x04: disk_134e04(regs, drive_fl); اكسر;
    حالة 0x06: disk_134e06(regs, drive_fl); اكسر;
    افتراضي:   disk_134eXX(regs, drive_fl); اكسر;
    }
}

ساكن فراغ
disk_13XX(بنية bregs *regs, بنية drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ساكن فراغ
disk_13(بنية bregs *regs, بنية drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    مجموعه_BDA(disk_وقفه_flag, 0);

    بدل (regs->ah) {
    حالة 0x00: disk_1300(regs, drive_fl); اكسر;
    حالة 0x01: disk_1301(regs, drive_fl); اكسر;
    حالة 0x02: disk_1302(regs, drive_fl); اكسر;
    حالة 0x03: disk_1303(regs, drive_fl); اكسر;
    حالة 0x04: disk_1304(regs, drive_fl); اكسر;
    حالة 0x05: disk_1305(regs, drive_fl); اكسر;
    حالة 0x08: disk_1308(regs, drive_fl); اكسر;
    حالة 0x09: disk_1309(regs, drive_fl); اكسر;
    حالة 0x0c: disk_130c(regs, drive_fl); اكسر;
    حالة 0x0d: disk_130d(regs, drive_fl); اكسر;
    حالة 0x10: disk_1310(regs, drive_fl); اكسر;
    حالة 0x11: disk_1311(regs, drive_fl); اكسر;
    حالة 0x14: disk_1314(regs, drive_fl); اكسر;
    حالة 0x15: disk_1315(regs, drive_fl); اكسر;
    حالة 0x16: disk_1316(regs, drive_fl); اكسر;
    حالة 0x41: disk_1341(regs, drive_fl); اكسر;
    حالة 0x42: disk_1342(regs, drive_fl); اكسر;
    حالة 0x43: disk_1343(regs, drive_fl); اكسر;
    حالة 0x44: disk_1344(regs, drive_fl); اكسر;
    حالة 0x45: disk_1345(regs, drive_fl); اكسر;
    حالة 0x46: disk_1346(regs, drive_fl); اكسر;
    حالة 0x47: disk_1347(regs, drive_fl); اكسر;
    حالة 0x48: disk_1348(regs, drive_fl); اكسر;
    حالة 0x49: disk_1349(regs, drive_fl); اكسر;
    حالة 0x4e: disk_134e(regs, drive_fl); اكسر;
    افتراضي:   disk_13XX(regs, drive_fl); اكسر;
    }
}

ساكن فراغ
floppy_13(بنية bregs *regs, بنية drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    بدل (regs->ah) {
    حالة 0x00:
    حالة 0x01:
    حالة 0x02:
    حالة 0x03:
    حالة 0x04:
    حالة 0x05:
    حالة 0x08:
    حالة 0x15:
    حالة 0x16:
        disk_13(regs, drive_fl);
        اكسر;
    افتراضي:   disk_13XX(regs, drive_fl); اكسر;
    }
}

// ElTorito - Terminate disk emu
ساكن فراغ
cdemu_134b(بنية bregs *regs)
{
    memcpy_far(regs->ds, (فراغ*)(regs->si+0), SEG_کم, &CDEmu, حجم(CDEmu));

    // If we have to terminate emulation
    إذا (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        مجموعه_کم(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ساكن فراغ
handle_legacy_disk(بنية bregs *regs, u8 extdrive)
{
    إذا (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        أرجع;
    }

    إذا (extdrive < EXTSTART_HD) {
        بنية drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        إذا (!drive_fl)
            اذهب fail;
        floppy_13(regs, drive_fl);
        أرجع;
    }

    بنية drive_s *drive_fl;
    إذا (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    وإلا
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    إذا (!drive_fl)
        اذهب fail;
    disk_13(regs, drive_fl);
    أرجع;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

فراغ VISIBLE16
handle_40(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
فراغ VISIBLE16
handle_13(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    إذا (CONFIG_CDROM_EMU) {
        إذا (regs->ah == 0x4b) {
            cdemu_134b(regs);
            أرجع;
        }
        إذا (GET_کم(CDEmu.media)) {
            u8 emudrive = GET_کم(CDEmu.emulated_drive);
            إذا (extdrive == emudrive) {
                // Access to an emulated drive.
                بنية drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                إذا (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    أرجع;
                }
                disk_13(regs, cdemu_gf);
                أرجع;
            }
            إذا (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
فراغ VISIBLE16
handle_76(فراغ)
{
    debug_isr(DEBUG_ISR_76);
    مجموعه_BDA(disk_وقفه_flag, 0xff);
    pic_eoi2();
}
