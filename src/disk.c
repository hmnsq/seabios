// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // SET_BDA
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "hw/ata.h" // ATA_CB_DC
#અન્તર્ભાવ "hw/pic.h" // pic_eoi2
#અન્તર્ભાવ "output.h" // debug_enter
#અન્તર્ભાવ "stacks.h" // call16_int
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

સ્થિર શૂન્ય
__disk_ret(સંરચના bregs *regs, u32 linecode, અચલ અક્ષર *fname)
{
    u8 code = linecode;
    જો (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    નહીંતર
        SET_BDA(disk_last_status, code);
    જો (code)
        __set_code_invalid(regs, linecode, fname);
    નહીંતર
        set_code_success(regs);
}

સ્થિર શૂન્ય
__disk_ret_unimplemented(સંરચના bregs *regs, u32 linecode, અચલ અક્ષર *fname)
{
    u8 code = linecode;
    જો (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    નહીંતર
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

સ્થિર શૂન્ય
__disk_stub(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
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
સ્થિર સંરચના chs_s
getLCHS(સંરચના drive_s *drive_fl)
{
    સંરચના chs_s res = { };
    જો (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_નીચલું(CDEmu.chs.sptcyl);
        res.cylinder = GET_નીચલું(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_નીચલું(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        પાછા res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    પાછા res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
સ્થિર પૂર્ણાંક
__send_disk_op(સંરચના disk_op_s *op_far, u16 op_seg)
{
    સંરચના disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, કદ(dop));

    પૂર્ણાંક status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->ગણતરી, dop.ગણતરી);

    પાછા status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
સ્થિર પૂર્ણાંક
send_disk_op(સંરચના disk_op_s *op)
{
    ASSERT16();
    જો (! CONFIG_DRIVES)
        પાછા -1;
    જો (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        પાછા ઢગલો_hop(__send_disk_op, op, GET_SEG(SS));
    પાછા process_op(op);
}

// Perform read/write/verify using old-style chs accesses
સ્થિર શૂન્ય noinline
basic_access(સંરચના bregs *regs, સંરચના drive_s *drive_fl, u16 command)
{
    સંરચના disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 ગણતરી = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    જો (ગણતરી > 128 || ગણતરી == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }
    dop.ગણતરી = ગણતરી;

    સંરચના chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    જો (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    પૂર્ણાંક status = send_disk_op(&dop);

    regs->al = dop.ગણતરી;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
સ્થિર શૂન્ય noinline
extended_access(સંરચના bregs *regs, સંરચના drive_s *drive_fl, u16 command)
{
    સંરચના disk_op_s dop;
    સંરચના int13ext_s *param_far = (સંરચના int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    જો (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.ગણતરી = GET_FARVAR(regs->ds, param_far->ગણતરી);
    જો (! dop.ગણતરી) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        પાછા;
    }

    પૂર્ણાંક status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->ગણતરી, dop.ગણતરી);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
સ્થિર શૂન્ય
disk_1300(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    સંરચના disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.ગણતરી = 0;
    પૂર્ણાંક status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
સ્થિર શૂન્ય
disk_1301(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    u8 v;
    જો (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    નહીંતર
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
સ્થિર શૂન્ય
disk_1302(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
સ્થિર શૂન્ય
disk_1303(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
સ્થિર શૂન્ય
disk_1304(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
સ્થિર શૂન્ય noinline
disk_1305(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    debug_stub(regs);

    સંરચના chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 ગણતરી = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    જો (cylinder >= nlc || head >= nlh || ગણતરી == 0 || ગણતરી > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }

    સંરચના disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.ગણતરી = ગણતરી;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    પૂર્ણાંક status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
સ્થિર શૂન્ય noinline
disk_1308(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    // Get logical geometry from table
    સંરચના chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 ગણતરી;
    જો (regs->dl < EXTSTART_HD) {
        // Floppy
        ગણતરી = GET_GLOBAL(Floppy_ગણતરી);

        જો (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_નીચલું(CDEmu.media) * 2;
        નહીંતર
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } નહીંતર જો (regs->dl < EXTSTART_CD) {
        // Hard drive
        ગણતરી = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } નહીંતર {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }

    જો (CONFIG_CDROM_EMU && GET_નીચલું(CDEmu.media)) {
        u8 emudrive = GET_નીચલું(CDEmu.emulated_drive);
        જો (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            ગણતરી++;
        જો (regs->dl < EXTSTART_HD && ગણતરી > 2)
            // Max of two floppy drives.
            ગણતરી = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = ગણતરી;
}

// initialize drive parameters
સ્થિર શૂન્ય
disk_1309(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
સ્થિર શૂન્ય
disk_130c(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
સ્થિર શૂન્ય
disk_130d(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
સ્થિર શૂન્ય
disk_1310(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    // should look at 40:8E also???

    સંરચના disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.ગણતરી = 0;
    પૂર્ણાંક status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
સ્થિર શૂન્ય
disk_1311(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
સ્થિર શૂન્ય
disk_1314(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
સ્થિર શૂન્ય noinline
disk_1315(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    જો (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        પાછા;
    }
    // Hard drive

    // Get logical geometry from table
    સંરચના chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

સ્થિર શૂન્ય
disk_1316(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    જો (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
સ્થિર શૂન્ય
disk_1341(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
સ્થિર શૂન્ય
disk_1342(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
સ્થિર શૂન્ય
disk_1343(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
સ્થિર શૂન્ય
disk_1344(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
સ્થિર શૂન્ય
disk_134500(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    પૂર્ણાંક cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_નીચલું(CDRom_locks[cdid]);
    જો (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        પાછા;
    }
    SET_નીચલું(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
સ્થિર શૂન્ય
disk_134501(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    પૂર્ણાંક cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_નીચલું(CDRom_locks[cdid]);
    જો (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        પાછા;
    }
    locks--;
    SET_નીચલું(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
સ્થિર શૂન્ય
disk_134502(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    પૂર્ણાંક cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_નીચલું(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

સ્થિર શૂન્ય
disk_1345XX(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
સ્થિર શૂન્ય
disk_1345(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    જો (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        પાછા;
    }

    પસંદગી (regs->al) {
    સ્થિતિ 0x00: disk_134500(regs, drive_fl); તોડો;
    સ્થિતિ 0x01: disk_134501(regs, drive_fl); તોડો;
    સ્થિતિ 0x02: disk_134502(regs, drive_fl); તોડો;
    મૂળભૂત:   disk_1345XX(regs, drive_fl); તોડો;
    }
}

// IBM/MS eject media
સ્થિર શૂન્ય noinline
disk_1346(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    જો (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        પાછા;
    }

    પૂર્ણાંક cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_નીચલું(CDRom_locks[cdid]);
    જો (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        પાછા;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    જો (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        પાછા;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
સ્થિર શૂન્ય
disk_1347(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
સ્થિર શૂન્ય
disk_1348(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    પૂર્ણાંક ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
સ્થિર શૂન્ય
disk_1349(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    જો (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        પાછા;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

સ્થિર શૂન્ય
disk_134e01(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

સ્થિર શૂન્ય
disk_134e03(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

સ્થિર શૂન્ય
disk_134e04(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

સ્થિર શૂન્ય
disk_134e06(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

સ્થિર શૂન્ય
disk_134eXX(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
સ્થિર શૂન્ય
disk_134e(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    પસંદગી (regs->al) {
    સ્થિતિ 0x01: disk_134e01(regs, drive_fl); તોડો;
    સ્થિતિ 0x03: disk_134e03(regs, drive_fl); તોડો;
    સ્થિતિ 0x04: disk_134e04(regs, drive_fl); તોડો;
    સ્થિતિ 0x06: disk_134e06(regs, drive_fl); તોડો;
    મૂળભૂત:   disk_134eXX(regs, drive_fl); તોડો;
    }
}

સ્થિર શૂન્ય
disk_13XX(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

સ્થિર શૂન્ય
disk_13(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    પસંદગી (regs->ah) {
    સ્થિતિ 0x00: disk_1300(regs, drive_fl); તોડો;
    સ્થિતિ 0x01: disk_1301(regs, drive_fl); તોડો;
    સ્થિતિ 0x02: disk_1302(regs, drive_fl); તોડો;
    સ્થિતિ 0x03: disk_1303(regs, drive_fl); તોડો;
    સ્થિતિ 0x04: disk_1304(regs, drive_fl); તોડો;
    સ્થિતિ 0x05: disk_1305(regs, drive_fl); તોડો;
    સ્થિતિ 0x08: disk_1308(regs, drive_fl); તોડો;
    સ્થિતિ 0x09: disk_1309(regs, drive_fl); તોડો;
    સ્થિતિ 0x0c: disk_130c(regs, drive_fl); તોડો;
    સ્થિતિ 0x0d: disk_130d(regs, drive_fl); તોડો;
    સ્થિતિ 0x10: disk_1310(regs, drive_fl); તોડો;
    સ્થિતિ 0x11: disk_1311(regs, drive_fl); તોડો;
    સ્થિતિ 0x14: disk_1314(regs, drive_fl); તોડો;
    સ્થિતિ 0x15: disk_1315(regs, drive_fl); તોડો;
    સ્થિતિ 0x16: disk_1316(regs, drive_fl); તોડો;
    સ્થિતિ 0x41: disk_1341(regs, drive_fl); તોડો;
    સ્થિતિ 0x42: disk_1342(regs, drive_fl); તોડો;
    સ્થિતિ 0x43: disk_1343(regs, drive_fl); તોડો;
    સ્થિતિ 0x44: disk_1344(regs, drive_fl); તોડો;
    સ્થિતિ 0x45: disk_1345(regs, drive_fl); તોડો;
    સ્થિતિ 0x46: disk_1346(regs, drive_fl); તોડો;
    સ્થિતિ 0x47: disk_1347(regs, drive_fl); તોડો;
    સ્થિતિ 0x48: disk_1348(regs, drive_fl); તોડો;
    સ્થિતિ 0x49: disk_1349(regs, drive_fl); તોડો;
    સ્થિતિ 0x4e: disk_134e(regs, drive_fl); તોડો;
    મૂળભૂત:   disk_13XX(regs, drive_fl); તોડો;
    }
}

સ્થિર શૂન્ય
floppy_13(સંરચના bregs *regs, સંરચના drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    પસંદગી (regs->ah) {
    સ્થિતિ 0x00:
    સ્થિતિ 0x01:
    સ્થિતિ 0x02:
    સ્થિતિ 0x03:
    સ્થિતિ 0x04:
    સ્થિતિ 0x05:
    સ્થિતિ 0x08:
    સ્થિતિ 0x15:
    સ્થિતિ 0x16:
        disk_13(regs, drive_fl);
        તોડો;
    મૂળભૂત:   disk_13XX(regs, drive_fl); તોડો;
    }
}

// ElTorito - Terminate disk emu
સ્થિર શૂન્ય
cdemu_134b(સંરચના bregs *regs)
{
    memcpy_far(regs->ds, (શૂન્ય*)(regs->si+0), SEG_નીચલું, &CDEmu, કદ(CDEmu));

    // If we have to terminate emulation
    જો (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_નીચલું(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

સ્થિર શૂન્ય
handle_legacy_disk(સંરચના bregs *regs, u8 extdrive)
{
    જો (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        પાછા;
    }

    જો (extdrive < EXTSTART_HD) {
        સંરચના drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        જો (!drive_fl)
            જાઓ fail;
        floppy_13(regs, drive_fl);
        પાછા;
    }

    સંરચના drive_s *drive_fl;
    જો (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    નહીંતર
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    જો (!drive_fl)
        જાઓ fail;
    disk_13(regs, drive_fl);
    પાછા;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

શૂન્ય VISIBLE16
handle_40(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
શૂન્ય VISIBLE16
handle_13(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    જો (CONFIG_CDROM_EMU) {
        જો (regs->ah == 0x4b) {
            cdemu_134b(regs);
            પાછા;
        }
        જો (GET_નીચલું(CDEmu.media)) {
            u8 emudrive = GET_નીચલું(CDEmu.emulated_drive);
            જો (extdrive == emudrive) {
                // Access to an emulated drive.
                સંરચના drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                જો (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    પાછા;
                }
                disk_13(regs, cdemu_gf);
                પાછા;
            }
            જો (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
શૂન્ય VISIBLE16
handle_76(શૂન્ય)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
