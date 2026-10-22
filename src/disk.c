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

ساکن خالی
__disk_ret(ساخت bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 رمز = linecode;
    اگر (regs->dl < EXTSTART_HD)
        سیٹ_BDA(floppy_آخری_حالت, رمز);
    ورنہ
        سیٹ_BDA(disk_آخری_حالت, رمز);
    اگر (رمز)
        __set_code_invalid(regs, linecode, fname);
    ورنہ
        سیٹ_رمز_success(regs);
}

ساکن خالی
__disk_ret_unimplemented(ساخت bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 رمز = linecode;
    اگر (regs->dl < EXTSTART_HD)
        سیٹ_BDA(floppy_آخری_حالت, رمز);
    ورنہ
        سیٹ_BDA(disk_آخری_حالت, رمز);
    __set_code_unimplemented(regs, linecode, fname);
}

ساکن خالی
__disk_stub(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#تعریف_کرو disk_ret(regs, رمز) \
    __disk_ret((regs), (رمز) | (__LINE__ << 8), __func__)
#تعریف_کرو disk_ret_unimplemented(regs, رمز) \
    __disk_ret_unimplemented((regs), (رمز) | (__LINE__ << 8), __func__)
#تعریف_کرو DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
ساکن ساخت chs_s
getLCHS(ساخت drive_s *drive_fl)
{
    ساخت chs_s res = { };
    اگر (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_کم(CDEmu.chs.sptcyl);
        res.cylinder = GET_کم(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.سر = GET_کم(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        واپس res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.سر = GET_FLATPTR(drive_fl->lchs.سر);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    واپس res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ساکن صحیح
__send_disk_op(ساخت disk_op_s *op_far, u16 op_seg)
{
    ساخت disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, حجم(dop));

    صحیح حالت_2 = process_op(&dop);

    // Update count with total sectors transferred.
    سیٹ_FARVAR(op_seg, op_far->گنتی, dop.گنتی);

    واپس حالت_2;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ساکن صحیح
send_disk_op(ساخت disk_op_s *op)
{
    ASSERT16();
    اگر (! CONFIG_DRIVES)
        واپس -1;
    اگر (!CONFIG_اندراج_EXTRASTACK)
        // Jump on to extra stack
        واپس ڈھیر_hop(__send_disk_op, op, GET_SEG(SS));
    واپس process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ساکن خالی noinline
basic_access(ساخت bregs *regs, ساخت drive_s *drive_fl, u16 command)
{
    ساخت disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 گنتی = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 سر = regs->dh;

    اگر (گنتی > 128 || گنتی == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }
    dop.گنتی = گنتی;

    ساخت chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    // sanity check on cyl heads, sec
    اگر (cylinder >= nlc || سر >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)سر) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    صحیح حالت_2 = send_disk_op(&dop);

    regs->al = dop.گنتی;

    disk_ret(regs, حالت_2);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ساکن خالی noinline
extended_access(ساخت bregs *regs, ساخت drive_s *drive_fl, u16 command)
{
    ساخت disk_op_s dop;
    ساخت int13ext_s *param_far = (ساخت int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    اگر (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }

    dop.buf_fl = SEGOFF_مقصد_FLATPTR(GET_FARVAR(regs->ds, param_far->ڈیٹا));
    dop.گنتی = GET_FARVAR(regs->ds, param_far->گنتی);
    اگر (! dop.گنتی) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        واپس;
    }

    صحیح حالت_2 = send_disk_op(&dop);

    سیٹ_FARVAR(regs->ds, param_far->گنتی, dop.گنتی);

    disk_ret(regs, حالت_2);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ساکن خالی
disk_1300(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    ساخت disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.گنتی = 0;
    صحیح حالت_2 = send_disk_op(&dop);
    disk_ret(regs, حالت_2);
}

// read disk status
ساکن خالی
disk_1301(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    u8 v;
    اگر (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_آخری_حالت);
    ورنہ
        v = GET_BDA(disk_آخری_حالت);
    regs->ah = v;
    سیٹ_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ساکن خالی
disk_1302(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ساکن خالی
disk_1303(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ساکن خالی
disk_1304(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ساکن خالی noinline
disk_1305(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    debug_stub(regs);

    ساخت chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    u8 گنتی = regs->al;
    u8 cylinder = regs->ch;
    u8 سر = regs->dh;

    اگر (cylinder >= nlc || سر >= nlh || گنتی == 0 || گنتی > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }

    ساخت disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)سر) * (u32)nls;
    dop.گنتی = گنتی;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    صحیح حالت_2 = send_disk_op(&dop);
    disk_ret(regs, حالت_2);
}

// read disk drive parameters
ساکن خالی noinline
disk_1308(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    // Get logical geometry from table
    ساخت chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;
    nlc--;
    nlh--;
    u8 گنتی;
    اگر (regs->dl < EXTSTART_HD) {
        // Floppy
        گنتی = GET_GLOBAL(Floppy_گنتی);

        اگر (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_کم(CDEmu.media) * 2;
        ورنہ
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } ورنہ اگر (regs->dl < EXTSTART_CD) {
        // Hard drive
        گنتی = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } ورنہ {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }

    اگر (CONFIG_CDROM_EMU && GET_کم(CDEmu.media)) {
        u8 emudrive = GET_کم(CDEmu.emulated_drive);
        اگر (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            گنتی++;
        اگر (regs->dl < EXTSTART_HD && گنتی > 2)
            // Max of two floppy drives.
            گنتی = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = گنتی;
}

// initialize drive parameters
ساکن خالی
disk_1309(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ساکن خالی
disk_130c(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ساکن خالی
disk_130d(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ساکن خالی
disk_1310(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    // should look at 40:8E also???

    ساخت disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.گنتی = 0;
    صحیح حالت_2 = send_disk_op(&dop);
    disk_ret(regs, حالت_2);
}

// recalibrate
ساکن خالی
disk_1311(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ساکن خالی
disk_1314(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ساکن خالی noinline
disk_1315(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    اگر (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        واپس;
    }
    // Hard drive

    // Get logical geometry from table
    ساخت chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.سر, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ساکن خالی
disk_1316(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    اگر (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ساکن خالی
disk_1341(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ساکن خالی
disk_1342(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ساکن خالی
disk_1343(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ساکن خالی
disk_1344(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

//تالا
ساکن خالی
disk_134500(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        واپس;
    }
    سیٹ_کم(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ساکن خالی
disk_134501(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        واپس;
    }
    locks--;
    سیٹ_کم(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

//حالت
ساکن خالی
disk_134502(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساکن خالی
disk_1345XX(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ساکن خالی
disk_1345(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        واپس;
    }

    انتخاب (regs->al) {
    حالت 0x00: disk_134500(regs, drive_fl); توڑو;
    حالت 0x01: disk_134501(regs, drive_fl); توڑو;
    حالت 0x02: disk_134502(regs, drive_fl); توڑو;
    طےشدہ:   disk_1345XX(regs, drive_fl); توڑو;
    }
}

// IBM/MS eject media
ساکن خالی noinline
disk_1346(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        واپس;
    }

    صحیح cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_کم(CDRom_locks[cdid]);
    اگر (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        واپس;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    اگر (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        واپس;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ساکن خالی
disk_1347(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ساکن خالی
disk_1348(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    صحیح ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ساکن خالی
disk_1349(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    اگر (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        واپس;
    }
    سیٹ_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ساکن خالی
disk_134e01(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساکن خالی
disk_134e03(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساکن خالی
disk_134e04(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساکن خالی
disk_134e06(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ساکن خالی
disk_134eXX(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ساکن خالی
disk_134e(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    انتخاب (regs->al) {
    حالت 0x01: disk_134e01(regs, drive_fl); توڑو;
    حالت 0x03: disk_134e03(regs, drive_fl); توڑو;
    حالت 0x04: disk_134e04(regs, drive_fl); توڑو;
    حالت 0x06: disk_134e06(regs, drive_fl); توڑو;
    طےشدہ:   disk_134eXX(regs, drive_fl); توڑو;
    }
}

ساکن خالی
disk_13XX(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ساکن خالی
disk_13(ساخت bregs *regs, ساخت drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    سیٹ_BDA(disk_مداخلت_flag, 0);

    انتخاب (regs->ah) {
    حالت 0x00: disk_1300(regs, drive_fl); توڑو;
    حالت 0x01: disk_1301(regs, drive_fl); توڑو;
    حالت 0x02: disk_1302(regs, drive_fl); توڑو;
    حالت 0x03: disk_1303(regs, drive_fl); توڑو;
    حالت 0x04: disk_1304(regs, drive_fl); توڑو;
    حالت 0x05: disk_1305(regs, drive_fl); توڑو;
    حالت 0x08: disk_1308(regs, drive_fl); توڑو;
    حالت 0x09: disk_1309(regs, drive_fl); توڑو;
    حالت 0x0c: disk_130c(regs, drive_fl); توڑو;
    حالت 0x0d: disk_130d(regs, drive_fl); توڑو;
    حالت 0x10: disk_1310(regs, drive_fl); توڑو;
    حالت 0x11: disk_1311(regs, drive_fl); توڑو;
    حالت 0x14: disk_1314(regs, drive_fl); توڑو;
    حالت 0x15: disk_1315(regs, drive_fl); توڑو;
    حالت 0x16: disk_1316(regs, drive_fl); توڑو;
    حالت 0x41: disk_1341(regs, drive_fl); توڑو;
    حالت 0x42: disk_1342(regs, drive_fl); توڑو;
    حالت 0x43: disk_1343(regs, drive_fl); توڑو;
    حالت 0x44: disk_1344(regs, drive_fl); توڑو;
    حالت 0x45: disk_1345(regs, drive_fl); توڑو;
    حالت 0x46: disk_1346(regs, drive_fl); توڑو;
    حالت 0x47: disk_1347(regs, drive_fl); توڑو;
    حالت 0x48: disk_1348(regs, drive_fl); توڑو;
    حالت 0x49: disk_1349(regs, drive_fl); توڑو;
    حالت 0x4e: disk_134e(regs, drive_fl); توڑو;
    طےشدہ:   disk_13XX(regs, drive_fl); توڑو;
    }
}

ساکن خالی
floppy_13(ساخت bregs *regs, ساخت drive_s *drive_fl)
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
        توڑو;
    طےشدہ:   disk_13XX(regs, drive_fl); توڑو;
    }
}

// ElTorito - Terminate disk emu
ساکن خالی
cdemu_134b(ساخت bregs *regs)
{
    memcpy_far(regs->ds, (خالی*)(regs->si+0), SEG_کم, &CDEmu, حجم(CDEmu));

    // If we have to terminate emulation
    اگر (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        سیٹ_کم(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ساکن خالی
handle_legacy_disk(ساخت bregs *regs, u8 extdrive)
{
    اگر (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        واپس;
    }

    اگر (extdrive < EXTSTART_HD) {
        ساخت drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        اگر (!drive_fl)
            جاؤ fail;
        floppy_13(regs, drive_fl);
        واپس;
    }

    ساخت drive_s *drive_fl;
    اگر (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    ورنہ
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    اگر (!drive_fl)
        جاؤ fail;
    disk_13(regs, drive_fl);
    واپس;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

خالی VISIBLE16
handle_40(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
خالی VISIBLE16
handle_13(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    اگر (CONFIG_CDROM_EMU) {
        اگر (regs->ah == 0x4b) {
            cdemu_134b(regs);
            واپس;
        }
        اگر (GET_کم(CDEmu.media)) {
            u8 emudrive = GET_کم(CDEmu.emulated_drive);
            اگر (extdrive == emudrive) {
                // Access to an emulated drive.
                ساخت drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                اگر (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    واپس;
                }
                disk_13(regs, cdemu_gf);
                واپس;
            }
            اگر (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
خالی VISIBLE16
handle_76(خالی)
{
    debug_isr(DEBUG_ISR_76);
    سیٹ_BDA(disk_مداخلت_flag, 0xff);
    pic_eoi2();
}
