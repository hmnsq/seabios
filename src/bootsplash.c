// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "farptr.h" // FLATPTR_TO_SEG
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadfile
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "std/vbe.h" // struct vbe_info
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
ಸ್ಥಿರ ಶೂನ್ಯ
call16_int10(ರಚನೆ bregs *br)
{
    br->flags = F_IF;
    ಆರಂಭ_preempt();
    call16_int(0x10, br);
    ಮುಕ್ತಾಯ_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

ಶೂನ್ಯ
enable_vga_console(ಶೂನ್ಯ)
{
    dprintf(1, "Turning on vga text mode console\n");
    ರಚನೆ bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, ಗಾತ್ರ(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    ಬರೆ("SeaBIOS (version %s)\n", ಆವೃತ್ತಿ);
    display_uuid();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಕಂಡುಹಿಡಿ_videomode(ರಚನೆ vbe_info *vesa_info, ರಚನೆ vbe_ಬಹುಲಕ_info *ಬಹುಲಕ_info
               , ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಪೂರ್ಣಾಂಕ bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", ಅಗಲ, ಎತ್ತರ);
    u16 *videomodes = SEGOFF_ಗಮ್ಯ_FLATPTR(vesa_info->video_ಬಹುಲಕ);
    ಗಾಗಿ (;; videomodes++) {
        u16 videomode = *videomodes;
        ಆದರೆ (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , ಅಗಲ, ಎತ್ತರ);
            ಹಿಂತಿರುಗಿ -1;
        }
        ರಚನೆ bregs br;
        memset(&br, 0, ಗಾತ್ರ(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ಬಹುಲಕ_info);
        br.es = FLATPTR_ಗಮ್ಯ_SEG(ಬಹುಲಕ_info);
        call16_int10(&br);
        ಆದರೆ (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ಮುಂದುವರಿ;
        }
        ಆದರೆ (ಬಹುಲಕ_info->xres != ಅಗಲ
            || ಬಹುಲಕ_info->yres != ಎತ್ತರ)
            ಮುಂದುವರಿ;
        u8 ಆಳ = ಬಹುಲಕ_info->ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_pixel;
        ಆದರೆ (bpp_req == 0) {
            ಆದರೆ ((ಆಳ != 16 && ಆಳ != 24 && ಆಳ != 32)
                || ಬಹುಲಕ_info->green_ಗಾತ್ರ == 5)
                ಮುಂದುವರಿ;
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಆದರೆ (ಆಳ != bpp_req)
                ಮುಂದುವರಿ;
        }
        ಹಿಂತಿರುಗಿ videomode;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Bootsplash_ಸಕ್ರಿಯ;

ಶೂನ್ಯ
enable_bootsplash(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_BOOTSPLASH)
        ಹಿಂತಿರುಗಿ;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    ಪೂರ್ಣಾಂಕ filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    ಆದರೆ (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        ಆದರೆ (!filedata)
            ಹಿಂತಿರುಗಿ;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ಖಾಲಿ; /* data buff used to be flushed to the video buf */
    ರಚನೆ jpeg_decdata *jpeg = ಖಾಲಿ;
    ರಚನೆ bmp_decdata *bmp = ಖಾಲಿ;
    ರಚನೆ vbe_info *vesa_info = malloc_tmplow(ಗಾತ್ರ(*vesa_info));
    ರಚನೆ vbe_ಬಹುಲಕ_info *ಬಹುಲಕ_info = malloc_tmplow(ಗಾತ್ರ(*ಬಹುಲಕ_info));
    ಆದರೆ (!vesa_info || !ಬಹುಲಕ_info) {
        warn_noalloc();
        ಹೋಗು ಮುಗಿದ;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, ಗಾತ್ರ(ರಚನೆ vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(vesa_info);
    br.es = FLATPTR_ಗಮ್ಯ_SEG(vesa_info);
    call16_int10(&br);
    ಆದರೆ (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        ಹೋಗು ಮುಗಿದ;
    }

    /* Print some debugging information about our card. */
    ಅಕ್ಷರ *vendor = SEGOFF_ಗಮ್ಯ_FLATPTR(vesa_info->oem_vendor_string);
    ಅಕ್ಷರ *ಗುಣಲಬ್ಧ = SEGOFF_ಗಮ್ಯ_FLATPTR(vesa_info->oem_ಗುಣಲಬ್ಧ_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->ಆವೃತ್ತಿ_2>>8, vesa_info->ಆವೃತ್ತಿ_2&0xff,
            vendor, ಗುಣಲಬ್ಧ);

    ಪೂರ್ಣಾಂಕ ret, ಅಗಲ, ಎತ್ತರ;
    ಪೂರ್ಣಾಂಕ bpp_require = 0;
    ಆದರೆ (type == 0) {
        jpeg = jpeg_alloc();
        ಆದರೆ (!jpeg) {
            warn_noalloc();
            ಹೋಗು ಮುಗಿದ;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        ಆದರೆ (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            ಹೋಗು ಮುಗಿದ;
        }
        jpeg_get_ಗಾತ್ರ(jpeg, &ಅಗಲ, &ಎತ್ತರ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        bmp = bmp_alloc();
        ಆದರೆ (!bmp) {
            warn_noalloc();
            ಹೋಗು ಮುಗಿದ;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        ಆದರೆ (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            ಹೋಗು ಮುಗಿದ;
        }
        bmp_get_info(bmp, &ಅಗಲ, &ಎತ್ತರ, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    ಪೂರ್ಣಾಂಕ videomode = ಕಂಡುಹಿಡಿ_videomode(vesa_info, ಬಹುಲಕ_info, ಅಗಲ, ಎತ್ತರ,
                                       bpp_require);
    ಆದರೆ (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    ಅಗಲ, ಎತ್ತರ, bpp_require);
        ಹೋಗು ಮುಗಿದ;
    }
    ಶೂನ್ಯ *framebuffer = (ಶೂನ್ಯ *)ಬಹುಲಕ_info->phys_ತಳ;
    ಪೂರ್ಣಾಂಕ ಆಳ = ಬಹುಲಕ_info->ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", ಬಹುಲಕ_info->ಬೈಟುಗಳು_ಪ್ರತಿ_scanline);
    dprintf(3, "bits per pixel: %d\n", ಆಳ);

    // Allocate space for image and decompress it.
    ಪೂರ್ಣಾಂಕ imagesize = ಎತ್ತರ * ಬಹುಲಕ_info->ಬೈಟುಗಳು_ಪ್ರತಿ_scanline;
    picture = malloc_tmphigh(imagesize);
    ಆದರೆ (!picture) {
        warn_noalloc();
        ಹೋಗು ಮುಗಿದ;
    }

    ಆದರೆ (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, ಅಗಲ, ಎತ್ತರ, ಆಳ,
                            ಬಹುಲಕ_info->ಬೈಟುಗಳು_ಪ್ರತಿ_scanline);
        ಆದರೆ (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            ಹೋಗು ಮುಗಿದ;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, ಅಗಲ, ಎತ್ತರ, ಆಳ,
                           ಬಹುಲಕ_info->ಬೈಟುಗಳು_ಪ್ರತಿ_scanline);
        ಆದರೆ (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            ಹೋಗು ಮುಗಿದ;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, ಗಾತ್ರ(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_ಬಹುಲಕ_ರೇಖೀಯ_ಚೌಕಟ್ಟು_ಬಫರ್;
    call16_int10(&br);
    ಆದರೆ (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        ಹೋಗು ಮುಗಿದ;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    Bootsplash_ಸಕ್ರಿಯ = 1;

ಮುಗಿದ:
    ಬಿಡುಗಡೆ(filedata);
    ಬಿಡುಗಡೆ(picture);
    ಬಿಡುಗಡೆ(vesa_info);
    ಬಿಡುಗಡೆ(ಬಹುಲಕ_info);
    ಬಿಡುಗಡೆ(jpeg);
    ಬಿಡುಗಡೆ(bmp);
    ಹಿಂತಿರುಗಿ;
}

ಶೂನ್ಯ
disable_bootsplash(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_BOOTSPLASH || !Bootsplash_ಸಕ್ರಿಯ)
        ಹಿಂತಿರುಗಿ;
    Bootsplash_ಸಕ್ರಿಯ = 0;
    enable_vga_console();
}
