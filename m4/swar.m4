
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


