!ERROR: INTEGER(KIND=0) is not a supported type
integer(kind=0) :: j0
!ERROR: INTEGER(KIND=-1) is not a supported type
integer(kind=-1) :: jm1
!ERROR: INTEGER(KIND=3) is not a supported type
integer(kind=3) :: j3
!ERROR: INTEGER(KIND=32) is not a supported type
integer(kind=32) :: j32
!ERROR: REAL(KIND=0) is not a supported type
real(kind=0) :: a0
!ERROR: REAL(KIND=-1) is not a supported type
real(kind=-1) :: am1
!ERROR: REAL(KIND=1) is not a supported type
real(kind=1) :: a1
!ERROR: REAL(KIND=7) is not a supported type
real(kind=7) :: a7
!ERROR: REAL(KIND=32) is not a supported type
real(kind=32) :: a32
!ERROR: COMPLEX(KIND=0) is not a supported type
complex(kind=0) :: z0
!ERROR: COMPLEX(KIND=-1) is not a supported type
complex(kind=-1) :: zm1
!ERROR: COMPLEX(KIND=1) is not a supported type
complex(kind=1) :: z1
!ERROR: COMPLEX(KIND=7) is not a supported type
complex(kind=7) :: z7
!ERROR: COMPLEX(KIND=32) is not a supported type
complex(kind=32) :: z32
!ERROR: COMPLEX*1 is not a supported type
complex*1 :: zs1
!ERROR: COMPLEX*2 is not a supported type
complex*2 :: zs2
!ERROR: COMPLEX*64 is not a supported type
complex*64 :: zs64
!ERROR: LOGICAL(KIND=0) is not a supported type
logical(kind=0) :: l0
!ERROR: LOGICAL(KIND=-1) is not a supported type
logical(kind=-1) :: lm1
!ERROR: LOGICAL(KIND=3) is not a supported type
logical(kind=3) :: l3
!ERROR: LOGICAL(KIND=16) is not a supported type
logical(kind=16) :: l16
end program
