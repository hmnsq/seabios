// Initialize the VGA console and possibly show a boot splash image.
//
// Copyright (C) 2009-2010  coresystems GmbH
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "farptr.h" // FLATPTR_TO_SEG
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadfile
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "std/vbe.h" // struct vbe_info
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // enable_bootsplash


/****************************************************************
 * Helper functions
 ****************************************************************/

// Call int10 vga handler.
स्थिर शून्य
call16_int10(संरचना bregs *br)
{
    br->flags = F_IF;
    start_preempt();
    call16_int(0x10, br);
    finish_preempt();
}


/****************************************************************
 * VGA text / graphics console
 ****************************************************************/

शून्य
enable_vga_console(शून्य)
{
    dprintf(1, "Turning on vga text mode console\n");
    संरचना bregs br;

    /* Enable VGA text mode */
    स_रखो(&br, 0, आकार(br));
    br.ax = 0x0003;
    call16_int10(&br);

    // Write to screen.
    म_लिखो("SeaBIOS (version %s)\n", VERSION);
    display_uuid();
}

स्थिर पूर्णांक
find_videomode(संरचना vbe_info *vesa_info, संरचना vbe_mode_info *mode_info
               , पूर्णांक width, पूर्णांक height, पूर्णांक bpp_req)
{
    dprintf(3, "Finding vesa mode with dimensions %d/%d\n", width, height);
    u16 *videomodes = SEGOFF_TO_FLATPTR(vesa_info->video_mode);
    साठी (;; videomodes++) {
        u16 videomode = *videomodes;
        जर (videomode == 0xffff) {
            dprintf(1, "Unable to find vesa video mode dimensions %d/%d\n"
                    , width, height);
            परत -1;
        }
        संरचना bregs br;
        स_रखो(&br, 0, आकार(br));
        br.ax = 0x4f01;
        br.cx = videomode;
        br.di = FLATPTR_TO_OFFSET(mode_info);
        br.es = FLATPTR_TO_SEG(mode_info);
        call16_int10(&br);
        जर (br.ax != 0x4f) {
            dprintf(1, "get_mode failed.\n");
            सुरूठेवा;
        }
        जर (mode_info->xres != width
            || mode_info->yres != height)
            सुरूठेवा;
        u8 depth = mode_info->bits_per_pixel;
        जर (bpp_req == 0) {
            जर ((depth != 16 && depth != 24 && depth != 32)
                || mode_info->green_size == 5)
                सुरूठेवा;
        } अन्यथा {
            जर (depth != bpp_req)
                सुरूठेवा;
        }
        परत videomode;
    }
}

स्थिर पूर्णांक BootsplashActive;

