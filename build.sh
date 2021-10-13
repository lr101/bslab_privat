#!/bin/sh
if [ ! -d "build" ]; then mkdir build; fi
cd build
cmake ..
num_of_cores="$(nproc)"
num_of_cores=$(($num_of_cores <= 3 ? $num_of_cores : 3))
printf '\e[33m%b\e[0m' "Using $num_of_cores jobs for compiling\n"
make -j$num_of_cores
if [ ! -d "mount" ]; then mkdir mount; fi
printf '\e[33m%b\e[0m' "\nTo mount the filesystem, use 'bin/mount.myfs mount -l log.txt' inside the build directory\n"
