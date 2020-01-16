! OPTIONS: -fopenacc

! Check OpenACC clause validity for the following directives:
!
!    2.5 PARALLEL construct
!    2.7.1 Loop construct
!    ...

program openacc_clause_validity

  implicit none

  integer :: i
  integer :: N = 256
  real(8) :: a(256)

  !$acc parallel device_type(*) num_gangs(2)
  !$acc loop
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end parallel

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the PARALLEL directive
  !$acc parallel device_type(*) if(.TRUE.)
  !$acc loop
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end parallel

  !$acc kernels device_type(*) async
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end kernels

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the KERNELS directive
  !$acc kernels device_type(*) if(.TRUE.)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end kernels

  !$acc serial device_type(*) async
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end serial

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the SERIAL directive
  !$acc serial device_type(*) if(.TRUE.)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end serial

end program openacc_clause_validity