
AC_DEFUN([AC_CC_CAN_EMMS],
[AC_CACHE_CHECK(whether CC can assemble emms instruction, ac_cc_can_emms,
        [ AC_TRY_LINK(,[unsigned long a;
	asm("emms");
	return a;],
	ac_cc_can_emms=yes, ac_cc_can_emms=no)])
])

AC_DEFUN([AC_CPU_CAN_EMMS],
[AC_CACHE_CHECK(whether build CPU can run the EMMS instruction,
	ac_cpu_can_emms,
        [ AC_ERROR(Test not yet implemented!) ],
	ac_cpu_can_emms=no, ac_cpu_can_emms=no)])
])

AC_DEFUN([AC_CC_CAN_FEMMS],
[AC_CACHE_CHECK(whether CC can assemble femms instruction, ac_cc_can_femms,
        [ AC_TRY_LINK(,[unsigned long a;
	asm("femms");
	return a;],
	ac_cc_can_femms=yes, ac_cc_can_femms=no)])
])

AC_DEFUN([AC_CPU_CAN_FEMMS],
[AC_CACHE_CHECK(whether build CPU can run the FEMMS instruction,
	ac_cpu_can_femms,
        [ AC_ERROR(Test not yet implemented!) ],
	ac_cpu_can_femms=no, ac_cpu_can_femms=no)])
])

AC_DEFUN([AC_CC_CAN_FALIGNDATA],
[AC_CACHE_CHECK(whether CC can assemble faligndata instruction, ac_cc_can_faligndata,
        [ AC_TRY_LINK(,[
	__asm__ __volatile__("faligndata %%f0, %%f4, %%f8" : );],
	ac_cc_can_faligndata=yes, ac_cc_can_faligndata=no)])
])

AC_DEFUN([AC_CPU_CAN_FALIGNDATA],
[AC_CACHE_CHECK(whether build CPU can run the FALIGNDATA instruction,
	ac_cpu_can_faligndata,
        [ AC_ERROR(Test not yet implemented!) ],
	ac_cpu_can_faligndata=no, ac_cpu_can_faligndata=no)])
])

