#!/bin/bash
set -eu

#
# tools/build-win64-toolchain.sh: Win64 toolchain build script.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

BINUTILS="ftp://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.bz2"
GCC="ftp://ftp.gnu.org/gnu/gcc/gcc-8.1.0/gcc-8.1.0.tar.gz"
GMP="ftp://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.bz2"
MAKE="ftp://ftp.gnu.org/gnu/make/make-4.2.1.tar.bz2"
MPC="ftp://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz"
MPFR="ftp://ftp.gnu.org/gnu/mpfr/mpfr-4.0.1.tar.bz2"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${SCRIPT_DIR} && mkdir -p {stamps,tarballs}

export PATH="${PATH}:${SCRIPT_DIR}/bin"

if [ ! -f stamps/binutils-download ]; then
  wget "${BINUTILS}" -O "tarballs/$(basename ${BINUTILS})"
  touch stamps/binutils-download
fi

if [ ! -f stamps/binutils-extract ]; then
  mkdir -p binutils-{build,source}
  tar -xf tarballs/$(basename ${BINUTILS}) -C binutils-source --strip 1
  touch stamps/binutils-extract
fi

if [ ! -f stamps/binutils-configure ]; then
  pushd binutils-build
  ../binutils-source/configure \
    --build=x86_64-linux-gnu \
    --host=x86_64-w64-mingw32 \
    --prefix="${SCRIPT_DIR}" \
    --with-lib-path="${SCRIPT_DIR}/lib" \
    --target=mips64-elf --with-arch=vr4300 \
    --enable-64-bit-bfd \
    --enable-plugins \
    --enable-shared \
    --disable-gold \
    --disable-multilib \
    --disable-nls \
    --disable-rpath \
    --disable-static \
    --disable-werror
  popd

  touch stamps/binutils-configure
fi

if [ ! -f stamps/binutils-build ]; then
  pushd binutils-build
  make
  popd

  touch stamps/binutils-build
fi

if [ ! -f stamps/binutils-install ]; then
  pushd binutils-build
  make install
  popd

  touch stamps/binutils-install
fi

if [ ! -f stamps/gmp-download ]; then
  wget "${GMP}" -O "tarballs/$(basename ${GMP})"
  touch stamps/gmp-download
fi

if [ ! -f stamps/mpfr-download ]; then
  wget "${MPFR}" -O "tarballs/$(basename ${MPFR})"
  touch stamps/mpfr-download
fi

if [ ! -f stamps/mpc-download ]; then
  wget "${MPC}" -O "tarballs/$(basename ${MPC})"
  touch stamps/mpc-download
fi

if [ ! -f stamps/gcc-download ]; then
  wget "${GCC}" -O "tarballs/$(basename ${GCC})"
  touch stamps/gcc-download
fi

if [ ! -f stamps/gcc-extract ]; then
  mkdir -p gcc-{build,source}
  tar -xf tarballs/$(basename ${GCC}) -C gcc-source --strip 1
  touch stamps/gcc-extract
fi

if [ ! -f stamps/gmp-extract ]; then
  mkdir -p gcc-source/gmp
  tar -xf tarballs/$(basename ${GMP}) -C gcc-source/gmp --strip 1
  touch stamps/gmp-extract
fi

if [ ! -f stamps/mpfr-extract ]; then
  mkdir -p gcc-source/mpfr
  tar -xf tarballs/$(basename ${MPFR}) -C gcc-source/mpfr --strip 1
  touch stamps/mpfr-extract
fi

if [ ! -f stamps/mpc-extract ]; then
  mkdir -p gcc-source/mpc
  tar -xf tarballs/$(basename ${MPC}) -C gcc-source/mpc --strip 1
  touch stamps/mpc-extract
fi

if [ ! -f stamps/gcc-configure ]; then
  pushd gcc-build
  ../gcc-source/configure \
    --build=x86_64-linux-gnu \
    --host=x86_64-w64-mingw32 \
    --prefix="${SCRIPT_DIR}" \
    --target=mips64-elf --with-arch=vr4300 \
    --enable-languages=c --without-headers --with-newlib \
    --with-gnu-as=${SCRIPT_DIR}/bin/mips64-elf-as.exe \
    --with-gnu-ld=${SCRIPT_DIR}/bin/mips64-elf-ld.exe \
    --enable-checking=release \
    --enable-shared \
    --enable-shared-libgcc \
    --disable-decimal-float \
    --disable-gold \
    --disable-libatomic \
    --disable-libgomp \
    --disable-libitm \
    --disable-libquadmath \
    --disable-libquadmath-support \
    --disable-libsanitizer \
    --disable-libssp \
    --disable-libunwind-exceptions \
    --disable-libvtv \
    --disable-multilib \
    --disable-nls \
    --disable-rpath \
    --disable-static \
    --disable-symvers \
    --disable-threads \
    --disable-win32-registry \
    --enable-lto \
    --enable-plugin \
    --without-included-gettext
  popd

  touch stamps/gcc-configure
fi

if [ ! -f stamps/gcc-build ]; then
  pushd gcc-build
  make all-gcc
  popd

  touch stamps/gcc-build
fi

if [ ! -f stamps/gcc-install ]; then
  pushd gcc-build
  make install-gcc
  popd

  # While not necessary, this is still a good idea.
  pushd "${SCRIPT_DIR}/bin"
  cp mips64-elf-{gcc,cc}.exe
  popd

  touch stamps/gcc-install
fi

if [ ! -f stamps/make-download ]; then
  wget "${MAKE}" -O "tarballs/$(basename ${MAKE})"
  touch stamps/make-download
fi

if [ ! -f stamps/make-extract ]; then
  mkdir -p make-{build,source}
  tar -xf tarballs/$(basename ${MAKE}) -C make-source --strip 1
  touch stamps/make-extract
fi

if [ ! -f stamps/make-configure ]; then
  pushd make-build
  ../make-source/configure \
    --build=x86_64-linux-gnu \
    --host=x86_64-w64-mingw32 \
    --prefix="${SCRIPT_DIR}" \
    --disable-largefile \
    --disable-nls \
    --disable-rpath
  popd

  touch stamps/make-configure
fi

if [ ! -f stamps/make-build ]; then
  pushd make-build
  make
  popd

  touch stamps/make-build
fi

if [ ! -f stamps/make-install ]; then
  pushd make-build
  make install
  popd

  touch stamps/make-install
fi

if [ ! -f stamps/checksum-build ]; then
  x86_64-w64-mingw32-gcc -Wall -Wextra -pedantic -std=c99 -O2 checksum.c -o bin/checksum.exe

  touch stamps/checksum-build
fi

if [ ! -f stamps/mkfs-build ]; then
  x86_64-w64-mingw32-gcc -Wall -Wextra -pedantic -std=c99 -O2 mkfs.c -o bin/mkfs.exe

  touch stamps/mkfs-build
fi

if [ ! -f stamps/rspasm-build ]; then
  pushd "${SCRIPT_DIR}/../rspasm"

  make clean
  CC=x86_64-w64-mingw32-gcc RSPASM_LIBS="-lws2_32" make
  cp rspasm ${SCRIPT_DIR}/bin/rspasm.exe
fi

rm -rf "${SCRIPT_DIR}"/../tools/tarballs
rm -rf "${SCRIPT_DIR}"/../tools/*-source
rm -rf "${SCRIPT_DIR}"/../tools/*-build
rm -rf "${SCRIPT_DIR}"/../tools/stamps
exit 0

