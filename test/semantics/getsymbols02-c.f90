! Tests -fget-symbols-sources with modules.

PROGRAM helloworld
    use mm2b
    implicit none
    integer::i
    i = callget5()
ENDPROGRAM

! RUN: ${F18} -fget-symbols-sources -fparse-only -fdebug-semantics %s 2>&1 | ${FileCheck} %s
! CHECK:callget5: mm2b
! CHECK:get5: mm2a
