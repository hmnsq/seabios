#నిర్వచితం_కాకపోతే __XEN_H
#నిర్వచించు __XEN_H

శూన్య xen_preinit(శూన్య);
శూన్య xen_ramsize_preinit(శూన్య);
శూన్య xen_hypercall_అమరిక(శూన్య);
శూన్య xen_biostable_అమరిక(శూన్య);

బాహ్య గుర్తులేని దీర్ఘ xen_hypercall_పుట;

#నిర్వచించు _hypercall0(type, name)                                         \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res;                                                         \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res)                                                  \
        : "0" (__hentry)                                                \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#నిర్వచించు _hypercall1(type, name, a1)                                     \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res, __ign1;                                                 \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1)                                   \
        : "0" (__hentry), "1" ((దీర్ఘ)(a1))                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#నిర్వచించు _hypercall2(type, name, a1, a2)                                 \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res, __ign1, __ign2;                                         \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                    \
        : "0" (__hentry), "1" ((దీర్ఘ)(a1)), "2" ((దీర్ఘ)(a2))            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#నిర్వచించు _hypercall3(type, name, a1, a2, a3)                             \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res, __ign1, __ign2, __ign3;                                 \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3)                                                 \
        : "0" (__hentry), "1" ((దీర్ఘ)(a1)), "2" ((దీర్ఘ)(a2)),           \
          "3" ((దీర్ఘ)(a3))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#నిర్వచించు _hypercall4(type, name, a1, a2, a3, a4)                         \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res, __ign1, __ign2, __ign3, __ign4;                         \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4)                                  \
        : "0" (__hentry), "1" ((దీర్ఘ)(a1)), "2" ((దీర్ఘ)(a2)),           \
          "3" ((దీర్ఘ)(a3)), "4" ((దీర్ఘ)(a4))                            \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

#నిర్వచించు _hypercall5(type, name, a1, a2, a3, a4, a5)                     \
({                                                                      \
    గుర్తులేని దీర్ఘ __hentry = xen_hypercall_పుట+__HYPERVISOR_##name*32; \
    దీర్ఘ __res, __ign1, __ign2, __ign3, __ign4, __ign5;                 \
    asm అస్థిర (                                                      \
        "call *%%eax"                                                   \
        : "=a" (__res), "=b" (__ign1), "=c" (__ign2),                   \
          "=d" (__ign3), "=S" (__ign4), "=D" (__ign5)                   \
        : "0" (__hentry), "1" ((దీర్ఘ)(a1)), "2" ((దీర్ఘ)(a2)),           \
          "3" ((దీర్ఘ)(a3)), "4" ((దీర్ఘ)(a4)),                           \
          "5" ((దీర్ఘ)(a5))                                              \
        : "memory" );                                                   \
    (type)__res;                                                        \
})

/******************************************************************************
 *
 * The following interface definitions are taken from Xen and have the
 * following license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* xen.h */

#నిర్వచించు __HYPERVISOR_xen_version          17

/* version.h */

/* arg == xen_extraversion_t. */
#నిర్వచించు XENVER_extraversion 1
రకనిర్వచనం అక్షరం xen_extraversion_t[16];
#నిర్వచించు XEN_EXTRAVERSION_LEN (పరిమాణం(xen_extraversion_t))

#అయితే_ముగింపు
