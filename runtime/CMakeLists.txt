#===-- runtime/CMakeLists.txt ----------------------------------------------===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#------------------------------------------------------------------------------#

add_library(FortranRuntime
  ISO_Fortran_binding.cc
  derived-type.cc
  descriptor.cc
  transformational.cc
  type-code.cc
)

target_link_libraries(FortranRuntime
  FortranEvaluate
)
