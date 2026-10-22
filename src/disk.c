// 16bit code to access hard drives.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // SET_BDA
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "hw/ata.h" // ATA_CB_DC
#include "hw/pic.h" // pic_eoi2
#include "output.h" // debug_enter
#include "stacks.h" // call16_int
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // CDRom_locks


/****************************************************************
 * Return status functions
 ****************************************************************/

static void
__disk_ret(sa_nracanaa bregs *regs, u32 linecode, const var_na *fname)
{
    u8 code = linecode;
    གལ་ཏེ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    དེ་མིན
        SET_BDA(disk_last_status, code);
    གལ་ཏེ (code)
        __set_code_invalid(regs, linecode, fname);
    དེ་མིན
        set_code_success(regs);
}

static void
__disk_ret_unimplemented(sa_nracanaa bregs *regs, u32 linecode, const var_na *fname)
{
    u8 code = linecode;
    གལ་ཏེ (regs->dl < EXTSTART_HD)
        SET_BDA(floppy_last_status, code);
    དེ་མིན
        SET_BDA(disk_last_status, code);
    __set_code_unimplemented(regs, linecode, fname);
}

static void
__disk_stub(sa_nracanaa bregs *regs, ཧྲིལ lineno, const var_na *fname)
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
static sa_nracanaa chs_s
getLCHS(sa_nracanaa drive_s *drive_fl)
{
    sa_nracanaa chs_s res = { };
    གལ་ཏེ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf)) {
        // Emulated drive - get info from CDEmu.  (It's not possible to
        // populate the geometry directly in the driveid because the
        // geometry is only known after the bios segment is made
        // read-only).
        u8 sptcyl = GET_LOW(CDEmu.chs.sptcyl);
        res.cylinder = GET_LOW(CDEmu.chs.cyllow) + ((sptcyl << 2) & 0x300) + 1;
        res.head = GET_LOW(CDEmu.chs.heads) + 1;
        res.sector = sptcyl & 0x3f;
        ཕྱིར res;
    }
    res.cylinder = GET_FLATPTR(drive_fl->lchs.cylinder);
    res.head = GET_FLATPTR(drive_fl->lchs.head);
    res.sector = GET_FLATPTR(drive_fl->lchs.sector);
    ཕྱིར res;
}

// Execute a "disk_op_s" request after jumping to the extra stack.
static ཧྲིལ
__send_disk_op(sa_nracanaa disk_op_s *op_far, u16 op_seg)
{
    sa_nracanaa disk_op_s dop;
    memcpy_far(GET_SEG(SS), &dop, op_seg, op_far, parimaa_na(dop));

    ཧྲིལ status = process_op(&dop);

    // Update count with total sectors transferred.
    SET_FARVAR(op_seg, op_far->count, dop.count);

    ཕྱིར status;
}

// Execute a "disk_op_s" request (using the extra 16bit stack).
static ཧྲིལ
send_disk_op(sa_nracanaa disk_op_s *op)
{
    ASSERT16();
    གལ་ཏེ (! CONFIG_DRIVES)
        ཕྱིར -1;
    གལ་ཏེ (!CONFIG_ENTRY_EXTRASTACK)
        // Jump on to extra stack
        ཕྱིར stack_hop(__send_disk_op, op, GET_SEG(SS));
    ཕྱིར process_op(op);
}

