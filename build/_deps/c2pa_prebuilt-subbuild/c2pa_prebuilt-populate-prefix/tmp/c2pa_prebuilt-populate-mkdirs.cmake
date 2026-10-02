# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-src"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-build"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/tmp"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/src/c2pa_prebuilt-populate-stamp"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/src"
  "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/src/c2pa_prebuilt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/src/c2pa_prebuilt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/comictor2ro/Desktop/panoramax_qa/build/_deps/c2pa_prebuilt-subbuild/c2pa_prebuilt-populate-prefix/src/c2pa_prebuilt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
