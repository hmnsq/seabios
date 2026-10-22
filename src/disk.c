// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "hw/ata.h" // ATA_CB_DC
#అన్తర్భావ "hw/pic.h" // pic_eoi2
#అన్తర్భావ "output.h" // debug_enter
#అన్తర్భావ "stacks.h" // call16_int
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

స్థిర శూన్య
__disk_ret(నిర్మాణం bregs *regs, u32 linecode, స్థిరరాశి అక్షరం *fname)
{
    u8 సంకేతం_2 = linecode;
    ఒకవేళ (regs->dl < EXTSTART_HD)
        సమితి_BDA(floppy_చివరి_స్థితి, సంకేతం_2);
    లేకపోతే
        సమితి_BDA(disk_చివరి_స్థితి, సంకేతం_2);
    ఒకవేళ (సంకేతం_2)
        __set_code_invalid(regs, linecode, fname);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
__disk_ret_unimplemented(నిర్మాణం bregs *regs, u32 linecode, స్థిరరాశి అక్షరం *fname)
{
    u8 సంకేతం_2 = linecode;
    ఒకవేళ (regs->dl < EXTSTART_HD)
        సమితి_BDA(floppy_చివరి_స్థితి, సంకేతం_2);
    లేకపోతే
        సమితి_BDA(disk_చివరి_స్థితి, సంకేతం_2);
    __set_code_unimplemented(regs, linecode, fname);
}

స్థిర శూన్య
__disk_stub(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    __disk_ret(regs, DISK_RET_SUCCESS | (lineno << 8), fname);
}

#నిర్వచించు disk_ret(regs, సంకేతం_2) \
    __disk_ret((regs), (సంకేతం_2) | (__LINE__ << 8), __func__)
#నిర్వచించు disk_ret_unimplemented(regs, సంకేతం_2) \
    __disk_ret_unimplemented((regs), (సంకేతం_2) | (__LINE__ << 8), __func__)
#నిర్వచించు DISK_STUB(regs)                         \
    __disk_stub((regs), __LINE__, __func__)


/****************************************************************
 * Helper functions
 ****************************************************************/

// Get the cylinders/heads/sectors for the given drive.
స్థిర నిర్మాణం chs_s
getLCHS(నిర్మాణం drive_s *drive_fl)
{
    నిర్మాణం chs_s res = { };
    ఒకవేళ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_దిగువ(CDEmu.chs.sptcyl);
        res.cylinder = GET_దిగువ(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.తల = GET_దిగువ(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        తిరిగి res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.తల = GET_FLATPTR(drive_fl->lchs.తల);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    తిరిగి res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
స్థిర పూర్ణాంకం
__send_disk_op(నిర్మాణం disk_op_s *op_far, u16 op_seg)
{
    నిర్మాణం disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, పరిమాణం(dop));

    పూర్ణాంకం స్థితి_2 = process_op(&dop);

    // Update count with total sectors transferred.
    సమితి_FARVAR(op_seg, op_far->లెక్క, dop.లెక్క);

    తిరిగి స్థితి_2;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
స్థిర పూర్ణాంకం
send_disk_op(నిర్మాణం disk_op_s *op)
{
    ASSERT16();
    ఒకవేళ (! CONFIG_DRIVES)
        తిరిగి -1;
    ఒకవేళ (!CONFIG_నమోదు_EXTRASTACK)
        // Jump on to extra stack
        తిరిగి దొంతర_hop(__send_disk_op, op, GET_SEG(SS));
    తిరిగి process_op(op);
}

// Perform read/write/verify using old-style chs accesses
స్థిర శూన్య noinline
basic_access(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl, u16 command)
{
    నిర్మాణం disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 లెక్క = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 తల = regs->dh;

    ఒకవేళ (లెక్క > 128 || లెక్క == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }
    dop.లెక్క = లెక్క;

    నిర్మాణం chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.తల, nls=chs.sector;

    // sanity check on cyl heads, sec
    ఒకవేళ (cylinder >= nlc || తల >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)తల) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    పూర్ణాంకం స్థితి_2 = send_disk_op(&dop);

    regs->al = dop.లెక్క;

    disk_ret(regs, స్థితి_2);
}

// Perform read/write/verify using new-style "int13ext" accesses.
స్థిర శూన్య noinline
extended_access(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl, u16 command)
{
    నిర్మాణం disk_op_s dop;
    నిర్మాణం int13ext_s *param_far = (నిర్మాణం int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    ఒకవేళ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }

    dop.buf_fl = SEGOFF_గమ్యం_FLATPTR(GET_FARVAR(regs->ds, param_far->దత్తాంశం));
    dop.లెక్క = GET_FARVAR(regs->ds, param_far->లెక్క);
    ఒకవేళ (! dop.లెక్క) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        తిరిగి;
    }

    పూర్ణాంకం స్థితి_2 = send_disk_op(&dop);

    సమితి_FARVAR(regs->ds, param_far->లెక్క, dop.లెక్క);

    disk_ret(regs, స్థితి_2);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
స్థిర శూన్య
disk_1300(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    నిర్మాణం disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.లెక్క = 0;
    పూర్ణాంకం స్థితి_2 = send_disk_op(&dop);
    disk_ret(regs, స్థితి_2);
}

// read disk status
స్థిర శూన్య
disk_1301(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    u8 v;
    ఒకవేళ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_చివరి_స్థితి);
    లేకపోతే
        v = GET_BDA(disk_చివరి_స్థితి);
    regs->ah = v;
    సమితి_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
స్థిర శూన్య
disk_1302(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
స్థిర శూన్య
disk_1303(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
స్థిర శూన్య
disk_1304(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
స్థిర శూన్య noinline
disk_1305(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    debug_stub(regs);

    నిర్మాణం chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.తల, nls=chs.sector;

    u8 లెక్క = regs->al;
    u8 cylinder = regs->ch;
    u8 తల = regs->dh;

    ఒకవేళ (cylinder >= nlc || తల >= nlh || లెక్క == 0 || లెక్క > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }

    నిర్మాణం disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)తల) * (u32)nls;
    dop.లెక్క = లెక్క;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    పూర్ణాంకం స్థితి_2 = send_disk_op(&dop);
    disk_ret(regs, స్థితి_2);
}

// read disk drive parameters
స్థిర శూన్య noinline
disk_1308(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    // Get logical geometry from table
    నిర్మాణం chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.తల, nls=chs.sector;
    nlc--;
    nlh--;
    u8 లెక్క;
    ఒకవేళ (regs->dl < EXTSTART_HD) {
        // Floppy
        లెక్క = GET_GLOBAL(Floppy_లెక్క);

        ఒకవేళ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_దిగువ(CDEmu.media) * 2;
        లేకపోతే
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } లేకపోతే ఒకవేళ (regs->dl < EXTSTART_CD) {
        // Hard drive
        లెక్క = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } లేకపోతే {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }

    ఒకవేళ (CONFIG_CDROM_EMU && GET_దిగువ(CDEmu.media)) {
        u8 emudrive = GET_దిగువ(CDEmu.emulated_drive);
        ఒకవేళ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            లెక్క++;
        ఒకవేళ (regs->dl < EXTSTART_HD && లెక్క > 2)
            // Max of two floppy drives.
            లెక్క = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = లెక్క;
}

// initialize drive parameters
స్థిర శూన్య
disk_1309(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
స్థిర శూన్య
disk_130c(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
స్థిర శూన్య
disk_130d(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
స్థిర శూన్య
disk_1310(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    // should look at 40:8E also???

    నిర్మాణం disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.లెక్క = 0;
    పూర్ణాంకం స్థితి_2 = send_disk_op(&dop);
    disk_ret(regs, స్థితి_2);
}

// recalibrate
స్థిర శూన్య
disk_1311(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
స్థిర శూన్య
disk_1314(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
స్థిర శూన్య noinline
disk_1315(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    ఒకవేళ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        తిరిగి;
    }
    // Hard drive

    // Get logical geometry from table
    నిర్మాణం chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.తల, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

స్థిర శూన్య
disk_1316(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    ఒకవేళ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
స్థిర శూన్య
disk_1341(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
స్థిర శూన్య
disk_1342(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
స్థిర శూన్య
disk_1343(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
స్థిర శూన్య
disk_1344(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

//తాళం
స్థిర శూన్య
disk_134500(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    పూర్ణాంకం cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_దిగువ(CDRom_locks[cdid]);
    ఒకవేళ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        తిరిగి;
    }
    సమితి_దిగువ(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
స్థిర శూన్య
disk_134501(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    పూర్ణాంకం cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_దిగువ(CDRom_locks[cdid]);
    ఒకవేళ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        తిరిగి;
    }
    locks--;
    సమితి_దిగువ(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

//స్థితి
స్థిర శూన్య
disk_134502(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    పూర్ణాంకం cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_దిగువ(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

స్థిర శూన్య
disk_1345XX(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
స్థిర శూన్య
disk_1345(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    ఒకవేళ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        తిరిగి;
    }

    ఎంపిక (regs->al) {
    స్థితి 0x00: disk_134500(regs, drive_fl); ఆపు;
    స్థితి 0x01: disk_134501(regs, drive_fl); ఆపు;
    స్థితి 0x02: disk_134502(regs, drive_fl); ఆపు;
    అప్రమేయ:   disk_1345XX(regs, drive_fl); ఆపు;
    }
}

// IBM/MS eject media
స్థిర శూన్య noinline
disk_1346(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    ఒకవేళ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        తిరిగి;
    }

    పూర్ణాంకం cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_దిగువ(CDRom_locks[cdid]);
    ఒకవేళ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        తిరిగి;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    ఒకవేళ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        తిరిగి;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
స్థిర శూన్య
disk_1347(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
స్థిర శూన్య
disk_1348(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    పూర్ణాంకం ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
స్థిర శూన్య
disk_1349(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    ఒకవేళ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        తిరిగి;
    }
    సమితి_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

స్థిర శూన్య
disk_134e01(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

స్థిర శూన్య
disk_134e03(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

స్థిర శూన్య
disk_134e04(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

స్థిర శూన్య
disk_134e06(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

స్థిర శూన్య
disk_134eXX(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
స్థిర శూన్య
disk_134e(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    ఎంపిక (regs->al) {
    స్థితి 0x01: disk_134e01(regs, drive_fl); ఆపు;
    స్థితి 0x03: disk_134e03(regs, drive_fl); ఆపు;
    స్థితి 0x04: disk_134e04(regs, drive_fl); ఆపు;
    స్థితి 0x06: disk_134e06(regs, drive_fl); ఆపు;
    అప్రమేయ:   disk_134eXX(regs, drive_fl); ఆపు;
    }
}

స్థిర శూన్య
disk_13XX(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

స్థిర శూన్య
disk_13(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    సమితి_BDA(disk_అంతరాయం_flag, 0);

    ఎంపిక (regs->ah) {
    స్థితి 0x00: disk_1300(regs, drive_fl); ఆపు;
    స్థితి 0x01: disk_1301(regs, drive_fl); ఆపు;
    స్థితి 0x02: disk_1302(regs, drive_fl); ఆపు;
    స్థితి 0x03: disk_1303(regs, drive_fl); ఆపు;
    స్థితి 0x04: disk_1304(regs, drive_fl); ఆపు;
    స్థితి 0x05: disk_1305(regs, drive_fl); ఆపు;
    స్థితి 0x08: disk_1308(regs, drive_fl); ఆపు;
    స్థితి 0x09: disk_1309(regs, drive_fl); ఆపు;
    స్థితి 0x0c: disk_130c(regs, drive_fl); ఆపు;
    స్థితి 0x0d: disk_130d(regs, drive_fl); ఆపు;
    స్థితి 0x10: disk_1310(regs, drive_fl); ఆపు;
    స్థితి 0x11: disk_1311(regs, drive_fl); ఆపు;
    స్థితి 0x14: disk_1314(regs, drive_fl); ఆపు;
    స్థితి 0x15: disk_1315(regs, drive_fl); ఆపు;
    స్థితి 0x16: disk_1316(regs, drive_fl); ఆపు;
    స్థితి 0x41: disk_1341(regs, drive_fl); ఆపు;
    స్థితి 0x42: disk_1342(regs, drive_fl); ఆపు;
    స్థితి 0x43: disk_1343(regs, drive_fl); ఆపు;
    స్థితి 0x44: disk_1344(regs, drive_fl); ఆపు;
    స్థితి 0x45: disk_1345(regs, drive_fl); ఆపు;
    స్థితి 0x46: disk_1346(regs, drive_fl); ఆపు;
    స్థితి 0x47: disk_1347(regs, drive_fl); ఆపు;
    స్థితి 0x48: disk_1348(regs, drive_fl); ఆపు;
    స్థితి 0x49: disk_1349(regs, drive_fl); ఆపు;
    స్థితి 0x4e: disk_134e(regs, drive_fl); ఆపు;
    అప్రమేయ:   disk_13XX(regs, drive_fl); ఆపు;
    }
}

స్థిర శూన్య
floppy_13(నిర్మాణం bregs *regs, నిర్మాణం drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    ఎంపిక (regs->ah) {
    స్థితి 0x00:
    స్థితి 0x01:
    స్థితి 0x02:
    స్థితి 0x03:
    స్థితి 0x04:
    స్థితి 0x05:
    స్థితి 0x08:
    స్థితి 0x15:
    స్థితి 0x16:
        disk_13(regs, drive_fl);
        ఆపు;
    అప్రమేయ:   disk_13XX(regs, drive_fl); ఆపు;
    }
}

// ElTorito - Terminate disk emu
స్థిర శూన్య
cdemu_134b(నిర్మాణం bregs *regs)
{
    memcpy_far(regs->ds, (శూన్య*)(regs->si+0), SEG_దిగువ, &CDEmu, పరిమాణం(CDEmu));

    // If we have to terminate emulation
    ఒకవేళ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        సమితి_దిగువ(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

స్థిర శూన్య
handle_legacy_disk(నిర్మాణం bregs *regs, u8 extdrive)
{
    ఒకవేళ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        తిరిగి;
    }

    ఒకవేళ (extdrive < EXTSTART_HD) {
        నిర్మాణం drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        ఒకవేళ (!drive_fl)
            వెళ్ళు fail;
        floppy_13(regs, drive_fl);
        తిరిగి;
    }

    నిర్మాణం drive_s *drive_fl;
    ఒకవేళ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    లేకపోతే
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    ఒకవేళ (!drive_fl)
        వెళ్ళు fail;
    disk_13(regs, drive_fl);
    తిరిగి;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

శూన్య VISIBLE16
handle_40(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
శూన్య VISIBLE16
handle_13(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    ఒకవేళ (CONFIG_CDROM_EMU) {
        ఒకవేళ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            తిరిగి;
        }
        ఒకవేళ (GET_దిగువ(CDEmu.media)) {
            u8 emudrive = GET_దిగువ(CDEmu.emulated_drive);
            ఒకవేళ (extdrive == emudrive) {
                // Access to an emulated drive.
                నిర్మాణం drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                ఒకవేళ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    తిరిగి;
                }
                disk_13(regs, cdemu_gf);
                తిరిగి;
            }
            ఒకవేళ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
శూన్య VISIBLE16
handle_76(శూన్య)
{
    debug_isr(DEBUG_ISR_76);
    సమితి_BDA(disk_అంతరాయం_flag, 0xff);
    pic_eoi2();
}
