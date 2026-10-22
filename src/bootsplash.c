// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "farptr.h" // FLATPTR_TO_SEG
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadfile
#અન્તર્ભાવ "stacks.h" // call16_int
#અન્તર્ભાવ "std/vbe.h" // struct vbe_info
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
સ્થિર શૂન્ય
call16_int10(સંરચના bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

શૂન્ય
enable_vga_console(શૂન્ય)
{
    dprintf(1, "Turning on vga text mode console\n");
    સંરચના bregs br;

    /* Enable VGA text mode */
    memset(&br, 0, કદ(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    printf("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

સ્થિર પૂર્ણાંક
find_videomode(સંરચના vbe_info *vesa_info, સંરચના vbe_mode_info *mode_info
               , પૂર્ણાંક width, પૂર્ણાંક height, પૂર્ણાંક bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    માટે (;; videomodes++) {
        u16 videomode = *videomodes;
        જો (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            પાછા -1;
        }
        સંરચના bregs br;
        memset(&br, 0, કદ(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        જો (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            ચાલુ;
        }
        જો (mode_info->xres != width
            || mode_info->yres != height)
            ચાલુ;
        u8 depth = mode_info->bits_per_pixel;
        જો (bpp_req == 0) {
            જો ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                ચાલુ;
        } નહીંતર {
            જો (depth != bpp_req)
                ચાલુ;
        }
        પાછા videomode;
    }
}

સ્થિર પૂર્ણાંક BootsplashActive;

શૂન્ય
enable_bootsplash(શૂન્ય)
{
    જો (!CONFIG_BOOTSPLASH)
        પાછા;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    પૂર્ણાંક filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    જો (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        જો (!filedata)
            પાછા;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = ખાલી; /* data buff used to be flushed to the video buf */
    સંરચના jpeg_decdata *jpeg = ખાલી;
    સંરચના bmp_decdata *bmp = ખાલી;
    સંરચના vbe_info *vesa_info = malloc_tmplow(કદ(*vesa_info));
    સંરચના vbe_mode_info *mode_info = malloc_tmplow(કદ(*mode_info));
    જો (!vesa_info || !mode_info) {
        warn_noalloc();
        જાઓ done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    memset(vesa_info, 0, કદ(સંરચના vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    જો (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        જાઓ done;
    }

    /* Print some debugging information about our card. */
    અક્ષર *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    અક્ષર *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    પૂર્ણાંક ret, width, height;
    પૂર્ણાંક bpp_require = 0;
    જો (type == 0) {
        jpeg = jpeg_alloc();
        જો (!jpeg) {
            warn_noalloc();
            જાઓ done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        જો (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            જાઓ done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } નહીંતર {
        bmp = bmp_alloc();
        જો (!bmp) {
            warn_noalloc();
            જાઓ done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        જો (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            જાઓ done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    પૂર્ણાંક videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    જો (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        જાઓ done;
    }
    શૂન્ય *framebuffer = (શૂન્ય *)mode_info->phys_base;
    પૂર્ણાંક depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    પૂર્ણાંક imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    જો (!picture) {
        warn_noalloc();
        જાઓ done;
    }

    જો (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        જો (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            જાઓ done;
        }
    } નહીંતર {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        જો (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            જાઓ done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    memset(&br, 0, કદ(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    જો (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        જાઓ done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    મુક્ત(filedata);
    મુક્ત(picture);
    મુક્ત(vesa_info);
    મુક્ત(mode_info);
    મુક્ત(jpeg);
    મુક્ત(bmp);
    પાછા;
}

શૂન્ય
disable_bootsplash(શૂન્ય)
{
    જો (!CONFIG_BOOTSPLASH || !BootsplashActive)
        પાછા;
    BootsplashActive = 0;
    enable_vga_console();
}
