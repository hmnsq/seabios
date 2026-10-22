// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // SET_BDA
#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "hw/ata.h" // ATA_CB_DC
#समावेश "hw/pic.h" // pic_eoi2
#समावेश "output.h" // debug_enter
#समावेश "stacks.h" // call16_int
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

अटल व्योम
__disk_ret(काष्ठा bregs *regs, u32 linecode, स्थिर अक्षर *fname)
{
    u8 code = linecode;
    अगर (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    अन्यथा
        SET_BDA(disk_last_status, code);
    अगर (code)
        __set_code_invalid(regs, linecode, fname);
    अन्यथा
        set_code_success(regs);
}

अटल व्योम
__disk_ret_unimplemented(काष्ठा bregs *regs, u32 linecode, स्थिर अक्षर *fname)
{
    u8 code = linecode;
    अगर (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    अन्यथा
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

अटल व्योम
__disk_stub(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#घोषणा disk_ret(regs, code) \
    __disk_ret((regs), (code) | (__LINE__ << 8), __func__)
#घोषणा disk_ret_unimplemented(regs, code) \
    __disk_ret_unimplemented((regs), (code) | (__LINE__ << 8), __func__)
#घोषणा DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
अटल काष्ठा chs_s
getLCHS(काष्ठा drive_s *drive_fl)
{
    काष्ठा chs_s res = { };
    अगर (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_निचला(CDEmu.chs.sptcyl);
        res.cylinder = GET_निचला(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.शीर्ष = GET_निचला(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        वापस res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.शीर्ष = GET_FLATPTR(drive_fl->lchs.शीर्ष);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    वापस res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
अटल पूर्णांक
__send_disk_op(काष्ठा disk_op_s *op_far, u16 op_seg)
{
    काष्ठा disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, माप(dop));

    पूर्णांक status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->गिनती, dop.गिनती);

    वापस status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
अटल पूर्णांक
send_disk_op(काष्ठा disk_op_s *op)
{
    ASSERT16();
    अगर (! CONFIG_DRIVES)
        वापस -1;
    अगर (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        वापस ढेर_hop(__send_disk_op, op, GET_SEG(SS));
    वापस process_op(op);
}

// Perform read/write/verify using old-style chs accesses
अटल व्योम noinline
basic_access(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl, u16 command)
{
    काष्ठा disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 गिनती = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 शीर्ष = regs->dh;

    अगर (गिनती > 128 || गिनती == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }
    dop.गिनती = गिनती;

    काष्ठा chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.शीर्ष, nls=chs.sector;

    // sanity check on cyl heads, sec
    अगर (cylinder >= nlc || शीर्ष >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)शीर्ष) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    पूर्णांक status = send_disk_op(&dop);

    regs->al = dop.गिनती;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
अटल व्योम noinline
extended_access(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl, u16 command)
{
    काष्ठा disk_op_s dop;
    काष्ठा int13ext_s *param_far = (काष्ठा int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    अगर (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->तथ्य));
    dop.गिनती = GET_FARVAR(regs->ds, param_far->गिनती);
    अगर (! dop.गिनती) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        वापस;
    }

    पूर्णांक status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->गिनती, dop.गिनती);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
अटल व्योम
disk_1300(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    काष्ठा disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.गिनती = 0;
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
अटल व्योम
disk_1301(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    u8 v;
    अगर (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    अन्यथा
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
अटल व्योम
disk_1302(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
अटल व्योम
disk_1303(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
अटल व्योम
disk_1304(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
अटल व्योम noinline
disk_1305(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    debug_stub(regs);

    काष्ठा chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.शीर्ष, nls=chs.sector;

    u8 गिनती = regs->al;
    u8 cylinder = regs->ch;
    u8 शीर्ष = regs->dh;

    अगर (cylinder >= nlc || शीर्ष >= nlh || गिनती == 0 || गिनती > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }

    काष्ठा disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)शीर्ष) * (u32)nls;
    dop.गिनती = गिनती;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
अटल व्योम noinline
disk_1308(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    // Get logical geometry from table
    काष्ठा chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.शीर्ष, nls=chs.sector;
    nlc--;
    nlh--;
    u8 गिनती;
    अगर (regs->dl < EXTSTART_HD) {
        // Floppy
        गिनती = GET_GLOBAL(Floppy_गिनती);

        अगर (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_निचला(CDEmu.media) * 2;
        अन्यथा
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } अन्यथा अगर (regs->dl < EXTSTART_CD) {
        // Hard drive
        गिनती = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } अन्यथा {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }

    अगर (CONFIG_CDROM_EMU && GET_निचला(CDEmu.media)) {
        u8 emudrive = GET_निचला(CDEmu.emulated_drive);
        अगर (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            गिनती++;
        अगर (regs->dl < EXTSTART_HD && गिनती > 2)
            // Max of two floppy drives.
            गिनती = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = गिनती;
}

// initialize drive parameters
अटल व्योम
disk_1309(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
अटल व्योम
disk_130c(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
अटल व्योम
disk_130d(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
अटल व्योम
disk_1310(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    // should look at 40:8E also???

    काष्ठा disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.गिनती = 0;
    पूर्णांक status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
अटल व्योम
disk_1311(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
अटल व्योम
disk_1314(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
अटल व्योम noinline
disk_1315(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    अगर (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        वापस;
    }
    // Hard drive

    // Get logical geometry from table
    काष्ठा chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.शीर्ष, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

अटल व्योम
disk_1316(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    अगर (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
अटल व्योम
disk_1341(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
अटल व्योम
disk_1342(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
अटल व्योम
disk_1343(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
अटल व्योम
disk_1344(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
अटल व्योम
disk_134500(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_निचला(CDRom_locks[cdid]);
    अगर (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        वापस;
    }
    SET_निचला(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
अटल व्योम
disk_134501(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_निचला(CDRom_locks[cdid]);
    अगर (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        वापस;
    }
    locks--;
    SET_निचला(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
अटल व्योम
disk_134502(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_निचला(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

अटल व्योम
disk_1345XX(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
अटल व्योम
disk_1345(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    अगर (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        वापस;
    }

    चयन (regs->al) {
    हाल 0x00: disk_134500(regs, drive_fl); अवरोध;
    हाल 0x01: disk_134501(regs, drive_fl); अवरोध;
    हाल 0x02: disk_134502(regs, drive_fl); अवरोध;
    शेष:   disk_1345XX(regs, drive_fl); अवरोध;
    }
}

// IBM/MS eject media
अटल व्योम noinline
disk_1346(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    अगर (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        वापस;
    }

    पूर्णांक cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_निचला(CDRom_locks[cdid]);
    अगर (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        वापस;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    अगर (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        वापस;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
अटल व्योम
disk_1347(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
अटल व्योम
disk_1348(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    पूर्णांक ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
अटल व्योम
disk_1349(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    अगर (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        वापस;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

अटल व्योम
disk_134e01(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

अटल व्योम
disk_134e03(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

अटल व्योम
disk_134e04(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

अटल व्योम
disk_134e06(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

अटल व्योम
disk_134eXX(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
अटल व्योम
disk_134e(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    चयन (regs->al) {
    हाल 0x01: disk_134e01(regs, drive_fl); अवरोध;
    हाल 0x03: disk_134e03(regs, drive_fl); अवरोध;
    हाल 0x04: disk_134e04(regs, drive_fl); अवरोध;
    हाल 0x06: disk_134e06(regs, drive_fl); अवरोध;
    शेष:   disk_134eXX(regs, drive_fl); अवरोध;
    }
}

अटल व्योम
disk_13XX(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

अटल व्योम
disk_13(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    चयन (regs->ah) {
    हाल 0x00: disk_1300(regs, drive_fl); अवरोध;
    हाल 0x01: disk_1301(regs, drive_fl); अवरोध;
    हाल 0x02: disk_1302(regs, drive_fl); अवरोध;
    हाल 0x03: disk_1303(regs, drive_fl); अवरोध;
    हाल 0x04: disk_1304(regs, drive_fl); अवरोध;
    हाल 0x05: disk_1305(regs, drive_fl); अवरोध;
    हाल 0x08: disk_1308(regs, drive_fl); अवरोध;
    हाल 0x09: disk_1309(regs, drive_fl); अवरोध;
    हाल 0x0c: disk_130c(regs, drive_fl); अवरोध;
    हाल 0x0d: disk_130d(regs, drive_fl); अवरोध;
    हाल 0x10: disk_1310(regs, drive_fl); अवरोध;
    हाल 0x11: disk_1311(regs, drive_fl); अवरोध;
    हाल 0x14: disk_1314(regs, drive_fl); अवरोध;
    हाल 0x15: disk_1315(regs, drive_fl); अवरोध;
    हाल 0x16: disk_1316(regs, drive_fl); अवरोध;
    हाल 0x41: disk_1341(regs, drive_fl); अवरोध;
    हाल 0x42: disk_1342(regs, drive_fl); अवरोध;
    हाल 0x43: disk_1343(regs, drive_fl); अवरोध;
    हाल 0x44: disk_1344(regs, drive_fl); अवरोध;
    हाल 0x45: disk_1345(regs, drive_fl); अवरोध;
    हाल 0x46: disk_1346(regs, drive_fl); अवरोध;
    हाल 0x47: disk_1347(regs, drive_fl); अवरोध;
    हाल 0x48: disk_1348(regs, drive_fl); अवरोध;
    हाल 0x49: disk_1349(regs, drive_fl); अवरोध;
    हाल 0x4e: disk_134e(regs, drive_fl); अवरोध;
    शेष:   disk_13XX(regs, drive_fl); अवरोध;
    }
}

अटल व्योम
floppy_13(काष्ठा bregs *regs, काष्ठा drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    चयन (regs->ah) {
    हाल 0x00:
    हाल 0x01:
    हाल 0x02:
    हाल 0x03:
    हाल 0x04:
    हाल 0x05:
    हाल 0x08:
    हाल 0x15:
    हाल 0x16:
        disk_13(regs, drive_fl);
        अवरोध;
    शेष:   disk_13XX(regs, drive_fl); अवरोध;
    }
}

// ElTorito - Terminate disk emu
अटल व्योम
cdemu_134b(काष्ठा bregs *regs)
{
    memcpy_far(regs->ds, (व्योम*)(regs->si+0), SEG_निचला, &CDEmu, माप(CDEmu));

    // If we have to terminate emulation
    अगर (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_निचला(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

अटल व्योम
handle_legacy_disk(काष्ठा bregs *regs, u8 extdrive)
{
    अगर (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        वापस;
    }

    अगर (extdrive < EXTSTART_HD) {
        काष्ठा drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        अगर (!drive_fl)
            जाओ fail;
        floppy_13(regs, drive_fl);
        वापस;
    }

    काष्ठा drive_s *drive_fl;
    अगर (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    अन्यथा
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    अगर (!drive_fl)
        जाओ fail;
    disk_13(regs, drive_fl);
    वापस;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

व्योम VISIBLE16
handle_40(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
व्योम VISIBLE16
handle_13(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    अगर (CONFIG_CDROM_EMU) {
        अगर (regs->ah == 0x4b) {
            cdemu_134b(regs);
            वापस;
        }
        अगर (GET_निचला(CDEmu.media)) {
            u8 emudrive = GET_निचला(CDEmu.emulated_drive);
            अगर (extdrive == emudrive) {
                // Access to an emulated drive.
                काष्ठा drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                अगर (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    वापस;
                }
                disk_13(regs, cdemu_gf);
                वापस;
            }
            अगर (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
व्योम VISIBLE16
handle_76(व्योम)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
