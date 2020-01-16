! OPTIONS: -fopenacc

! Check OpenACC restruction in branch in and out of some construct
!

program openacc_clause_validity

    implicit none

    integer :: i
    integer :: N = 256
    real(8) :: a(256)

    !$acc parallel device_type(*) num_gangs(2)
    !$acc loop
    do i = 1, N
        a(i) = 3.14
        !ERROR: RETURN statement is not allowed in a PARALLEL construct
        return
    end do
    !$acc end parallel

    !$acc parallel device_type(*) num_gangs(2)
    !$acc loop
    do i = 1, N
        a(i) = 3.14
        if(i == N-1) THEN
            !ERROR: EXIT statement is not allowed in a PARALLEL construct
            exit
        end if
    end do
    !$acc end parallel

end program openacc_clause_validity
