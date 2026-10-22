// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "farptr.h" // FLATPTR_TO_SEG
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadfile
#شامل "stacks.h" // call16_int
#شامل "std/vbe.h" // struct vbe_info
#شامل "string.h" // memset
#شامل "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ساکن خالی
call16_int10(ساخت bregs *br)
{
    br->flags = F_IF;
    آغاز_preempt();
    call16_int(0x10, br);
    اختتام_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

خالی
enable_vga_console(خالی)
{
    dprintf(1, "Turning on vga text mode console\n");
    ساخت bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, حجم(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    لکھو("SeaBIOS (version %s)\n", نسخہ);
    display_uuid();
}

ساکن صحیح
ڈھونڈو_videomode(ساخت vbe_info *vesa_info, ساخت vbe_منوال_info *منوال_info
               , صحیح چوڑائی, صحیح اونچائی, صحیح bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", چوڑائی, اونچائی);
    u16 *videomodes = SEGOFF_مقصد_FLATPTR(vesa_info->video_منوال);
    برائے (;; videomodes++) {
        u16 videomode = *videomodes;
        اگر (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , چوڑائی, اونچائی);
            واپس -1;
        }
        ساخت bregs br;
        memset(&br, 0, حجم(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_مقصد_انحراف(منوال_info);
        br.es = FLATPTR_مقصد_SEG(منوال_info);
        call16_int10(&br);
        اگر (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            جاری;
        }
        اگر (منوال_info->xres != چوڑائی
            || منوال_info->yres != اونچائی)
            جاری;
        u8 گہرائی = منوال_info->بٹ_فی_pixel;
        اگر (bpp_req == 0) {
            اگر ((گہرائی != 16 && گہرائی != 24 && گہرائی != 32)
                || منوال_info->green_سائز == 5)
                جاری;
        } ورنہ {
            اگر (گہرائی != bpp_req)
                جاری;
        }
        واپس videomode;
    }
}

ساکن صحیح Bootsplash_فعال;

خالی
enable_bootsplash(خالی)
{
    اگر (!CONFIG_BOOTSPLASH)
        واپس;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    صحیح filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    اگر (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        اگر (!filedata)
            واپس;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = تہی; /* data buff used to be flushed to the video buf */
    ساخت jpeg_decdata *jpeg = تہی;
    ساخت bmp_decdata *bmp = تہی;
    ساخت vbe_info *vesa_info = malloc_tmplow(حجم(*vesa_info));
    ساخت vbe_منوال_info *منوال_info = malloc_tmplow(حجم(*منوال_info));
    اگر (!vesa_info || !منوال_info) {
        warn_noalloc();
        جاؤ مکمل;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, حجم(ساخت vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_مقصد_انحراف(vesa_info);
    br.es = FLATPTR_مقصد_SEG(vesa_info);
    call16_int10(&br);
    اگر (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        جاؤ مکمل;
    }

    /* Print some debugging information about our card. */
    حرف *vendor = SEGOFF_مقصد_FLATPTR(vesa_info->oem_vendor_string);
    حرف *حاصل_ضرب = SEGOFF_مقصد_FLATPTR(vesa_info->oem_حاصل_ضرب_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->نسخہ_2>>8, vesa_info->نسخہ_2&0xff,
            vendor, حاصل_ضرب);

    صحیح ret, چوڑائی, اونچائی;
    صحیح bpp_require = 0;
    اگر (type == 0) {
        jpeg = jpeg_alloc();
        اگر (!jpeg) {
            warn_noalloc();
            جاؤ مکمل;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        اگر (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            جاؤ مکمل;
        }
        jpeg_get_سائز(jpeg, &چوڑائی, &اونچائی);
    } ورنہ {
        bmp = bmp_alloc();
        اگر (!bmp) {
            warn_noalloc();
            جاؤ مکمل;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        اگر (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            جاؤ مکمل;
        }
        bmp_get_info(bmp, &چوڑائی, &اونچائی, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    صحیح videomode = ڈھونڈو_videomode(vesa_info, منوال_info, چوڑائی, اونچائی,
                                       bpp_require);
    اگر (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    چوڑائی, اونچائی, bpp_require);
        جاؤ مکمل;
    }
    خالی *framebuffer = (خالی *)منوال_info->phys_بنیاد;
    صحیح گہرائی = منوال_info->بٹ_فی_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", منوال_info->بائٹیں_فی_scanline);
    dprintf(3, "bits per pixel: %d\n", گہرائی);

    // Allocate space for image and decompress it.
    صحیح imagesize = اونچائی * منوال_info->بائٹیں_فی_scanline;
    picture = malloc_tmphigh(imagesize);
    اگر (!picture) {
        warn_noalloc();
        جاؤ مکمل;
    }

    اگر (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, چوڑائی, اونچائی, گہرائی,
                            منوال_info->بائٹیں_فی_scanline);
        اگر (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            جاؤ مکمل;
        }
    } ورنہ {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, چوڑائی, اونچائی, گہرائی,
                           منوال_info->بائٹیں_فی_scanline);
        اگر (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            جاؤ مکمل;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, حجم(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_منوال_خطی_چوکھٹا_بفر;
    call16_int10(&br);
    اگر (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        جاؤ مکمل;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_فعال = 1;

مکمل:
    آزاد(filedata);
    آزاد(picture);
    آزاد(vesa_info);
    آزاد(منوال_info);
    آزاد(jpeg);
    آزاد(bmp);
    واپس;
}

خالی
disable_bootsplash(خالی)
{
    اگر (!CONFIG_BOOTSPLASH || !Bootsplash_فعال)
        واپس;
    Bootsplash_فعال = 0;
    enable_vga_console();
}
