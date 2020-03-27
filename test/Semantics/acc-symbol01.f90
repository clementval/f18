! RUN: %S/test_symbols.sh %s %flang %t
!OPTIONS: -fopenacc

! Test clauses that accept list.
! 2.1 Directive Format
!   A list consists of a comma-separated collection of one or more list items.
!   A list item is a variable, array section or common block name (enclosed in
!   slashes).

program mm

 real x, y
 integer a(10), b(10), c(10), i
 
 b = 2
 !$acc parallel present(c) firstprivate(b) private(a)
 !$acc loop 
 do i=1,10
  a(i) = b(i)
 end do
 !$acc end parallel
end program
