
AC_DEFUN([AC_CC_CAN_INT64],
[AC_MSG_CHECKING(whether CC thinks type $GGI_64 is a fully working 64-bit int)
 AC_TRY_LINK([#include <stdio.h>],[
              unsigned $GGI_64 u64; 
              $GGI_64 s64;
              u64 = 0x123456789abcdef0 ; 
              u64 <<= 1; printf("%llx", u64);
	      u64 *= 17; printf("%llx", u64);
              u64 += 300; printf("%llx", u64);
              u64 -= 300; printf("%llx", u64);
              u64 /= 17; printf("%%llx", u64);
	      u64 ^= 0xfefefefefefefefe; printf("%llx", u64);
	      u64 %= 3; printf("%%llx", u64);
              s64 = -0x123456789abcdef0; 
              s64 <<= 1; printf("%llx", s64);
	      s64 *= 17; printf("%llx", s64);
              s64 += 300; printf("%llx", s64);
              s64 -= 300; printf("%llx", s64);
              s64 /= 17; printf("%%llx", s64);
	      s64 ^= 0xfefefefefefefefe; printf("%llx", s64);
	      s64 %= 3; printf("%%llx", s64);
	      s64 *= u64; printf("%%llx", s64);
              return u64;],
          [ AC_MSG_RESULT(yes)
            ac_cc_can_int64="yes" ], 
          [ AC_MSG_RESULT(nope)
            ac_cc_can_int64="no" ])
])

AC_DEFUN([AC_CC_CAN_CPUID],
[AC_CACHE_CHECK(whether CC can assemble cpuid instruction, ac_cc_can_cpuid,
        [ AC_TRY_LINK(,[unsigned long a,b,c,d,in; 
	asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));
	return d;],
	ac_cc_can_cpuid="yes", ac_cc_can_cpuid="no")])
	if test "x$ac_cc_can_cpuid" = "xyes"; then
	     AC_DEFINE(CC_CAN_CPUID, 1,
		       [Define if CC can compile cpuid opcode (Intel)])
	fi
])

AC_DEFUN([AC_CC_CAN_AMASK],
[AC_CACHE_CHECK(whether CC can assemble amask instruction, ac_cc_can_amask,
        [ AC_TRY_LINK(,[unsigned long a,b; 
	asm("amask %1, %0": "=r" (a) : "ri" (b));
	return a;],
	ac_cc_can_amask="yes", ac_cc_can_amask="no")])
	if test "x$ac_cc_can_amask" = "xyes"; then
	     AC_DEFINE(CC_CAN_AMASK, 1,
		       [Define if CC can compile amask opcode (DEC)])
	fi
])
