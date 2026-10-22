#ifndef __ROMFILE_H
#define __ROMFILE_H

#অন্তর্ভাব "types.h" // u32

// romfile.c
কাঠামো romfile_s {
    কাঠামো romfile_s *পরবর্তী;
    অক্ষর name[128];
    u32 আকার_2;
    পূর্ণসংখ্যা (*copy)(কাঠামো romfile_s *file, শূন্য *dest, u32 maxlen);
};
শূন্য romfile_add(কাঠামো romfile_s *file);
কাঠামো romfile_s *romfile_findprefix(ধ্রুবক অক্ষর *prefix, কাঠামো romfile_s *পূর্ব);
কাঠামো romfile_s *romfile_find(ধ্রুবক অক্ষর *name);
শূন্য *romfile_loadfile(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা *psize);
u64 romfile_loadint(ধ্রুবক অক্ষর *name, u64 defval);
u32 romfile_loadbool(ধ্রুবক অক্ষর *name, u32 defval);

শূন্য const_romfile_add_int(অক্ষর *name, u32 মান);

#endif // romfile.h
