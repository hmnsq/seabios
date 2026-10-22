#నిర్వచితం_కాకపోతే __MEMMAP_H
#నిర్వచించు __MEMMAP_H

#అన్తర్భావ "types.h" // u32

// A typical OS page size
#నిర్వచించు పుట_కొలత 4096
#నిర్వచించు పుట_జరుపు 12

స్థిర అంతఃవరుస u32 virt_గమ్యం_phys(శూన్య *v) {
    తిరిగి (u32)v;
}
స్థిర అంతఃవరుస శూన్య *memremap(u32 addr, u32 len) {
    తిరిగి (శూన్య*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#నిర్వచించు చిహ్నం(SYM) ({ బాహ్య అక్షరం SYM[]; (u32)SYM; })
#నిర్వచించు VSYMBOL(SYM) ((శూన్య*)చిహ్నం(SYM))

#అయితే_ముగింపు // memmap.h
