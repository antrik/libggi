
AC_DEFUN([GGI_CC_CAN_EMMS],
[AC_CACHE_CHECK([whether $CC can assemble emms instruction],
	ggi_cv_cc_can_emms,
	[ AC_TRY_LINK(,[unsigned long a;
	asm("emms");
	return a;],
	ggi_cv_cc_can_emms=yes, ggi_cv_cc_can_emms=no)])
])

AC_DEFUN([GGI_CPU_CAN_EMMS],
[AC_CACHE_CHECK([whether build CPU can run the EMMS instruction],
	ggi_cv_cpu_can_emms,
	[ AC_ERROR([Test not yet implemented!]) ],
	ggi_cpu_can_emms=no, ggi_cpu_can_emms=no)])
])

AC_DEFUN([GGI_CC_CAN_FEMMS],
[AC_CACHE_CHECK([whether $CC can assemble femms instruction],
	ggi_cv_cc_can_femms,
	[ AC_TRY_LINK(,[unsigned long a;
	asm("femms");
	return a;],
	ggi_cv_cc_can_femms=yes, ggi_cv_cc_can_femms=no)])
])

AC_DEFUN([GGI_CPU_CAN_FEMMS],
[AC_CACHE_CHECK([whether build CPU can run the FEMMS instruction],
	ggi_cv_cpu_can_femms,
	[ AC_ERROR([Test not yet implemented!]) ],
	ggi_cv_cpu_can_femms=no, ggi_cv_cpu_can_femms=no)])
])

AC_DEFUN([GGI_CC_CAN_FALIGNDATA],
[AC_CACHE_CHECK([whether $CC can assemble faligndata instruction],
	ggi_cv_cc_can_faligndata,
	[ AC_TRY_LINK(,[
	__asm__ __volatile__("faligndata %%f0, %%f4, %%f8" : );],
	ggi_cv_cc_can_faligndata=yes, ggi_cv_cc_can_faligndata=no)])
])

AC_DEFUN([GGI_CPU_CAN_FALIGNDATA],
[AC_CACHE_CHECK([whether build CPU can run the FALIGNDATA instruction],
	ggi_cv_cpu_can_faligndata,
	[ AC_ERROR([Test not yet implemented!]) ],
	ggi_cv_cpu_can_faligndata=no, ggi_cv_cpu_can_faligndata=no)])
])

AC_DEFUN([GGI_CC_CAN_PKLB],
[AC_CACHE_CHECK([whether $CC can assemble pklb instruction],
	ggi_cv_cc_can_pklb,
	[ AC_TRY_LINK(,[
	unsigned long foo;
	__asm__ __volatile__("pklb %1, %0" : "=r" (foo) : "rI" (foo));],
	ggi_cv_cc_can_pklb=yes, ggi_cv_cc_can_pklb=no)])
])

AC_DEFUN([GGI_CPU_CAN_PKLB],
[AC_CACHE_CHECK([whether build CPU can run the PKLB instruction],
	ggi_cv_cpu_can_pklb,
	[ AC_ERROR([Test not yet implemented!]) ],
	ggi_cv_cpu_can_pklb=no, ggi_cv_cpu_can_pklb=no)])
])

