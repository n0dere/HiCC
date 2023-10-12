#   This file is part of Hilight Color Changer (HiCC).
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with HiCC.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)
project(HiCC VERSION 1.0)

if (NOT WIN32)
    message(FATAL_ERROR "Only Microsoft Windows")
endif ()

add_executable(HiCC WIN32)

target_sources(HiCC PRIVATE SRC/bmutils.cpp SRC/clrkmeans.c SRC/clrregistry.c
                            SRC/controls.c SRC/dlgabout.c SRC/resource.rc
                            SRC/wallpaper.c SRC/wndmain.c)

target_link_libraries(HiCC PRIVATE gdiplus msimg32 uxtheme comctl32 shlwapi)

target_compile_definitions(HiCC PRIVATE _UNICODE UNICODE)

set_target_properties(HiCC PROPERTIES WIN32_EXECUTABLE TRUE)

if (MINGW)
    target_compile_options(HiCC PRIVATE -municode)
    target_link_options(HiCC PRIVATE -municode)
    target_link_options(HiCC PRIVATE -static-libgcc -static-libstdc++ -static)
endif ()

if (MSVC)
    target_compile_definitions(HiCC PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_options(HiCC PRIVATE /MANIFEST:NO)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8) # 64 bits
    set_target_properties(HiCC PROPERTIES OUTPUT_NAME "HilightColorChanger_x64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4) # 32 bits
    set_target_properties(HiCC PROPERTIES OUTPUT_NAME "HilightColorChanger_x32")
endif ()