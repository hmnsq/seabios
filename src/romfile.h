#నిర్వచితం_కాకపోతే __ROMFILE_H
#నిర్వచించు __ROMFILE_H

#అన్తర్భావ "types.h" // u32

// romfile.c
నిర్మాణం romfile_s {
    నిర్మాణం romfile_s *తదుపరి;
    అక్షరం name[128];
    u32 కొలత;
    పూర్ణాంకం (*copy)(నిర్మాణం romfile_s *file, శూన్య *dest, u32 maxlen);
};
శూన్య romfile_కూడు(నిర్మాణం romfile_s *file);
నిర్మాణం romfile_s *romfile_findprefix(స్థిరరాశి అక్షరం *ఉపసర్గ, నిర్మాణం romfile_s *prev);
నిర్మాణం romfile_s *romfile_కనుగొను(స్థిరరాశి అక్షరం *name);
శూన్య *romfile_loadfile(స్థిరరాశి అక్షరం *name, పూర్ణాంకం *psize);
u64 romfile_loadint(స్థిరరాశి అక్షరం *name, u64 defval);
u32 romfile_loadbool(స్థిరరాశి అక్షరం *name, u32 defval);

శూన్య const_romfile_కూడు_int(అక్షరం *name, u32 విలువ);

#అయితే_ముగింపు // romfile.h
