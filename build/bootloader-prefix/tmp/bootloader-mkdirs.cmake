# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lukasz/.platformio/packages/framework-espidf/components/bootloader/subproject"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/tmp"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/src/bootloader-stamp"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/src"
  "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/media/lukasz/55DFF36A13D271611/programming/PPFDmeter/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
