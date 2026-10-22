#నిర్వచితం_కాకపోతే __SERIALIO_H
#నిర్వచించు __SERIALIO_H

#అన్తర్భావ "types.h" // u16

#నిర్వచించు పోర్టు_LPT2              0x0278
#నిర్వచించు పోర్టు_SERIAL4           0x02e8
#నిర్వచించు పోర్టు_SERIAL2           0x02f8
#నిర్వచించు పోర్టు_LPT1              0x0378
#నిర్వచించు పోర్టు_SERIAL3           0x03e8
#నిర్వచించు పోర్టు_SERIAL1           0x03f8

// Serial port offsets
#నిర్వచించు SEROFF_దత్తాంశం    0
#నిర్వచించు SEROFF_DLL     0
#నిర్వచించు SEROFF_IER     1
#నిర్వచించు SEROFF_DLH     1
#నిర్వచించు SEROFF_IIR     2
#నిర్వచించు SEROFF_LCR     3
#నిర్వచించు SEROFF_LSR     5
#నిర్వచించు SEROFF_MSR     6

శూన్య serial_debug_preinit(శూన్య);
శూన్య serial_debug_putc(అక్షరం c);
శూన్య serial_debug_flush(శూన్య);
బాహ్య u16 Debug_Output_పోర్టు;
శూన్య qemu_debug_preinit(శూన్య);
శూన్య qemu_debug_putc(అక్షరం c);

#అయితే_ముగింపు // serialio.h
