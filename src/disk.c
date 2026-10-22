// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // SET_BDA
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "hw/ata.h" // ATA_CB_DC
#അന്തര്ഭാവ "hw/pic.h" // pic_eoi2
#അന്തര്ഭാവ "output.h" // debug_enter
#അന്തര്ഭാവ "stacks.h" // call16_int
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

സ്ഥിതം ശൂന്യം
__disk_ret(ഘടന bregs *regs, u32 linecode, സ്ഥിരം അക്ഷരം *fname)
{
    u8 code = linecode;
    എങ്കിൽ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    അല്ലെങ്കിൽ
        SET_BDA(disk_last_status, code);
    എങ്കിൽ (code)
        __set_code_invalid(regs, linecode, fname);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
__disk_ret_unimplemented(ഘടന bregs *regs, u32 linecode, സ്ഥിരം അക്ഷരം *fname)
{
    u8 code = linecode;
    എങ്കിൽ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    അല്ലെങ്കിൽ
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

സ്ഥിതം ശൂന്യം
__disk_stub(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
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
സ്ഥിതം ഘടന chs_s
getLCHS(ഘടന drive_s *drive_fl)
{
    ഘടന chs_s res = { };
    എങ്കിൽ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_താഴെയുള്ള(CDEmu.chs.sptcyl);
        res.cylinder = GET_താഴെയുള്ള(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_താഴെയുള്ള(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        തിരികെ res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    തിരികെ res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
സ്ഥിതം പൂർണ്ണസംഖ്യ
__send_disk_op(ഘടന disk_op_s *op_far, u16 op_seg)
{
    ഘടന disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, വലുപ്പം(dop));

    പൂർണ്ണസംഖ്യ status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->എണ്ണം, dop.എണ്ണം);

    തിരികെ status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
സ്ഥിതം പൂർണ്ണസംഖ്യ
send_disk_op(ഘടന disk_op_s *op)
{
    ASSERT16();
    എങ്കിൽ (! CONFIG_DRIVES)
        തിരികെ -1;
    എങ്കിൽ (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        തിരികെ അട്ടി_hop(__send_disk_op, op, GET_SEG(SS));
    തിരികെ process_op(op);
}

// Perform read/write/verify using old-style chs accesses
സ്ഥിതം ശൂന്യം noinline
basic_access(ഘടന bregs *regs, ഘടന drive_s *drive_fl, u16 command)
{
    ഘടന disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 എണ്ണം = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    എങ്കിൽ (എണ്ണം > 128 || എണ്ണം == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }
    dop.എണ്ണം = എണ്ണം;

    ഘടന chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    എങ്കിൽ (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    പൂർണ്ണസംഖ്യ status = send_disk_op(&dop);

    regs->al = dop.എണ്ണം;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
സ്ഥിതം ശൂന്യം noinline
extended_access(ഘടന bregs *regs, ഘടന drive_s *drive_fl, u16 command)
{
    ഘടന disk_op_s dop;
    ഘടന int13ext_s *param_far = (ഘടന int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    എങ്കിൽ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.എണ്ണം = GET_FARVAR(regs->ds, param_far->എണ്ണം);
    എങ്കിൽ (! dop.എണ്ണം) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->എണ്ണം, dop.എണ്ണം);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
സ്ഥിതം ശൂന്യം
disk_1300(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    ഘടന disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.എണ്ണം = 0;
    പൂർണ്ണസംഖ്യ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
സ്ഥിതം ശൂന്യം
disk_1301(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    u8 v;
    എങ്കിൽ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    അല്ലെങ്കിൽ
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
സ്ഥിതം ശൂന്യം
disk_1302(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
സ്ഥിതം ശൂന്യം
disk_1303(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
സ്ഥിതം ശൂന്യം
disk_1304(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
സ്ഥിതം ശൂന്യം noinline
disk_1305(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    debug_stub(regs);

    ഘടന chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 എണ്ണം = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    എങ്കിൽ (cylinder >= nlc || head >= nlh || എണ്ണം == 0 || എണ്ണം > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }

    ഘടന disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.എണ്ണം = എണ്ണം;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    പൂർണ്ണസംഖ്യ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
സ്ഥിതം ശൂന്യം noinline
disk_1308(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    // Get logical geometry from table
    ഘടന chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 എണ്ണം;
    എങ്കിൽ (regs->dl < EXTSTART_HD) {
        // Floppy
        എണ്ണം = GET_GLOBAL(Floppy_എണ്ണം);

        എങ്കിൽ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_താഴെയുള്ള(CDEmu.media) * 2;
        അല്ലെങ്കിൽ
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } അല്ലെങ്കിൽ എങ്കിൽ (regs->dl < EXTSTART_CD) {
        // Hard drive
        എണ്ണം = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } അല്ലെങ്കിൽ {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }

    എങ്കിൽ (CONFIG_CDROM_EMU && GET_താഴെയുള്ള(CDEmu.media)) {
        u8 emudrive = GET_താഴെയുള്ള(CDEmu.emulated_drive);
        എങ്കിൽ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            എണ്ണം++;
        എങ്കിൽ (regs->dl < EXTSTART_HD && എണ്ണം > 2)
            // Max of two floppy drives.
            എണ്ണം = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = എണ്ണം;
}

// initialize drive parameters
സ്ഥിതം ശൂന്യം
disk_1309(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
സ്ഥിതം ശൂന്യം
disk_130c(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
സ്ഥിതം ശൂന്യം
disk_130d(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
സ്ഥിതം ശൂന്യം
disk_1310(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    // should look at 40:8E also???

    ഘടന disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.എണ്ണം = 0;
    പൂർണ്ണസംഖ്യ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
സ്ഥിതം ശൂന്യം
disk_1311(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
സ്ഥിതം ശൂന്യം
disk_1314(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
സ്ഥിതം ശൂന്യം noinline
disk_1315(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    എങ്കിൽ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        തിരികെ;
    }
    // Hard drive

    // Get logical geometry from table
    ഘടന chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

സ്ഥിതം ശൂന്യം
disk_1316(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    എങ്കിൽ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
സ്ഥിതം ശൂന്യം
disk_1341(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
സ്ഥിതം ശൂന്യം
disk_1342(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
സ്ഥിതം ശൂന്യം
disk_1343(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
സ്ഥിതം ശൂന്യം
disk_1344(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
സ്ഥിതം ശൂന്യം
disk_134500(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    പൂർണ്ണസംഖ്യ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_താഴെയുള്ള(CDRom_locks[cdid]);
    എങ്കിൽ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        തിരികെ;
    }
    SET_താഴെയുള്ള(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
സ്ഥിതം ശൂന്യം
disk_134501(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    പൂർണ്ണസംഖ്യ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_താഴെയുള്ള(CDRom_locks[cdid]);
    എങ്കിൽ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        തിരികെ;
    }
    locks--;
    SET_താഴെയുള്ള(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
സ്ഥിതം ശൂന്യം
disk_134502(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    പൂർണ്ണസംഖ്യ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_താഴെയുള്ള(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

സ്ഥിതം ശൂന്യം
disk_1345XX(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
സ്ഥിതം ശൂന്യം
disk_1345(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    എങ്കിൽ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x00: disk_134500(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x01: disk_134501(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x02: disk_134502(regs, drive_fl); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   disk_1345XX(regs, drive_fl); നിർത്തുക;
    }
}

// IBM/MS eject media
സ്ഥിതം ശൂന്യം noinline
disk_1346(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    എങ്കിൽ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_താഴെയുള്ള(CDRom_locks[cdid]);
    എങ്കിൽ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        തിരികെ;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    എങ്കിൽ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        തിരികെ;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
സ്ഥിതം ശൂന്യം
disk_1347(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
സ്ഥിതം ശൂന്യം
disk_1348(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    പൂർണ്ണസംഖ്യ ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
സ്ഥിതം ശൂന്യം
disk_1349(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    എങ്കിൽ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        തിരികെ;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

സ്ഥിതം ശൂന്യം
disk_134e01(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

സ്ഥിതം ശൂന്യം
disk_134e03(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

സ്ഥിതം ശൂന്യം
disk_134e04(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

സ്ഥിതം ശൂന്യം
disk_134e06(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

സ്ഥിതം ശൂന്യം
disk_134eXX(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
സ്ഥിതം ശൂന്യം
disk_134e(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x01: disk_134e01(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x03: disk_134e03(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x04: disk_134e04(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x06: disk_134e06(regs, drive_fl); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   disk_134eXX(regs, drive_fl); നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
disk_13XX(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

സ്ഥിതം ശൂന്യം
disk_13(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x00: disk_1300(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x01: disk_1301(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x02: disk_1302(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x03: disk_1303(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x04: disk_1304(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x05: disk_1305(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x08: disk_1308(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x09: disk_1309(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x0c: disk_130c(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x0d: disk_130d(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x10: disk_1310(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x11: disk_1311(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x14: disk_1314(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x15: disk_1315(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x16: disk_1316(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x41: disk_1341(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x42: disk_1342(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x43: disk_1343(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x44: disk_1344(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x45: disk_1345(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x46: disk_1346(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x47: disk_1347(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x48: disk_1348(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x49: disk_1349(regs, drive_fl); നിർത്തുക;
    അവസ്ഥ 0x4e: disk_134e(regs, drive_fl); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   disk_13XX(regs, drive_fl); നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
floppy_13(ഘടന bregs *regs, ഘടന drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x00:
    അവസ്ഥ 0x01:
    അവസ്ഥ 0x02:
    അവസ്ഥ 0x03:
    അവസ്ഥ 0x04:
    അവസ്ഥ 0x05:
    അവസ്ഥ 0x08:
    അവസ്ഥ 0x15:
    അവസ്ഥ 0x16:
        disk_13(regs, drive_fl);
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   disk_13XX(regs, drive_fl); നിർത്തുക;
    }
}

// ElTorito - Terminate disk emu
സ്ഥിതം ശൂന്യം
cdemu_134b(ഘടന bregs *regs)
{
    memcpy_far(regs->ds, (ശൂന്യം*)(regs->si+0), SEG_താഴെയുള്ള, &CDEmu, വലുപ്പം(CDEmu));

    // If we have to terminate emulation
    എങ്കിൽ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_താഴെയുള്ള(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

സ്ഥിതം ശൂന്യം
handle_legacy_disk(ഘടന bregs *regs, u8 extdrive)
{
    എങ്കിൽ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        തിരികെ;
    }

    എങ്കിൽ (extdrive < EXTSTART_HD) {
        ഘടന drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        എങ്കിൽ (!drive_fl)
            പോകുക fail;
        floppy_13(regs, drive_fl);
        തിരികെ;
    }

    ഘടന drive_s *drive_fl;
    എങ്കിൽ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    അല്ലെങ്കിൽ
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    എങ്കിൽ (!drive_fl)
        പോകുക fail;
    disk_13(regs, drive_fl);
    തിരികെ;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

ശൂന്യം VISIBLE16
handle_40(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
ശൂന്യം VISIBLE16
handle_13(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    എങ്കിൽ (CONFIG_CDROM_EMU) {
        എങ്കിൽ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            തിരികെ;
        }
        എങ്കിൽ (GET_താഴെയുള്ള(CDEmu.media)) {
            u8 emudrive = GET_താഴെയുള്ള(CDEmu.emulated_drive);
            എങ്കിൽ (extdrive == emudrive) {
                // Access to an emulated drive.
                ഘടന drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                എങ്കിൽ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    തിരികെ;
                }
                disk_13(regs, cdemu_gf);
                തിരികെ;
            }
            എങ്കിൽ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
ശൂന്യം VISIBLE16
handle_76(ശൂന്യം)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
