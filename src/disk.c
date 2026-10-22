// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // SET_BDA
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "hw/ata.h" // ATA_CB_DC
#அந்தர்பாவ "hw/pic.h" // pic_eoi2
#அந்தர்பாவ "output.h" // debug_enter
#அந்தர்பாவ "stacks.h" // call16_int
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

நிலையான வெறுமை
__disk_ret(கட்டமைப்பு bregs *regs, u32 linecode, மாறிலி எழுத்து *fname)
{
    u8 code = linecode;
    எனில் (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    இல்லையெனில்
        SET_BDA(disk_last_status, code);
    எனில் (code)
        __set_code_invalid(regs, linecode, fname);
    இல்லையெனில்
        set_code_success(regs);
}

நிலையான வெறுமை
__disk_ret_unimplemented(கட்டமைப்பு bregs *regs, u32 linecode, மாறிலி எழுத்து *fname)
{
    u8 code = linecode;
    எனில் (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    இல்லையெனில்
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

நிலையான வெறுமை
__disk_stub(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
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
நிலையான கட்டமைப்பு chs_s
getLCHS(கட்டமைப்பு drive_s *drive_fl)
{
    கட்டமைப்பு chs_s res = { };
    எனில் (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_கீழ்(CDEmu.chs.sptcyl);
        res.cylinder = GET_கீழ்(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_கீழ்(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        திரும்பு res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    திரும்பு res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
நிலையான முழு
__send_disk_op(கட்டமைப்பு disk_op_s *op_far, u16 op_seg)
{
    கட்டமைப்பு disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, அளவு(dop));

    முழு status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->மொத்தம், dop.மொத்தம்);

    திரும்பு status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
நிலையான முழு
send_disk_op(கட்டமைப்பு disk_op_s *op)
{
    ASSERT16();
    எனில் (! CONFIG_DRIVES)
        திரும்பு -1;
    எனில் (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        திரும்பு அடுக்கு_hop(__send_disk_op, op, GET_SEG(SS));
    திரும்பு process_op(op);
}

// Perform read/write/verify using old-style chs accesses
நிலையான வெறுமை noinline
basic_access(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl, u16 command)
{
    கட்டமைப்பு disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 மொத்தம் = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    எனில் (மொத்தம் > 128 || மொத்தம் == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }
    dop.மொத்தம் = மொத்தம்;

    கட்டமைப்பு chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    எனில் (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    முழு status = send_disk_op(&dop);

    regs->al = dop.மொத்தம்;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
நிலையான வெறுமை noinline
extended_access(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl, u16 command)
{
    கட்டமைப்பு disk_op_s dop;
    கட்டமைப்பு int13ext_s *param_far = (கட்டமைப்பு int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    எனில் (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.மொத்தம் = GET_FARVAR(regs->ds, param_far->மொத்தம்);
    எனில் (! dop.மொத்தம்) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        திரும்பு;
    }

    முழு status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->மொத்தம், dop.மொத்தம்);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
நிலையான வெறுமை
disk_1300(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    கட்டமைப்பு disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.மொத்தம் = 0;
    முழு status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
நிலையான வெறுமை
disk_1301(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    u8 v;
    எனில் (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    இல்லையெனில்
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
நிலையான வெறுமை
disk_1302(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
நிலையான வெறுமை
disk_1303(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
நிலையான வெறுமை
disk_1304(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
நிலையான வெறுமை noinline
disk_1305(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    debug_stub(regs);

    கட்டமைப்பு chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 மொத்தம் = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    எனில் (cylinder >= nlc || head >= nlh || மொத்தம் == 0 || மொத்தம் > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }

    கட்டமைப்பு disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.மொத்தம் = மொத்தம்;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    முழு status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
நிலையான வெறுமை noinline
disk_1308(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    // Get logical geometry from table
    கட்டமைப்பு chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 மொத்தம்;
    எனில் (regs->dl < EXTSTART_HD) {
        // Floppy
        மொத்தம் = GET_GLOBAL(Floppy_மொத்தம்);

        எனில் (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_கீழ்(CDEmu.media) * 2;
        இல்லையெனில்
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } இல்லையெனில் எனில் (regs->dl < EXTSTART_CD) {
        // Hard drive
        மொத்தம் = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } இல்லையெனில் {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }

    எனில் (CONFIG_CDROM_EMU && GET_கீழ்(CDEmu.media)) {
        u8 emudrive = GET_கீழ்(CDEmu.emulated_drive);
        எனில் (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            மொத்தம்++;
        எனில் (regs->dl < EXTSTART_HD && மொத்தம் > 2)
            // Max of two floppy drives.
            மொத்தம் = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = மொத்தம்;
}

// initialize drive parameters
நிலையான வெறுமை
disk_1309(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
நிலையான வெறுமை
disk_130c(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
நிலையான வெறுமை
disk_130d(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
நிலையான வெறுமை
disk_1310(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    // should look at 40:8E also???

    கட்டமைப்பு disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.மொத்தம் = 0;
    முழு status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
நிலையான வெறுமை
disk_1311(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
நிலையான வெறுமை
disk_1314(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
நிலையான வெறுமை noinline
disk_1315(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    எனில் (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        திரும்பு;
    }
    // Hard drive

    // Get logical geometry from table
    கட்டமைப்பு chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

நிலையான வெறுமை
disk_1316(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    எனில் (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
நிலையான வெறுமை
disk_1341(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
நிலையான வெறுமை
disk_1342(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
நிலையான வெறுமை
disk_1343(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
நிலையான வெறுமை
disk_1344(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
நிலையான வெறுமை
disk_134500(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    முழு cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_கீழ்(CDRom_locks[cdid]);
    எனில் (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        திரும்பு;
    }
    SET_கீழ்(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
நிலையான வெறுமை
disk_134501(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    முழு cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_கீழ்(CDRom_locks[cdid]);
    எனில் (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        திரும்பு;
    }
    locks--;
    SET_கீழ்(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
நிலையான வெறுமை
disk_134502(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    முழு cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_கீழ்(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

நிலையான வெறுமை
disk_1345XX(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
நிலையான வெறுமை
disk_1345(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    எனில் (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        திரும்பு;
    }

    தேர்வு (regs->al) {
    நிலை 0x00: disk_134500(regs, drive_fl); நிறுத்து;
    நிலை 0x01: disk_134501(regs, drive_fl); நிறுத்து;
    நிலை 0x02: disk_134502(regs, drive_fl); நிறுத்து;
    இயல்பு:   disk_1345XX(regs, drive_fl); நிறுத்து;
    }
}

// IBM/MS eject media
நிலையான வெறுமை noinline
disk_1346(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    எனில் (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        திரும்பு;
    }

    முழு cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_கீழ்(CDRom_locks[cdid]);
    எனில் (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        திரும்பு;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    எனில் (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        திரும்பு;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
நிலையான வெறுமை
disk_1347(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
நிலையான வெறுமை
disk_1348(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    முழு ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
நிலையான வெறுமை
disk_1349(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    எனில் (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        திரும்பு;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

நிலையான வெறுமை
disk_134e01(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

நிலையான வெறுமை
disk_134e03(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

நிலையான வெறுமை
disk_134e04(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

நிலையான வெறுமை
disk_134e06(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

நிலையான வெறுமை
disk_134eXX(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
நிலையான வெறுமை
disk_134e(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    தேர்வு (regs->al) {
    நிலை 0x01: disk_134e01(regs, drive_fl); நிறுத்து;
    நிலை 0x03: disk_134e03(regs, drive_fl); நிறுத்து;
    நிலை 0x04: disk_134e04(regs, drive_fl); நிறுத்து;
    நிலை 0x06: disk_134e06(regs, drive_fl); நிறுத்து;
    இயல்பு:   disk_134eXX(regs, drive_fl); நிறுத்து;
    }
}

நிலையான வெறுமை
disk_13XX(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

நிலையான வெறுமை
disk_13(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    தேர்வு (regs->ah) {
    நிலை 0x00: disk_1300(regs, drive_fl); நிறுத்து;
    நிலை 0x01: disk_1301(regs, drive_fl); நிறுத்து;
    நிலை 0x02: disk_1302(regs, drive_fl); நிறுத்து;
    நிலை 0x03: disk_1303(regs, drive_fl); நிறுத்து;
    நிலை 0x04: disk_1304(regs, drive_fl); நிறுத்து;
    நிலை 0x05: disk_1305(regs, drive_fl); நிறுத்து;
    நிலை 0x08: disk_1308(regs, drive_fl); நிறுத்து;
    நிலை 0x09: disk_1309(regs, drive_fl); நிறுத்து;
    நிலை 0x0c: disk_130c(regs, drive_fl); நிறுத்து;
    நிலை 0x0d: disk_130d(regs, drive_fl); நிறுத்து;
    நிலை 0x10: disk_1310(regs, drive_fl); நிறுத்து;
    நிலை 0x11: disk_1311(regs, drive_fl); நிறுத்து;
    நிலை 0x14: disk_1314(regs, drive_fl); நிறுத்து;
    நிலை 0x15: disk_1315(regs, drive_fl); நிறுத்து;
    நிலை 0x16: disk_1316(regs, drive_fl); நிறுத்து;
    நிலை 0x41: disk_1341(regs, drive_fl); நிறுத்து;
    நிலை 0x42: disk_1342(regs, drive_fl); நிறுத்து;
    நிலை 0x43: disk_1343(regs, drive_fl); நிறுத்து;
    நிலை 0x44: disk_1344(regs, drive_fl); நிறுத்து;
    நிலை 0x45: disk_1345(regs, drive_fl); நிறுத்து;
    நிலை 0x46: disk_1346(regs, drive_fl); நிறுத்து;
    நிலை 0x47: disk_1347(regs, drive_fl); நிறுத்து;
    நிலை 0x48: disk_1348(regs, drive_fl); நிறுத்து;
    நிலை 0x49: disk_1349(regs, drive_fl); நிறுத்து;
    நிலை 0x4e: disk_134e(regs, drive_fl); நிறுத்து;
    இயல்பு:   disk_13XX(regs, drive_fl); நிறுத்து;
    }
}

நிலையான வெறுமை
floppy_13(கட்டமைப்பு bregs *regs, கட்டமைப்பு drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    தேர்வு (regs->ah) {
    நிலை 0x00:
    நிலை 0x01:
    நிலை 0x02:
    நிலை 0x03:
    நிலை 0x04:
    நிலை 0x05:
    நிலை 0x08:
    நிலை 0x15:
    நிலை 0x16:
        disk_13(regs, drive_fl);
        நிறுத்து;
    இயல்பு:   disk_13XX(regs, drive_fl); நிறுத்து;
    }
}

// ElTorito - Terminate disk emu
நிலையான வெறுமை
cdemu_134b(கட்டமைப்பு bregs *regs)
{
    memcpy_far(regs->ds, (வெறுமை*)(regs->si+0), SEG_கீழ், &CDEmu, அளவு(CDEmu));

    // If we have to terminate emulation
    எனில் (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_கீழ்(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

நிலையான வெறுமை
handle_legacy_disk(கட்டமைப்பு bregs *regs, u8 extdrive)
{
    எனில் (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        திரும்பு;
    }

    எனில் (extdrive < EXTSTART_HD) {
        கட்டமைப்பு drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        எனில் (!drive_fl)
            செல் fail;
        floppy_13(regs, drive_fl);
        திரும்பு;
    }

    கட்டமைப்பு drive_s *drive_fl;
    எனில் (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    இல்லையெனில்
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    எனில் (!drive_fl)
        செல் fail;
    disk_13(regs, drive_fl);
    திரும்பு;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

வெறுமை VISIBLE16
handle_40(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
வெறுமை VISIBLE16
handle_13(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    எனில் (CONFIG_CDROM_EMU) {
        எனில் (regs->ah == 0x4b) {
            cdemu_134b(regs);
            திரும்பு;
        }
        எனில் (GET_கீழ்(CDEmu.media)) {
            u8 emudrive = GET_கீழ்(CDEmu.emulated_drive);
            எனில் (extdrive == emudrive) {
                // Access to an emulated drive.
                கட்டமைப்பு drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                எனில் (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    திரும்பு;
                }
                disk_13(regs, cdemu_gf);
                திரும்பு;
            }
            எனில் (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
வெறுமை VISIBLE16
handle_76(வெறுமை)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
