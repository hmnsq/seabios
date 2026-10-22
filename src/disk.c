// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "hw/ata.h" // ATA_CB_DC
#अन्तर्भाव "hw/pic.h" // pic_eoi2
#अन्तर्भाव "output.h" // debug_enter
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

स्थिर शून्य
__disk_ret(संरचना bregs *regs, u32 linecode, स्थिरांक अक्षर *fname)
{
    u8 code = linecode;
    जर (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    अन्यथा
        SET_BDA(disk_last_status, code);
    जर (code)
        __set_code_invalid(regs, linecode, fname);
    अन्यथा
        set_code_success(regs);
}

स्थिर शून्य
__disk_ret_unimplemented(संरचना bregs *regs, u32 linecode, स्थिरांक अक्षर *fname)
{
    u8 code = linecode;
    जर (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    अन्यथा
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

स्थिर शून्य
__disk_stub(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
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
स्थिर संरचना chs_s
getLCHS(संरचना drive_s *drive_fl)
{
    संरचना chs_s res = { };
    जर (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_खालचा(CDEmu.chs.sptcyl);
        res.cylinder = GET_खालचा(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_खालचा(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        परत res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    परत res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
स्थिर पूर्णांक
__send_disk_op(संरचना disk_op_s *op_far, u16 op_seg)
{
    संरचना disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, आकार(dop));

    पूर्णांक status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->मोजणी, dop.मोजणी);

    परत status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
स्थिर पूर्णांक
send_disk_op(संरचना disk_op_s *op)
{
    ASSERT16();
    जर (! CONFIG_DRIVES)
        परत -1;
    जर (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        परत चळत_hop(__send_disk_op, op, GET_SEG(SS));
    परत process_op(op);
}

// Perform read/write/verify using old-style chs accesses
स्थिर शून्य noinline
basic_access(संरचना bregs *regs, संरचना drive_s *drive_fl, u16 command)
{
    संरचना disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 मोजणी = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    जर (मोजणी > 128 || मोजणी == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }
    dop.मोजणी = मोजणी;

    संरचना chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    जर (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    पूर्णांक status = send_disk_op(&dop);

    regs->al = dop.मोजणी;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
स्थिर शून्य noinline
extended_access(संरचना bregs *regs, संरचना drive_s *drive_fl, u16 command)
{
    संरचना disk_op_s dop;
    संरचना int13ext_s *param_far = (संरचना int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    जर (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.मोजणी = GET_FARVAR(regs->ds, param_far->मोजणी);
    जर (! dop.मोजणी) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        परत;
    }

    पूर्णांक status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->मोजणी, dop.मोजणी);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
स्थिर शून्य
disk_1300(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    संरचना disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.मोजणी = 0;
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
स्थिर शून्य
disk_1301(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    u8 v;
    जर (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    अन्यथा
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
स्थिर शून्य
disk_1302(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
स्थिर शून्य
disk_1303(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
स्थिर शून्य
disk_1304(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
स्थिर शून्य noinline
disk_1305(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    debug_stub(regs);

    संरचना chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 मोजणी = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    जर (cylinder >= nlc || head >= nlh || मोजणी == 0 || मोजणी > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }

    संरचना disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.मोजणी = मोजणी;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
स्थिर शून्य noinline
disk_1308(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    // Get logical geometry from table
    संरचना chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 मोजणी;
    जर (regs->dl < EXTSTART_HD) {
        // Floppy
        मोजणी = GET_GLOBAL(Floppy_मोजणी);

        जर (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_खालचा(CDEmu.media) * 2;
        अन्यथा
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } अन्यथा जर (regs->dl < EXTSTART_CD) {
        // Hard drive
        मोजणी = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } अन्यथा {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }

    जर (CONFIG_CDROM_EMU && GET_खालचा(CDEmu.media)) {
        u8 emudrive = GET_खालचा(CDEmu.emulated_drive);
        जर (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            मोजणी++;
        जर (regs->dl < EXTSTART_HD && मोजणी > 2)
            // Max of two floppy drives.
            मोजणी = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = मोजणी;
}

// initialize drive parameters
स्थिर शून्य
disk_1309(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
स्थिर शून्य
disk_130c(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
स्थिर शून्य
disk_130d(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
स्थिर शून्य
disk_1310(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    // should look at 40:8E also???

    संरचना disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.मोजणी = 0;
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
स्थिर शून्य
disk_1311(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
स्थिर शून्य
disk_1314(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
स्थिर शून्य noinline
disk_1315(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    जर (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        परत;
    }
    // Hard drive

    // Get logical geometry from table
    संरचना chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

स्थिर शून्य
disk_1316(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    जर (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
स्थिर शून्य
disk_1341(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
स्थिर शून्य
disk_1342(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
स्थिर शून्य
disk_1343(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
स्थिर शून्य
disk_1344(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
स्थिर शून्य
disk_134500(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_खालचा(CDRom_locks[cdid]);
    जर (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        परत;
    }
    SET_खालचा(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
स्थिर शून्य
disk_134501(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_खालचा(CDRom_locks[cdid]);
    जर (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        परत;
    }
    locks--;
    SET_खालचा(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
स्थिर शून्य
disk_134502(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_खालचा(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

स्थिर शून्य
disk_1345XX(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
स्थिर शून्य
disk_1345(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    जर (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        परत;
    }

    निवड (regs->al) {
    स्थिती 0x00: disk_134500(regs, drive_fl); थांबा;
    स्थिती 0x01: disk_134501(regs, drive_fl); थांबा;
    स्थिती 0x02: disk_134502(regs, drive_fl); थांबा;
    पूर्वनिर्धारित:   disk_1345XX(regs, drive_fl); थांबा;
    }
}

// IBM/MS eject media
स्थिर शून्य noinline
disk_1346(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    जर (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        परत;
    }

    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_खालचा(CDRom_locks[cdid]);
    जर (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        परत;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    संरचना bregs br;
    स_रखो(&br, 0, आकार(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    जर (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        परत;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
स्थिर शून्य
disk_1347(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
स्थिर शून्य
disk_1348(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    पूर्णांक ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
स्थिर शून्य
disk_1349(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    जर (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        परत;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

स्थिर शून्य
disk_134e01(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

स्थिर शून्य
disk_134e03(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

स्थिर शून्य
disk_134e04(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

स्थिर शून्य
disk_134e06(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

स्थिर शून्य
disk_134eXX(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
स्थिर शून्य
disk_134e(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    निवड (regs->al) {
    स्थिती 0x01: disk_134e01(regs, drive_fl); थांबा;
    स्थिती 0x03: disk_134e03(regs, drive_fl); थांबा;
    स्थिती 0x04: disk_134e04(regs, drive_fl); थांबा;
    स्थिती 0x06: disk_134e06(regs, drive_fl); थांबा;
    पूर्वनिर्धारित:   disk_134eXX(regs, drive_fl); थांबा;
    }
}

स्थिर शून्य
disk_13XX(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

स्थिर शून्य
disk_13(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    निवड (regs->ah) {
    स्थिती 0x00: disk_1300(regs, drive_fl); थांबा;
    स्थिती 0x01: disk_1301(regs, drive_fl); थांबा;
    स्थिती 0x02: disk_1302(regs, drive_fl); थांबा;
    स्थिती 0x03: disk_1303(regs, drive_fl); थांबा;
    स्थिती 0x04: disk_1304(regs, drive_fl); थांबा;
    स्थिती 0x05: disk_1305(regs, drive_fl); थांबा;
    स्थिती 0x08: disk_1308(regs, drive_fl); थांबा;
    स्थिती 0x09: disk_1309(regs, drive_fl); थांबा;
    स्थिती 0x0c: disk_130c(regs, drive_fl); थांबा;
    स्थिती 0x0d: disk_130d(regs, drive_fl); थांबा;
    स्थिती 0x10: disk_1310(regs, drive_fl); थांबा;
    स्थिती 0x11: disk_1311(regs, drive_fl); थांबा;
    स्थिती 0x14: disk_1314(regs, drive_fl); थांबा;
    स्थिती 0x15: disk_1315(regs, drive_fl); थांबा;
    स्थिती 0x16: disk_1316(regs, drive_fl); थांबा;
    स्थिती 0x41: disk_1341(regs, drive_fl); थांबा;
    स्थिती 0x42: disk_1342(regs, drive_fl); थांबा;
    स्थिती 0x43: disk_1343(regs, drive_fl); थांबा;
    स्थिती 0x44: disk_1344(regs, drive_fl); थांबा;
    स्थिती 0x45: disk_1345(regs, drive_fl); थांबा;
    स्थिती 0x46: disk_1346(regs, drive_fl); थांबा;
    स्थिती 0x47: disk_1347(regs, drive_fl); थांबा;
    स्थिती 0x48: disk_1348(regs, drive_fl); थांबा;
    स्थिती 0x49: disk_1349(regs, drive_fl); थांबा;
    स्थिती 0x4e: disk_134e(regs, drive_fl); थांबा;
    पूर्वनिर्धारित:   disk_13XX(regs, drive_fl); थांबा;
    }
}

स्थिर शून्य
floppy_13(संरचना bregs *regs, संरचना drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    निवड (regs->ah) {
    स्थिती 0x00:
    स्थिती 0x01:
    स्थिती 0x02:
    स्थिती 0x03:
    स्थिती 0x04:
    स्थिती 0x05:
    स्थिती 0x08:
    स्थिती 0x15:
    स्थिती 0x16:
        disk_13(regs, drive_fl);
        थांबा;
    पूर्वनिर्धारित:   disk_13XX(regs, drive_fl); थांबा;
    }
}

// ElTorito - Terminate disk emu
स्थिर शून्य
cdemu_134b(संरचना bregs *regs)
{
    memcpy_far(regs->ds, (शून्य*)(regs->si+0), SEG_खालचा, &CDEmu, आकार(CDEmu));

    // If we have to terminate emulation
    जर (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_खालचा(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

स्थिर शून्य
handle_legacy_disk(संरचना bregs *regs, u8 extdrive)
{
    जर (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        परत;
    }

    जर (extdrive < EXTSTART_HD) {
        संरचना drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        जर (!drive_fl)
            जा fail;
        floppy_13(regs, drive_fl);
        परत;
    }

    संरचना drive_s *drive_fl;
    जर (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    अन्यथा
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    जर (!drive_fl)
        जा fail;
    disk_13(regs, drive_fl);
    परत;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

शून्य VISIBLE16
handle_40(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
शून्य VISIBLE16
handle_13(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    जर (CONFIG_CDROM_EMU) {
        जर (regs->ah == 0x4b) {
            cdemu_134b(regs);
            परत;
        }
        जर (GET_खालचा(CDEmu.media)) {
            u8 emudrive = GET_खालचा(CDEmu.emulated_drive);
            जर (extdrive == emudrive) {
                // Access to an emulated drive.
                संरचना drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                जर (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    परत;
                }
                disk_13(regs, cdemu_gf);
                परत;
            }
            जर (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
शून्य VISIBLE16
handle_76(शून्य)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