शून्य
enable_bootsplash(शून्य)
{
    जर (!CONFIG_BOOTSPLASH)
        परत;
    /* splash picture can be bmp or jpeg file */
    dprintf(3, "Checking for bootsplash\n");
    u8 type = 0; /* 0 means jpg, 1 means bmp, default is 0=jpg */
    पूर्णांक filesize;
    u8 *filedata = romfile_loadfile("bootsplash.jpg", &filesize);
    जर (!filedata) {
        filedata = romfile_loadfile("bootsplash.bmp", &filesize);
        जर (!filedata)
            परत;
        type = 1;
    }
    dprintf(3, "start showing bootsplash\n");

    u8 *picture = रिक्त; /* data buff used to be flushed to the video buf */
    संरचना jpeg_decdata *jpeg = रिक्त;
    संरचना bmp_decdata *bmp = रिक्त;
    संरचना vbe_info *vesa_info = malloc_tmplow(आकार(*vesa_info));
    संरचना vbe_mode_info *mode_info = malloc_tmplow(आकार(*mode_info));
    जर (!vesa_info || !mode_info) {
        warn_noalloc();
        जा done;
    }

    /* Check whether we have a VESA 2.0 compliant BIOS */
    स_रखो(vesa_info, 0, आकार(संरचना vbe_info));
    vesa_info->signature = VBE2_SIGNATURE;
    संरचना bregs br;
    स_रखो(&br, 0, आकार(br));
    br.ax = 0x4f00;
    br.di = FLATPTR_TO_OFFSET(vesa_info);
    br.es = FLATPTR_TO_SEG(vesa_info);
    call16_int10(&br);
    जर (vesa_info->signature != VESA_SIGNATURE) {
        dprintf(1,"No VBE2 found.\n");
        जा done;
    }

    /* Print some debugging information about our card. */
    अक्षर *vendor = SEGOFF_TO_FLATPTR(vesa_info->oem_vendor_string);
    अक्षर *product = SEGOFF_TO_FLATPTR(vesa_info->oem_product_string);
    dprintf(3, "VESA %d.%d\nVENDOR: %s\nPRODUCT: %s\n",
            vesa_info->version>>8, vesa_info->version&0xff,
            vendor, product);

    पूर्णांक ret, width, height;
    पूर्णांक bpp_require = 0;
    जर (type == 0) {
        jpeg = jpeg_alloc();
        जर (!jpeg) {
            warn_noalloc();
            जा done;
        }
        /* Parse jpeg and get image size. */
        dprintf(5, "Decoding bootsplash.jpg\n");
        ret = jpeg_decode(jpeg, filedata);
        जर (ret) {
            dprintf(1, "jpeg_decode failed with return code %d...\n", ret);
            जा done;
        }
        jpeg_get_size(jpeg, &width, &height);
    } अन्यथा {
        bmp = bmp_alloc();
        जर (!bmp) {
            warn_noalloc();
            जा done;
        }
        /* Parse bmp and get image size. */
        dprintf(5, "Decoding bootsplash.bmp\n");
        ret = bmp_decode(bmp, filedata, filesize);
        जर (ret) {
            dprintf(1, "bmp_decode failed with return code %d...\n", ret);
            जा done;
        }
        bmp_get_info(bmp, &width, &height, &bpp_require);
    }

    // jpeg would use 16 or 24 bpp video mode, BMP uses 16/24/32 bpp mode.

    // Try to find a graphics mode with the corresponding dimensions.
    पूर्णांक videomode = find_videomode(vesa_info, mode_info, width, height,
                                       bpp_require);
    जर (videomode < 0) {
        dprintf(1, "failed to find a videomode with %dx%d %dbpp (0=any).\n",
                    width, height, bpp_require);
        जा done;
    }
    शून्य *framebuffer = (शून्य *)mode_info->phys_base;
    पूर्णांक depth = mode_info->bits_per_pixel;
    dprintf(3, "mode: %04x\n", videomode);
    dprintf(3, "framebuffer: %p\n", framebuffer);
    dprintf(3, "bytes per scanline: %d\n", mode_info->bytes_per_scanline);
    dprintf(3, "bits per pixel: %d\n", depth);

    // Allocate space for image and decompress it.
    पूर्णांक imagesize = height * mode_info->bytes_per_scanline;
    picture = malloc_tmphigh(imagesize);
    जर (!picture) {
        warn_noalloc();
        जा done;
    }

    जर (type == 0) {
        dprintf(5, "Decompressing bootsplash.jpg\n");
        ret = jpeg_show(jpeg, picture, width, height, depth,
                            mode_info->bytes_per_scanline);
        जर (ret) {
            dprintf(1, "jpeg_show failed with return code %d...\n", ret);
            जा done;
        }
    } अन्यथा {
        dprintf(5, "Decompressing bootsplash.bmp\n");
        ret = bmp_show(bmp, picture, width, height, depth,
                           mode_info->bytes_per_scanline);
        जर (ret) {
            dprintf(1, "bmp_show failed with return code %d...\n", ret);
            जा done;
        }
    }

    /* Switch to graphics mode */
    dprintf(5, "Switching to graphics mode\n");
    स_रखो(&br, 0, आकार(br));
    br.ax = 0x4f02;
    br.bx = videomode | VBE_MODE_LINEAR_FRAME_BUFFER;
    call16_int10(&br);
    जर (br.ax != 0x4f) {
        dprintf(1, "set_mode failed.\n");
        जा done;
    }

    /* Show the picture */
    dprintf(5, "Showing bootsplash picture\n");
    iomemcpy(framebuffer, picture, imagesize);
    dprintf(5, "Bootsplash copy complete\n");
    BootsplashActive = 1;

done:
    मुक्त(filedata);
    मुक्त(picture);
    मुक्त(vesa_info);
    मुक्त(mode_info);
    मुक्त(jpeg);
    मुक्त(bmp);
    परत;
}

शून्य
disable_bootsplash(शून्य)
{
    जर (!CONFIG_BOOTSPLASH || !BootsplashActive)
        परत;
    BootsplashActive = 0;
    enable_vga_console();
}