// Perform read/write/verify using old-style chs accesses
static void noinline
basic_access(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl, u16 command)
{
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = command;

    u8 count = regs->al;
    u16 cylinder = regs->ch | ((((u16)regs->cl) << 2) & 0x300);
    u16 sector = regs->cl & 0x3f;
    u16 head = regs->dh;

    གལ་ཏེ (count > 128 || count == 0 || sector == 0) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }
    dop.count = count;

    sa_nracanaa chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // sanity check on cyl heads, sec
    གལ་ཏེ (cylinder >= nlc || head >= nlh || sector > nls) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }

    // translate lchs to lba
    dop.lba = (((((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls)
               + (u32)sector - 1);

    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);

    ཧྲིལ status = send_disk_op(&dop);

    regs->al = dop.count;

    disk_ret(regs, status);
}

// Perform read/write/verify using new-style "int13ext" accesses.
static void noinline
extended_access(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl, u16 command)
{
    sa_nracanaa disk_op_s dop;
    sa_nracanaa int13ext_s *param_far = (sa_nracanaa int13ext_s*)(regs->si+0);
    // Get lba and check.
    dop.lba = GET_FARVAR(regs->ds, param_far->lba);
    dop.command = command;
    dop.drive_fl = drive_fl;
    གལ་ཏེ (dop.lba >= GET_FLATPTR(drive_fl->sectors)) {
        warn_invalid(regs);
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }

    dop.buf_fl = SEGOFF_TO_FLATPTR(GET_FARVAR(regs->ds, param_far->data));
    dop.count = GET_FARVAR(regs->ds, param_far->count);
    གལ་ཏེ (! dop.count) {
        // Nothing to do.
        disk_ret(regs, DISK_RET_SUCCESS);
        ཕྱིར;
    }

    ཧྲིལ status = send_disk_op(&dop);

    SET_FARVAR(regs->ds, param_far->count, dop.count);

    disk_ret(regs, status);
}


/****************************************************************
 * Hard Drive functions
 ****************************************************************/

// disk controller reset
static void
disk_1300(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_RESET;
    dop.count = 0;
    ཧྲིལ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk status
static void
disk_1301(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    u8 v;
    གལ་ཏེ (regs->dl < EXTSTART_HD)
        // Floppy
        v = GET_BDA(floppy_last_status);
    དེ་མིན
        v = GET_BDA(disk_last_status);
    regs->ah = v;
    set_cf(regs, v);
    // XXX - clear disk_last_status?
}

// read disk sectors
static void
disk_1302(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_READ);
}

// write disk sectors
static void
disk_1303(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_WRITE);
}

// verify disk sectors
static void
disk_1304(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    basic_access(regs, drive_fl, CMD_VERIFY);
}

