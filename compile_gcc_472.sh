#!/bin/bash
set -e

SCRIPT_DIR=$(dirname $(readlink -f $0) )
INSTALL_DIR=${INSTALL_DIR:-$PWD/install}
GCC_VER=4.7.2
MPFR_VER=2.4.2
GMP_VER=4.3.2
MPC_VER=0.8.1

rm -rf gcc-$GCC_VER gcc-build

tar xaf $SCRIPT_DIR/gcc-$GCC_VER.tar.bz2
cd gcc-$GCC_VER

#contrib/download_prerequisites
#wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$MPFR.tar.bz2 || exit 1
#wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$GMP.tar.bz2 || exit 1
#wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$MPC.tar.gz || exit 1

tar xaf $SCRIPT_DIR/mpfr-$MPFR_VER.tar.bz2 || exit 1
ln -sf mpfr-$MPFR_VER mpfr || exit 1

tar xaf $SCRIPT_DIR/gmp-$GMP_VER.tar.bz2  || exit 1
ln -sf gmp-$GMP_VER gmp || exit 1

tar xaf $SCRIPT_DIR/mpc-$MPC_VER.tar.gz || exit 1
ln -sf mpc-$MPC_VER mpc || exit 1

sed -i 's/install_to_$(INSTALL_DEST) //' libiberty/Makefile.in
#sed -i 's@\./fixinc\.sh@-c true@'        gcc/Makefile.in

case `uname -m` in
      i?86) sed -i 's/^T_CFLAGS =$/& -fomit-frame-pointer/' gcc/Makefile.in ;;
esac

export CFLAGS+=" -I/usr/include/x86_64-linux-gnu/"
export CXXFLAGS+=" -I/usr/include/x86_64-linux-gnu/"

mkdir -p ../gcc-build
cd ../gcc-build
../gcc-$GCC_VER/configure         \
    --prefix="$INSTALL_DIR"       \
    --with-system-zlib         \
    --enable-shared            \
    --enable-threads=posix     \
    --enable-__cxa_atexit      \
    --disable-multilib         \
    --enable-bootstrap         \
    --enable-clocale=gnu       \
    --enable-languages=c,c++,fortran

make bootstrap -j6
make -j6
make install

../gcc-$GCC_VER/contrib/test_summary
