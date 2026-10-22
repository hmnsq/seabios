// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // SET_BDA
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "hw/ata.h" // ATA_CB_DC
#ଅନ୍ତର୍ଭାଵ "hw/pic.h" // pic_eoi2
#ଅନ୍ତର୍ଭାଵ "output.h" // debug_enter
#ଅନ୍ତର୍ଭାଵ "stacks.h" // call16_int
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

ସ୍ଥିର ଶୂନ୍ୟ
__disk_ret(ସଂରଚନା bregs *regs, u32 linecode, ଅଚଳ ଅକ୍ଷର *fname)
{
    u8 code = linecode;
    ଯଦି (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    ଅନ୍ୟଥା
        SET_BDA(disk_last_status, code);
    ଯଦି (code)
        __set_code_invalid(regs, linecode, fname);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
__disk_ret_unimplemented(ସଂରଚନା bregs *regs, u32 linecode, ଅଚଳ ଅକ୍ଷର *fname)
{
    u8 code = linecode;
    ଯଦି (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    ଅନ୍ୟଥା
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

ସ୍ଥିର ଶୂନ୍ୟ
__disk_stub(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
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
ସ୍ଥିର ସଂରଚନା chs_s
getLCHS(ସଂରଚନା drive_s *drive_fl)
{
    ସଂରଚନା chs_s res = { };
    ଯଦି (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_ତଳ(CDEmu.chs.sptcyl);
        res.cylinder = GET_ତଳ(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_ତଳ(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        ଫେରନ୍ତୁ res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    ଫେରନ୍ତୁ res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
__send_disk_op(ସଂରଚନା disk_op_s *op_far, u16 op_seg)
{
    ସଂରଚନା disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, ଆକାର(dop));

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->ଗଣନା_2, dop.ଗଣନା_2);

    ଫେରନ୍ତୁ status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
send_disk_op(ସଂରଚନା disk_op_s *op)
{
    ASSERT16();
    ଯଦି (! CONFIG_DRIVES)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        ଫେରନ୍ତୁ ଗଦା_hop(__send_disk_op, op, GET_SEG(SS));
    ଫେରନ୍ତୁ process_op(op);
}

// Perform read/write/verify using old-style chs accesses
ସ୍ଥିର ଶୂନ୍ୟ noinline
basic_access(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl, u16 command)
{
    ସଂରଚନା disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 ଗଣନା_2 = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    ଯଦି (ଗଣନା_2 > 128 || ଗଣନା_2 == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }
    dop.ଗଣନା_2 = ଗଣନା_2;

    ସଂରଚନା chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    ଯଦି (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = send_disk_op(&dop);

    regs->al = dop.ଗଣନା_2;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
ସ୍ଥିର ଶୂନ୍ୟ noinline
extended_access(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl, u16 command)
{
    ସଂରଚନା disk_op_s dop;
    ସଂରଚନା int13ext_s *param_far = (ସଂରଚନା int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    ଯଦି (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.ଗଣନା_2 = GET_FARVAR(regs->ds, param_far->ଗଣନା_2);
    ଯଦି (! dop.ଗଣନା_2) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->ଗଣନା_2, dop.ଗଣନା_2);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
ସ୍ଥିର ଶୂନ୍ୟ
disk_1300(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ସଂରଚନା disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.ଗଣନା_2 = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
ସ୍ଥିର ଶୂନ୍ୟ
disk_1301(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    u8 v;
    ଯଦି (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    ଅନ୍ୟଥା
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
ସ୍ଥିର ଶୂନ୍ୟ
disk_1302(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
ସ୍ଥିର ଶୂନ୍ୟ
disk_1303(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
ସ୍ଥିର ଶୂନ୍ୟ
disk_1304(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
ସ୍ଥିର ଶୂନ୍ୟ noinline
disk_1305(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    debug_stub(regs);

    ସଂରଚନା chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 ଗଣନା_2 = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    ଯଦି (cylinder >= nlc || head >= nlh || ଗଣନା_2 == 0 || ଗଣନା_2 > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }

    ସଂରଚନା disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.ଗଣନା_2 = ଗଣନା_2;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
ସ୍ଥିର ଶୂନ୍ୟ noinline
disk_1308(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    // Get logical geometry from table
    ସଂରଚନା chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 ଗଣନା_2;
    ଯଦି (regs->dl < EXTSTART_HD) {
        // Floppy
        ଗଣନା_2 = GET_GLOBAL(Floppy_ଗଣନା);

        ଯଦି (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_ତଳ(CDEmu.media) * 2;
        ଅନ୍ୟଥା
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } ଅନ୍ୟଥା ଯଦି (regs->dl < EXTSTART_CD) {
        // Hard drive
        ଗଣନା_2 = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } ଅନ୍ୟଥା {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }

    ଯଦି (CONFIG_CDROM_EMU && GET_ତଳ(CDEmu.media)) {
        u8 emudrive = GET_ତଳ(CDEmu.emulated_drive);
        ଯଦି (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            ଗଣନା_2++;
        ଯଦି (regs->dl < EXTSTART_HD && ଗଣନା_2 > 2)
            // Max of two floppy drives.
            ଗଣନା_2 = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = ଗଣନା_2;
}

// initialize drive parameters
ସ୍ଥିର ଶୂନ୍ୟ
disk_1309(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
ସ୍ଥିର ଶୂନ୍ୟ
disk_130c(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
ସ୍ଥିର ଶୂନ୍ୟ
disk_130d(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
ସ୍ଥିର ଶୂନ୍ୟ
disk_1310(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    // should look at 40:8E also???

    ସଂରଚନା disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.ଗଣନା_2 = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
ସ୍ଥିର ଶୂନ୍ୟ
disk_1311(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
ସ୍ଥିର ଶୂନ୍ୟ
disk_1314(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
ସ୍ଥିର ଶୂନ୍ୟ noinline
disk_1315(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    ଯଦି (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        ଫେରନ୍ତୁ;
    }
    // Hard drive

    // Get logical geometry from table
    ସଂରଚନା chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_1316(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ଯଦି (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
ସ୍ଥିର ଶୂନ୍ୟ
disk_1341(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
ସ୍ଥିର ଶୂନ୍ୟ
disk_1342(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
ସ୍ଥିର ଶୂନ୍ୟ
disk_1343(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
ସ୍ଥିର ଶୂନ୍ୟ
disk_1344(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
ସ୍ଥିର ଶୂନ୍ୟ
disk_134500(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ତଳ(CDRom_locks[cdid]);
    ଯଦି (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        ଫେରନ୍ତୁ;
    }
    SET_ତଳ(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
ସ୍ଥିର ଶୂନ୍ୟ
disk_134501(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ତଳ(CDRom_locks[cdid]);
    ଯଦି (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        ଫେରନ୍ତୁ;
    }
    locks--;
    SET_ତଳ(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
ସ୍ଥିର ଶୂନ୍ୟ
disk_134502(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ତଳ(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_1345XX(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
ସ୍ଥିର ଶୂନ୍ୟ
disk_1345(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ଯଦି (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x00: disk_134500(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: disk_134501(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: disk_134502(regs, drive_fl); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   disk_1345XX(regs, drive_fl); ଭାଙ୍ଗ;
    }
}

// IBM/MS eject media
ସ୍ଥିର ଶୂନ୍ୟ noinline
disk_1346(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ଯଦି (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_ତଳ(CDRom_locks[cdid]);
    ଯଦି (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ଫେରନ୍ତୁ;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    ଯଦି (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ଫେରନ୍ତୁ;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
ସ୍ଥିର ଶୂନ୍ୟ
disk_1347(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
ସ୍ଥିର ଶୂନ୍ୟ
disk_1348(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
ସ୍ଥିର ଶୂନ୍ୟ
disk_1349(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ଯଦି (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ଫେରନ୍ତୁ;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_134e01(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_134e03(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_134e04(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_134e06(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_134eXX(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
ସ୍ଥିର ଶୂନ୍ୟ
disk_134e(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x01: disk_134e01(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: disk_134e03(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x04: disk_134e04(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: disk_134e06(regs, drive_fl); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   disk_134eXX(regs, drive_fl); ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_13XX(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

ସ୍ଥିର ଶୂନ୍ୟ
disk_13(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00: disk_1300(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: disk_1301(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: disk_1302(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: disk_1303(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x04: disk_1304(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x05: disk_1305(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x08: disk_1308(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x09: disk_1309(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0c: disk_130c(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0d: disk_130d(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x10: disk_1310(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x11: disk_1311(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x14: disk_1314(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x15: disk_1315(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x16: disk_1316(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x41: disk_1341(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x42: disk_1342(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x43: disk_1343(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x44: disk_1344(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x45: disk_1345(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x46: disk_1346(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x47: disk_1347(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x48: disk_1348(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x49: disk_1349(regs, drive_fl); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x4e: disk_134e(regs, drive_fl); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   disk_13XX(regs, drive_fl); ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
floppy_13(ସଂରଚନା bregs *regs, ସଂରଚନା drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00:
    ସ୍ଥିତି 0x01:
    ସ୍ଥିତି 0x02:
    ସ୍ଥିତି 0x03:
    ସ୍ଥିତି 0x04:
    ସ୍ଥିତି 0x05:
    ସ୍ଥିତି 0x08:
    ସ୍ଥିତି 0x15:
    ସ୍ଥିତି 0x16:
        disk_13(regs, drive_fl);
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   disk_13XX(regs, drive_fl); ଭାଙ୍ଗ;
    }
}

// ElTorito - Terminate disk emu
ସ୍ଥିର ଶୂନ୍ୟ
cdemu_134b(ସଂରଚନା bregs *regs)
{
    memcpy_far(regs->ds, (ଶୂନ୍ୟ*)(regs->si+0), SEG_ତଳ, &CDEmu, ଆକାର(CDEmu));

    // If we have to terminate emulation
    ଯଦି (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_ତଳ(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

ସ୍ଥିର ଶୂନ୍ୟ
handle_legacy_disk(ସଂରଚନା bregs *regs, u8 extdrive)
{
    ଯଦି (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        ଫେରନ୍ତୁ;
    }

    ଯଦି (extdrive < EXTSTART_HD) {
        ସଂରଚନା drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        ଯଦି (!drive_fl)
            ଯାଅ fail;
        floppy_13(regs, drive_fl);
        ଫେରନ୍ତୁ;
    }

    ସଂରଚନା drive_s *drive_fl;
    ଯଦି (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    ଅନ୍ୟଥା
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    ଯଦି (!drive_fl)
        ଯାଅ fail;
    disk_13(regs, drive_fl);
    ଫେରନ୍ତୁ;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

ଶୂନ୍ୟ VISIBLE16
handle_40(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_13(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    ଯଦି (CONFIG_CDROM_EMU) {
        ଯଦି (regs->ah == 0x4b) {
            cdemu_134b(regs);
            ଫେରନ୍ତୁ;
        }
        ଯଦି (GET_ତଳ(CDEmu.media)) {
            u8 emudrive = GET_ତଳ(CDEmu.emulated_drive);
            ଯଦି (extdrive == emudrive) {
                // Access to an emulated drive.
                ସଂରଚନା drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                ଯଦି (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    ଫେରନ୍ତୁ;
                }
                disk_13(regs, cdemu_gf);
                ଫେରନ୍ତୁ;
            }
            ଯଦି (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
ଶୂନ୍ୟ VISIBLE16
handle_76(ଶୂନ୍ୟ)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
