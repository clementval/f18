! RUN: %S/test_symbols.sh %s %flang %t
!OPTIONS: -fopenacc

! Test clauses that accept list.
! 2.1 Directive Format
!   A list consists of a comma-separated collection of one or more list items.
!   A list item is a variable, array section or common block name (enclosed in
!   slashes).

program mm

 real x, y
 integer b(10), i

 b = 2
 !$acc parallel
 do i=1,10
 end do
 !$acc end parallel
end program
