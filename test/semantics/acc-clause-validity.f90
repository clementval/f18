! OPTIONS: -fopenacc

! Check OpenACC clause validity for the following construct and directive:
!   2.6.5 Data
!   2.5.1 Parallel
!   2.5.2 Kernels
!   2.5.3 Serial
!   2.15.1 Routine

program openacc_clause_validity

  implicit none

  integer :: i
  integer :: N = 256
  !ERROR: At least one clause is required on the DECLARE directive
  !$acc declare
  real(8) :: a(256)

  !ERROR: At least one of ATTACH, COPYIN, CREATE clause must appear on the ENTER DATA directive
  !$acc enter data

  !ERROR: At least one of COPYOUT, DELETE, DETACH clause must appear on the EXIT DATA directive
  !$acc exit data

  !ERROR: At least one of USE_DEVICE clause must appear on the HOST_DATA directive
  !$acc host_data
  !$acc end host_data

  !ERROR: At least one of DEFAULT_ASYNC, DEVICE_NUM, DEVICE_TYPE clause must appear on the SET directive
  !$acc set

  !ERROR: At least one of ATTACH, COPY, COPYIN, COPYOUT, DEFAULT, CREATE, DEVICEPTR, NO_CREATE, PRESENT clause must appear on the DATA directive
  !$acc data
  !$acc end data

  !$acc update device(i) device_type(*) async

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the UPDATE directive
  !$acc update device(i) device_type(*) if(.TRUE.)

  !$acc parallel device_type(*) num_gangs(2)
  !$acc loop
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end parallel

  !$acc parallel
  !ERROR: Clause PRIVATE is not allowed after clause DEVICE_TYPE on the LOOP directive
  !$acc loop device_type(*) private(i)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end parallel

  !$acc parallel
  !ERROR: Clause GANG is not allowed if clause SEQ appears on the LOOP directive
  !$acc loop gang seq
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

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the PARALLEL LOOP directive
  !$acc parallel loop device_type(*) if(.TRUE.)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end parallel loop

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

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the KERNELS LOOP directive
  !$acc kernels loop device_type(*) if(.TRUE.)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end kernels loop

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

  !ERROR: Clause IF is not allowed after clause DEVICE_TYPE on the SERIAL LOOP directive
  !$acc serial loop device_type(*) if(.TRUE.)
  do i = 1, N
    a(i) = 3.14
  end do
  !$acc end serial loop

contains

  subroutine sub1(a)
    real :: a(:)
    !ERROR: At least one of GANG, SEQ, VECTOR, WORKER clause must appear on the ROUTINE directive
    !$acc routine
  end subroutine sub1

end program openacc_clause_validity