// format disk track
static void noinline
disk_1305(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    debug_stub(regs);

    sa_nracanaa chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    u8 count = regs->al;
    u8 cylinder = regs->ch;
    u8 head = regs->dh;

    གལ་ཏེ (cylinder >= nlc || head >= nlh || count == 0 || count > nls) {
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }

    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_FORMAT;
    dop.lba = (((u32)cylinder * (u32)nlh) + (u32)head) * (u32)nls;
    dop.count = count;
    dop.buf_fl = MAKE_FLATPTR(regs->es, regs->bx);
    ཧྲིལ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// read disk drive parameters
static void noinline
disk_1308(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    // Get logical geometry from table
    sa_nracanaa chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;
    nlc--;
    nlh--;
    u8 count;
    གལ་ཏེ (regs->dl < EXTSTART_HD) {
        // Floppy
        count = GET_GLOBAL(FloppyCount);

        གལ་ཏེ (CONFIG_CDROM_EMU && drive_fl == GET_GLOBAL(cdemu_drive_gf))
            regs->bx = GET_LOW(CDEmu.media) * 2;
        དེ་མིན
            regs->bx = GET_FLATPTR(drive_fl->floppy_type);

        // set es & di to point to 11 byte diskette param table in ROM
        regs->es = SEG_BIOS;
        regs->di = (u32)&diskette_param_table2;
    } དེ་མིན གལ་ཏེ (regs->dl < EXTSTART_CD) {
        // Hard drive
        count = GET_BDA(hdcount);
        nlc--;  // last sector reserved
    } དེ་མིན {
        // Not supported on CDROM
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }

    གལ་ཏེ (CONFIG_CDROM_EMU && GET_LOW(CDEmu.media)) {
        u8 emudrive = GET_LOW(CDEmu.emulated_drive);
        གལ་ཏེ (((emudrive ^ regs->dl) & 0x80) == 0)
            // Note extra drive due to emulation.
            count++;
        གལ་ཏེ (regs->dl < EXTSTART_HD && count > 2)
            // Max of two floppy drives.
            count = 2;
    }

    regs->al = 0;
    regs->ch = nlc & 0xff;
    regs->cl = ((nlc >> 2) & 0xc0) | (nls & 0x3f);
    regs->dh = nlh;

    disk_ret(regs, DISK_RET_SUCCESS);
    regs->dl = count;
}

// initialize drive parameters
static void
disk_1309(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// seek to specified cylinder
static void
disk_130c(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// alternate disk reset
static void
disk_130d(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// check drive ready
static void
disk_1310(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    // should look at 40:8E also???

    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_fl;
    dop.command = CMD_ISREADY;
    dop.count = 0;
    ཧྲིལ status = send_disk_op(&dop);
    disk_ret(regs, status);
}

// recalibrate
static void
disk_1311(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// controller internal diagnostic
static void
disk_1314(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    DISK_STUB(regs);
}

// read disk drive size
static void noinline
disk_1315(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
    གལ་ཏེ (regs->dl < EXTSTART_HD || regs->dl >= EXTSTART_CD) {
        // Floppy or cdrom
        regs->ah = 1;
        ཕྱིར;
    }
    // Hard drive

    // Get logical geometry from table
    sa_nracanaa chs_s chs = getLCHS(drive_fl);
    u16 nlc=chs.cylinder, nlh=chs.head, nls=chs.sector;

    // Compute sector count seen by int13
    u32 lba = (u32)(nlc - 1) * (u32)nlh * (u32)nls;
    regs->cx = lba >> 16;
    regs->dx = lba & 0xffff;
    regs->ah = 3; // hard disk accessible
}

static void
disk_1316(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    གལ་ཏེ (regs->dl >= EXTSTART_HD) {
        // Hard drive
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }
    disk_ret(regs, DISK_RET_ECHANGED);
}

// IBM/MS installation check
static void
disk_1341(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    regs->bx = 0xaa55;  // install check
    regs->cx = 0x0007;  // ext disk access and edd, removable supported
    disk_ret(regs, DISK_RET_SUCCESS);
    regs->ah = 0x30;    // EDD 3.0
}

// IBM/MS extended read
static void
disk_1342(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_READ);
}

// IBM/MS extended write
static void
disk_1343(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_WRITE);
}

// IBM/MS verify
static void
disk_1344(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_VERIFY);
}

// Locks for removable devices
u8 CDRom_locks[BUILD_MAX_EXTDRIVE] VARLOW;

// lock
static void
disk_134500(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    ཧྲིལ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_LOW(CDRom_locks[cdid]);
    གལ་ཏེ (locks == 0xff) {
        regs->al = 1;
        disk_ret(regs, DISK_RET_ETOOMANYLOCKS);
        ཕྱིར;
    }
    SET_LOW(CDRom_locks[cdid], locks + 1);
    regs->al = 1;
    disk_ret(regs, DISK_RET_SUCCESS);
}

// unlock
static void
disk_134501(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    ཧྲིལ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_LOW(CDRom_locks[cdid]);
    གལ་ཏེ (locks == 0x00) {
        regs->al = 0;
        disk_ret(regs, DISK_RET_ENOTLOCKED);
        ཕྱིར;
    }
    locks--;
    SET_LOW(CDRom_locks[cdid], locks);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

// status
static void
disk_134502(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    ཧྲིལ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_LOW(CDRom_locks[cdid]);
    regs->al = (locks ? 1 : 0);
    disk_ret(regs, DISK_RET_SUCCESS);
}

static void
disk_1345XX(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

// IBM/MS lock/unlock drive
static void
disk_1345(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    གལ་ཏེ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ཕྱིར;
    }

    switch (regs->al) {
    pak_sa 0x00: disk_134500(regs, drive_fl); bha_nga;
    pak_sa 0x01: disk_134501(regs, drive_fl); bha_nga;
    pak_sa 0x02: disk_134502(regs, drive_fl); bha_nga;
    default:   disk_1345XX(regs, drive_fl); bha_nga;
    }
}

// IBM/MS eject media
static void noinline
disk_1346(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    གལ་ཏེ (regs->dl < EXTSTART_CD) {
        // Volume Not Removable
        disk_ret(regs, DISK_RET_ENOTREMOVABLE);
        ཕྱིར;
    }

    ཧྲིལ cdid = regs->dl - EXTSTART_CD;
    u8 locks = GET_LOW(CDRom_locks[cdid]);
    གལ་ཏེ (locks != 0) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ཕྱིར;
    }

    // FIXME should handle 0x31 no media in device
    // FIXME should handle 0xb5 valid request failed

    // Call removable media eject
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.ah = 0x52;
    br.dl = regs->dl;
    call16_int(0x15, &br);

    གལ་ཏེ (br.ah || br.flags & F_CF) {
        disk_ret(regs, DISK_RET_ELOCKED);
        ཕྱིར;
    }
    disk_ret(regs, DISK_RET_SUCCESS);
}

// IBM/MS extended seek
static void
disk_1347(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    extended_access(regs, drive_fl, CMD_SEEK);
}

// IBM/MS get drive parameters
static void
disk_1348(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    ཧྲིལ ret = fill_edd(SEGOFF(regs->ds, regs->si), drive_fl);
    disk_ret(regs, ret);
}

// IBM/MS extended media change
static void
disk_1349(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    གལ་ཏེ (regs->dl < EXTSTART_CD) {
        // Always success for HD
        disk_ret(regs, DISK_RET_SUCCESS);
        ཕྱིར;
    }
    set_invalid(regs);
    // always send changed ??
    regs->ah = DISK_RET_ECHANGED;
}

static void
disk_134e01(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

static void
disk_134e03(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

static void
disk_134e04(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

static void
disk_134e06(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_SUCCESS);
}

static void
disk_134eXX(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret(regs, DISK_RET_EPARAM);
}

// IBM/MS set hardware configuration
static void
disk_134e(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    switch (regs->al) {
    pak_sa 0x01: disk_134e01(regs, drive_fl); bha_nga;
    pak_sa 0x03: disk_134e03(regs, drive_fl); bha_nga;
    pak_sa 0x04: disk_134e04(regs, drive_fl); bha_nga;
    pak_sa 0x06: disk_134e06(regs, drive_fl); bha_nga;
    default:   disk_134eXX(regs, drive_fl); bha_nga;
    }
}

static void
disk_13XX(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    disk_ret_unimplemented(regs, DISK_RET_EPARAM);
}

static void
disk_13(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    //debug_stub(regs);

    // clear completion flag
    SET_BDA(disk_interrupt_flag, 0);

    switch (regs->ah) {
    pak_sa 0x00: disk_1300(regs, drive_fl); bha_nga;
    pak_sa 0x01: disk_1301(regs, drive_fl); bha_nga;
    pak_sa 0x02: disk_1302(regs, drive_fl); bha_nga;
    pak_sa 0x03: disk_1303(regs, drive_fl); bha_nga;
    pak_sa 0x04: disk_1304(regs, drive_fl); bha_nga;
    pak_sa 0x05: disk_1305(regs, drive_fl); bha_nga;
    pak_sa 0x08: disk_1308(regs, drive_fl); bha_nga;
    pak_sa 0x09: disk_1309(regs, drive_fl); bha_nga;
    pak_sa 0x0c: disk_130c(regs, drive_fl); bha_nga;
    pak_sa 0x0d: disk_130d(regs, drive_fl); bha_nga;
    pak_sa 0x10: disk_1310(regs, drive_fl); bha_nga;
    pak_sa 0x11: disk_1311(regs, drive_fl); bha_nga;
    pak_sa 0x14: disk_1314(regs, drive_fl); bha_nga;
    pak_sa 0x15: disk_1315(regs, drive_fl); bha_nga;
    pak_sa 0x16: disk_1316(regs, drive_fl); bha_nga;
    pak_sa 0x41: disk_1341(regs, drive_fl); bha_nga;
    pak_sa 0x42: disk_1342(regs, drive_fl); bha_nga;
    pak_sa 0x43: disk_1343(regs, drive_fl); bha_nga;
    pak_sa 0x44: disk_1344(regs, drive_fl); bha_nga;
    pak_sa 0x45: disk_1345(regs, drive_fl); bha_nga;
    pak_sa 0x46: disk_1346(regs, drive_fl); bha_nga;
    pak_sa 0x47: disk_1347(regs, drive_fl); bha_nga;
    pak_sa 0x48: disk_1348(regs, drive_fl); bha_nga;
    pak_sa 0x49: disk_1349(regs, drive_fl); bha_nga;
    pak_sa 0x4e: disk_134e(regs, drive_fl); bha_nga;
    default:   disk_13XX(regs, drive_fl); bha_nga;
    }
}

static void
floppy_13(sa_nracanaa bregs *regs, sa_nracanaa drive_s *drive_fl)
{
    // Only limited commands are supported on floppies.
    switch (regs->ah) {
    pak_sa 0x00:
    pak_sa 0x01:
    pak_sa 0x02:
    pak_sa 0x03:
    pak_sa 0x04:
    pak_sa 0x05:
    pak_sa 0x08:
    pak_sa 0x15:
    pak_sa 0x16:
        disk_13(regs, drive_fl);
        bha_nga;
    default:   disk_13XX(regs, drive_fl); bha_nga;
    }
}

// ElTorito - Terminate disk emu
static void
cdemu_134b(sa_nracanaa bregs *regs)
{
    memcpy_far(regs->ds, (void*)(regs->si+0), SEG_LOW, &CDEmu, parimaa_na(CDEmu));

    // If we have to terminate emulation
    གལ་ཏེ (regs->al == 0x00) {
        // FIXME ElTorito Various. Should be handled accordingly to spec
        SET_LOW(CDEmu.media, 0x00); // bye bye

        // XXX - update floppy/hd count.
    }

    disk_ret(regs, DISK_RET_SUCCESS);
}


/****************************************************************
 * Entry points
 ****************************************************************/

static void
handle_legacy_disk(sa_nracanaa bregs *regs, u8 extdrive)
{
    གལ་ཏེ (! CONFIG_DRIVES) {
        // XXX - support handle_1301 anyway?
        disk_ret(regs, DISK_RET_EPARAM);
        ཕྱིར;
    }

    གལ་ཏེ (extdrive < EXTSTART_HD) {
        sa_nracanaa drive_s *drive_fl = getDrive(EXTTYPE_FLOPPY, extdrive);
        གལ་ཏེ (!drive_fl)
            goto fail;
        floppy_13(regs, drive_fl);
        ཕྱིར;
    }

    sa_nracanaa drive_s *drive_fl;
    གལ་ཏེ (extdrive >= EXTSTART_CD)
        drive_fl = getDrive(EXTTYPE_CD, extdrive - EXTSTART_CD);
    དེ་མིན
        drive_fl = getDrive(EXTTYPE_HD, extdrive - EXTSTART_HD);
    གལ་ཏེ (!drive_fl)
        goto fail;
    disk_13(regs, drive_fl);
    ཕྱིར;

fail:
    // XXX - support 1301/1308/1315 anyway?
    disk_ret(regs, DISK_RET_EPARAM);
}

void VISIBLE16
handle_40(sa_nracanaa bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_40);
    handle_legacy_disk(regs, regs->dl);
}

// INT 13h Fixed Disk Services Entry Point
void VISIBLE16
handle_13(sa_nracanaa bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_13);
    u8 extdrive = regs->dl;

    གལ་ཏེ (CONFIG_CDROM_EMU) {
        གལ་ཏེ (regs->ah == 0x4b) {
            cdemu_134b(regs);
            ཕྱིར;
        }
        གལ་ཏེ (GET_LOW(CDEmu.media)) {
            u8 emudrive = GET_LOW(CDEmu.emulated_drive);
            གལ་ཏེ (extdrive == emudrive) {
                // Access to an emulated drive.
                sa_nracanaa drive_s *cdemu_gf = GET_GLOBAL(cdemu_drive_gf);
                གལ་ཏེ (regs->ah > 0x16) {
                    // Only old-style commands supported.
                    disk_13XX(regs, cdemu_gf);
                    ཕྱིར;
                }
                disk_13(regs, cdemu_gf);
                ཕྱིར;
            }
            གལ་ཏེ (extdrive < EXTSTART_CD && ((emudrive ^ extdrive) & 0x80) == 0)
                // Adjust id to make room for emulated drive.
                extdrive--;
        }
    }
    handle_legacy_disk(regs, extdrive);
}

// record completion in BIOS task complete flag
void VISIBLE16
handle_76(void)
{
    debug_isr(DEBUG_ISR_76);
    SET_BDA(disk_interrupt_flag, 0xff);
    pic_eoi2();
}
