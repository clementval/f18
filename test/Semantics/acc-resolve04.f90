! RUN: %B/test/Semantics/test_errors.sh %s %flang %t
! OPTIONS: -fopenacc

! 2.15.3 Data-Sharing Attribute Clauses
! 2.15.3.1 default Clause

subroutine default_none()
  integer :: a(3), c, i

  a = 1
  !ERROR: 'c' appears in more than one data-sharing clause on the same OpenACC directive
  !$acc parallel firstprivate(c) private(c)
  do i = 0, 3
    a(i) = c
  end do
  !$acc end parallel
end subroutine default_none

program mm
  call default_none()
  !TODO: private, firstprivate, shared
end
