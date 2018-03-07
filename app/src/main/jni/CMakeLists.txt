#
# Copyright (C)  The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=format-security")

add_library(droid48 SHARED
    actions.c binio.c debugger.c device.c
  disasm.c emulate.c init.c
  lcd.c main.c memory.c options.c register.c
  resources.c romio.c rpl.c serial.c timer.c x48.c)

# add lib dependencies
target_link_libraries(droid48
    android
    log)