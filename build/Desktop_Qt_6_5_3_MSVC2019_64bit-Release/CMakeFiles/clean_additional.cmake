# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\CMakeProject1_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CMakeProject1_autogen.dir\\ParseCache.txt"
  "CMakeProject1_autogen"
  )
endif()
