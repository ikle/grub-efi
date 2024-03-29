dnl Configure script for GRUB.
dnl Copyright 1999,2000,2001,2002,2003,2004,2005 Free Software Foundation, Inc.

dnl Permission to use, copy, modify and distribute this software and its
dnl documentation is hereby granted, provided that both the copyright
dnl notice and this permission notice appear in all copies of the
dnl software, derivative works or modified versions, and any portions
dnl thereof, and that both notices appear in supporting documentation.
dnl
dnl THE FREE SOFTWARE FOUNDATION ALLOWS FREE USE OF THIS SOFTWARE IN ITS
dnl "AS IS" CONDITION.  THE FREE SOFTWARE FOUNDATION DISCLAIMS ANY
dnl LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE
dnl USE OF THIS SOFTWARE.

AC_PREREQ(2.57)
AC_INIT([GRUB], [0.97], [bug-grub@gnu.org])
AC_CONFIG_SRCDIR([stage2/stage2.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([subdir-objects])

AC_CANONICAL_HOST

case "$host_cpu" in
i[[3456]]86) host_cpu=i386 ;;
x86_64) host_cpu=x86_64 ;;
*) AC_MSG_ERROR([unsupported CPU type]) ;;
esac

AC_SUBST(host_cpu)
AC_SUBST(host_vendor)

# Specify the platform (such as firmware).
AC_ARG_WITH([platform],
            AS_HELP_STRING([--with-platform=PLATFORM],
                           [select the host platform [[guessed]]]))

# Guess the platform if not specified.
if test "x$with_platform" = x; then
  case "$host_cpu"-"$host_vendor" in
    i386-*) platform=pc ;;
    x86_64-*) platform=pc ;;
    *) AC_MSG_ERROR([unsupported machine type $host_cpu-$host_vendor]) ;;
  esac
else
  platform="$with_platform"
fi

# Sanity check.
case "$host_cpu"-"$platform" in
  i386-pc) ;;
  i386-efi) ;;
  x86_64-pc) ;;
  x86_64-efi) ;;
  *) AC_MSG_ERROR([unsupported machine type "$host_cpu"-"$platform"]) ;;
esac

if test "x$platform" = xefi; then
  case "$host_cpu" in
    i386) EFI_ARCH=ia32 ;;
    x86_64) EFI_ARCH=x86_64 ;;
  *) AC_MSG_ERROR([unsupported machine type "$host_cpu"]) ;;
  esac
  AC_SUBST(EFI_ARCH)
fi

gnuefi_path=${libdir}
gnuefi_crt0=${gnuefi_path}/crt0-efi-${EFI_ARCH}.o
if ! test -f $gnuefi_crt0 ; then
  gnuefi_crt0=crt0-efi.o
fi
GNUEFI_CRT0=${gnuefi_crt0}
AC_SUBST(GNUEFI_CRT0)
gnuefi_lds=${gnuefi_path}/elf_${EFI_ARCH}_efi.lds
if ! test -f $gnuefi_lds ; then
  gnuefi_lds=${EFI_ARCH}/elf_efi.lds
fi
GNUEFI_LDS=${gnuefi_lds}
AC_SUBST(GNUEFI_LDS)
LIBGNUEFI=${libdir}/libgnuefi.a
AC_SUBST(LIBGNUEFI)
LIBEFI=${libdir}/libefi.a
AC_SUBST(LIBEFI)

AC_SUBST(platform)
AM_CONDITIONAL(PLATFORM_EFI, test "x$platform" = xefi)

if test "x$platform" = xefi; then
   AC_DEFINE(PLATFORM_EFI, 1, [Define if you run on EFI platform.])
fi

#
# Options
#

AM_MAINTAINER_MODE
if test "x$enable_maintainer_mode" = xyes; then
  AC_PATH_PROG(PERL,perl)
  if test -z "$PERL"; then
    AC_MSG_ERROR([perl not found])
  fi
fi

if test "x$platform" = xpc -a "x$host_cpu" = xx86_64; then
  CFLAGS="-m32 $CFLAGS"
fi

#
# Programs
#

AC_CHECK_TOOL(CC, gcc)
AC_PROG_CC
# We need this for older versions of Autoconf.
_AM_DEPENDENCIES(CC)

dnl Because recent automake complains about AS, set it here.
AM_PROG_AS

AC_ARG_WITH(binutils,
  [  --with-binutils=DIR     search the directory DIR to find binutils])

if test "x$with_binutils" != x; then
dnl AC_PATH_TOOL is not seen in autoconf 2.13, so use AC_PATH_PROG
dnl instead for now. It is preferable when you cross-compile GRUB.
dnl  AC_PATH_TOOL(RANLIB, ranlib, :, "$with_binutils:$PATH")
  AC_PATH_PROG(RANLIB, ranlib, :, "$with_binutils:$PATH")
else
  AC_PROG_RANLIB
fi

# optimization flags
if test "x$ac_compiler_gnu" = xyes; then
    # Autoconf may set CFLAGS to -O2 and/or -g. So eliminate them.
    CFLAGS="`echo $CFLAGS | sed -e 's/-g//g' -e 's/-O[[0-9]]//g'` -g"
    # If the user specify the directory for binutils, add the option `-B'.
    if test "x$with_binutils" != x; then
      CFLAGS="-B$with_binutils/ $CFLAGS"
    fi
    STAGE1_CFLAGS="-O2"
    GRUB_CFLAGS="-O2"
    AC_CACHE_CHECK([whether optimization for size works], grub_cv_cc_Os, [
      saved_CFLAGS=$CFLAGS
      CFLAGS="-Os -g"
      AC_TRY_COMPILE(, , grub_cv_cc_Os=yes, grub_cv_cc_Os=no)
      CFLAGS=$saved_CFLAGS
    ])
    if test "x$grub_cv_cc_Os" = xyes; then
      STAGE2_CFLAGS="-Os"
    else
      STAGE2_CFLAGS="-O2 -fno-strength-reduce -fno-unroll-loops"
    fi
    # OpenBSD has a GCC extension for protecting applications from
    # stack smashing attacks, but GRUB doesn't want this feature.
    AC_CACHE_CHECK([whether gcc has -fno-stack-protector],
		   grub_cv_cc_no_stack_protector, [
      saved_CFLAGS=$CFLAGS
      CFLAGS="-fno-stack-protector"
      AC_TRY_COMPILE(,
		     ,
		     grub_cv_cc_no_stack_protector=yes,
		     grub_cv_cc_no_stack_protector=no)
      CFLAGS=$saved_CFLAGS
    ])
    if test "x$grub_cv_cc_no_stack_protector" = xyes; then
      STAGE2_CFLAGS="$STAGE2_CFLAGS -fno-stack-protector"
    fi
    # GCC >= 3.3 supports -fno-reorder-functions; this defends us against
    # unlikely-to-be-executed functions being linked before _start with GCC
    # >= 4.6.
    AC_CACHE_CHECK([whether gcc has -fno-reorder-functions],
		   grub_cv_cc_no_reorder_functions, [
      saved_CFLAGS=$CFLAGS
      CFLAGS="-fno-reorder-functions"
      AC_TRY_COMPILE(,
		     ,
		     grub_cv_cc_no_reorder_functions=yes,
		     grub_cv_cc_no_reorder_functions=no)
      CFLAGS=$saved_CFLAGS
    ])
    if test "x$grub_cv_cc_no_reorder_functions" = xyes; then
      STAGE2_CFLAGS="$STAGE2_CFLAGS -fno-reorder-functions"
    fi
    # GCC >= 4.6 supports -fno-combine-stack-adjustments; this defends us
    # against optimisations that confuse the stack games played by GRUB
    # itself.
    AC_CACHE_CHECK([whether gcc has -fno-combine-stack-adjustments],
		   grub_cv_cc_no_combine_stack_adjustments, [
      saved_CFLAGS=$CFLAGS
      CFLAGS="-fno-combine-stack-adjustments"
      AC_TRY_COMPILE(,
		     ,
		     grub_cv_cc_no_combine_stack_adjustments=yes,
		     grub_cv_cc_no_combine_stack_adjustments=no)
      CFLAGS=$saved_CFLAGS
    ])
    if test "x$grub_cv_cc_no_combine_stack_adjustments" = xyes; then
      GRUB_CFLAGS="$GRUB_CFLAGS -fno-combine-stack-adjustments"
    fi
    # Disable PIE if possible.
    AC_CACHE_CHECK([whether gcc has -fno-PIE],
		   grub_cv_cc_no_pie, [
      saved_CFLAGS=$CFLAGS
      CFLAGS="-fno-PIE"
      AC_TRY_COMPILE(, , grub_cv_cc_no_pie=yes, grub_cv_cc_no_pie=no)
      CFLAGS=$saved_CFLAGS
    ])
    if test "x$grub_cv_cc_no_pie" = xyes; then
      STAGE1_CFLAGS="$STAGE1_CFLAGS -fno-PIE"
      STAGE2_CFLAGS="$STAGE2_CFLAGS -fno-PIE"
    fi
    AC_CACHE_CHECK([whether gcc has -no-pie],
		   grub_cv_ld_no_pie, [
      saved_LDFLAGS=$LDFLAGS
      LDFLAGS="-no-pie"
      AC_TRY_LINK(, , grub_cv_ld_no_pie=yes, grub_cv_ld_no_pie=no)
      LDFLAGS=$saved_LDFLAGS
    ])
    if test "x$grub_cv_ld_no_pie" = xyes; then
      LDFLAGS="$LDFLAGS -no-pie"
    fi
fi
STAGE2_CFLAGS="$STAGE2_CFLAGS -isystem `$CC -print-file-name=include`"

LIBGCC=$($CC $CFLAGS -static-libgcc -print-libgcc-file-name)
AC_SUBST(LIBGCC)

LOADER_LDFLAGS="-nostdlib ${LIBGCC}"

if test "x$platform" = xpc; then
  LOADER_LDFLAGS="$LOADER_LDFLAGS -static"
fi

AC_SUBST(LOADER_LDFLAGS)

if test "x$platform" = xefi; then
  STAGE2_CFLAGS="$STAGE2_CFLAGS -fpic -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-reorder-functions"
  if test "x$EFI_ARCH" = xx86_64; then
     STAGE2_CFLAGS="$STAGE2_CFLAGS -DEFI_FUNCTION_WRAPPER"
  fi
fi

#
# We can't use builtins or standard includes
#
STAGE1_CFLAGS="$STAGE1_CFLAGS -fno-builtin -nostdinc"
STAGE2_CFLAGS="$STAGE2_CFLAGS -fno-builtin -nostdinc"

AC_SUBST(STAGE1_CFLAGS)
AC_SUBST(STAGE2_CFLAGS)
AC_SUBST(GRUB_CFLAGS)

# Enforce coding standards.
CPPFLAGS="$CPPFLAGS -Wall -Wmissing-prototypes -Wunused -Wshadow"
CPPFLAGS="$CPPFLAGS -Wpointer-arith"

AC_CACHE_CHECK([whether -Wundef works], grub_cv_cc_Wundef, [
  saved_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="-Wundef"
  AC_TRY_COMPILE(, , grub_cv_cc_Wundef=yes, grub_cv_cc_Wundef=no)
  CPPFLAGS="$saved_CPPFLAGS"
])

# The options `-falign-*' are supported by gcc 3.0 or later.
# Probably it is sufficient to only check for -falign-loops.
AC_CACHE_CHECK([whether -falign-loops works], [grub_cv_cc_falign_loop], [
  saved_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="-falign-loops=1"
  AC_TRY_COMPILE(, , [grub_cv_cc_falign_loop=yes], [grub_cv_cc_falign_loop=no])
  CPPFLAGS="$saved_CPPFLAGS"
])

# Force no alignment to save space.
if test "x$grub_cv_cc_falign_loop" = xyes; then
  CPPFLAGS="$CPPFLAGS -falign-jumps=1 -falign-loops=1 -falign-functions=1"
else
  CPPFLAGS="$CPPFLAGS -malign-jumps=1 -malign-loops=1 -malign-functions=1"
fi

if test "x$grub_cv_cc_Wundef" = xyes; then
  CPPFLAGS="$CPPFLAGS -Wundef"
fi

# Check if build ID can be disabled in the linker
AC_MSG_CHECKING([whether linker accepts `--build-id=none'])
save_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -Wl,--build-id=none"
AC_TRY_LINK(, , build_id_flag=yes, build_id_flag=no)
AC_MSG_RESULT([$build_id_flag])
LDFLAGS="$save_LDFLAGS"
if test "x$build_id_flag" = xyes; then
  LDFLAGS="$LDFLAGS -Wl,--build-id=none"
fi

if test "x$with_binutils" != x; then
dnl  AC_PATH_TOOL(OBJCOPY, objcopy, , "$with_binutils:$PATH")
  AC_PATH_PROG(OBJCOPY, objcopy, , "$with_binutils:$PATH")
else
  AC_CHECK_TOOL(OBJCOPY, objcopy)
fi

# Defined in acinclude.m4.
grub_ASM_USCORE

if test "x$platform" != xefi; then
  grub_PROG_OBJCOPY_ABSOLUTE
  if test "x$grub_cv_prog_objcopy_absolute" != xyes; then
    AC_MSG_ERROR([GRUB requires a working absolute objcopy; upgrade your binutils])
  fi
fi

grub_ASM_PREFIX_REQUIREMENT

grub_ASM_ADDR32
if test "x$grub_cv_asm_addr32" != xyes; then
  AC_MSG_ERROR([GRUB requires GAS .code16 addr32 support; upgrade your binutils])
fi

grub_ASM_ABSOLUTE_WITHOUT_ASTERISK

grub_CHECK_START_SYMBOL
grub_CHECK_USCORE_START_SYMBOL
if test "x$grub_cv_check_start_symbol" != "xyes" \
	-a "x$grub_cv_check_uscore_start_symbol" != "xyes"; then
  AC_MSG_ERROR([Neither start nor _start is defined])
fi

grub_CHECK_USCORE_USCORE_BSS_START_SYMBOL
grub_CHECK_USCORE_EDATA_SYMBOL
grub_CHECK_EDATA_SYMBOL
if test "x$grub_cv_check_uscore_uscore_bss_start_symbol" != "xyes" \
	-a "x$grub_cv_check_uscore_edata_symbol" != "xyes" \
	-a "x$grub_cv_check_edata_symbol" != "xyes"; then
  AC_MSG_ERROR([None of __bss_start, _edata, edata defined])
fi

grub_CHECK_END_SYMBOL
grub_CHECK_USCORE_END_SYMBOL
if test "x$grub_cv_check_end_symbol" != "xyes" \
	-a "x$grub_cv_check_uscore_end_symbol" != "xyes"; then
  AC_MSG_ERROR([Neither end nor _end is defined])
fi

# Check for curses libraries.
AC_ARG_WITH(curses,
  [  --without-curses        do not use curses])

# Get the filename or the whole disk and open it.
# Known to work on NetBSD.
AC_CHECK_LIB(util, opendisk, [GRUB_LIBS="$GRUB_LIBS -lutil"
  AC_DEFINE(HAVE_OPENDISK, 1, [Define if opendisk() in -lutil can be used])])

# Unless the user specify --without-curses, check for curses.
if test "x$with_curses" != "xno"; then
  AC_CHECK_LIB(ncurses, wgetch, [GRUB_LIBS="$GRUB_LIBS -Wl,-Bstatic -lncurses -ltinfo -Wl,-Bdynamic"
  AC_DEFINE(HAVE_LIBCURSES, 1, [Define if you have a curses library])],
    [AC_CHECK_LIB(curses, wgetch, [GRUB_LIBS="$GRUB_LIBS -Wl,-Bstatic -lcurses -Wl,-Bdynamic"
       AC_DEFINE(HAVE_LIBCURSES, 1, [Define if you have a curses library])])])
fi

AC_SUBST(GRUB_LIBS)

# Check for headers.
AC_CHECK_HEADERS(string.h strings.h ncurses/curses.h ncurses.h curses.h)

# Check for user options.

# filesystems support.
AC_ARG_ENABLE(ext2fs,
  [  --disable-ext2fs        disable ext2fs support in Stage 2])

if test x"$enable_ext2fs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_EXT2FS=1"
fi

AC_ARG_ENABLE(fat,
  [  --disable-fat           disable FAT support in Stage 2])

if test x"$enable_fat" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_FAT=1"
fi

AC_ARG_ENABLE(ffs,
  [  --disable-ffs           disable FFS support in Stage 2])

if test x"$enable_ffs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_FFS=1"
fi

AC_ARG_ENABLE(ufs2,
  [  --disable-ufs2          disable UFS2 support in Stage 2])

if test x"$enable_ufs2" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_UFS2=1"
fi

AC_ARG_ENABLE(minix,
  [  --disable-minix         disable Minix fs support in Stage 2])

if test x"$enable_minix" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_MINIX=1"
fi

AC_ARG_ENABLE(reiserfs,
  [  --disable-reiserfs      disable ReiserFS support in Stage 2])

if test x"$enable_reiserfs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_REISERFS=1"
fi

AC_ARG_ENABLE(vstafs,
  [  --disable-vstafs        disable VSTa FS support in Stage 2])

if test x"$enable_vstafs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_VSTAFS=1"
fi

AC_ARG_ENABLE(jfs,
  [  --disable-jfs           disable IBM JFS support in Stage 2])

if test x"$enable_jfs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_JFS=1"
fi

AC_ARG_ENABLE(xfs,
  [  --disable-xfs           disable SGI XFS support in Stage 2])

if test x"$enable_xfs" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_XFS=1"
fi

AC_ARG_ENABLE(iso9660,
  [  --disable-iso9660       disable ISO9660 support in Stage 2])

if test x"$enable_iso9660" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_ISO9660=1"
fi

dnl AC_ARG_ENABLE(tftp,
dnl [  --enable-tftp           enable TFTP support in Stage 2])
dnl 
dnl #if test x"$enable_tftp" = xyes; then
dnl FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_TFTP=1"
dnl fi

AC_ARG_ENABLE(gunzip,
  [  --disable-gunzip        disable decompression in Stage 2])

if test x"$enable_gunzip" = xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DNO_DECOMPRESSION=1"
fi

AC_ARG_ENABLE(md5-password,
  [  --disable-md5-password  disable MD5 password support in Stage 2])
if test "x$enable_md5_password" != xno; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DUSE_MD5_PASSWORDS=1"
fi

dnl The netboot support.
dnl General options.
AC_ARG_ENABLE(packet-retransmission,
  [  --disable-packet-retransmission
                          turn off packet retransmission])
if test "x$enable_packet_retransmission" != xno; then
  NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DCONGESTED=1"
fi

AC_ARG_ENABLE(pci-direct,
  [  --enable-pci-direct     access PCI directly instead of using BIOS])
if test "x$enable_pci_direct" = xyes; then
  NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DCONFIG_PCI_DIRECT=1"
fi

dnl Device drivers.
AC_ARG_ENABLE(3c509,
  [  --enable-3c509          enable 3Com509 driver])
if test "x$enable_3c509" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C509"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c509.o"
fi

AC_ARG_ENABLE(3c529,
  [  --enable-3c529          enable 3Com529 driver])
if test "x$enable_3c529" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C529=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c529.o"
fi

AC_ARG_ENABLE(3c595,
  [  --enable-3c595          enable 3Com595 driver])
if test "x$enable_3c595" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C595=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c595.o"
fi

AC_ARG_ENABLE(3c90x,
  [  --enable-3c90x          enable 3Com90x driver])
if test "x$enable_3c90x" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C90X=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c90x.o"
fi

AC_ARG_ENABLE(cs89x0,
  [  --enable-cs89x0         enable CS89x0 driver])
if test "x$enable_cs89x0" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_CS89X0=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS cs89x0.o"
fi

AC_ARG_ENABLE(davicom,
  [  --enable-davicom        enable Davicom driver])
if test "x$enable_davicom" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_DAVICOM=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS davicom.o"
fi

AC_ARG_ENABLE(depca,
  [  --enable-depca          enable DEPCA and EtherWORKS driver])
if test "x$enable_depca" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_DEPCA=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS depca.o"
fi

AC_ARG_ENABLE(eepro,
  [  --enable-eepro          enable Etherexpress Pro/10 driver])
if test "x$enable_eepro" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_EEPRO=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS eepro.o"
fi

AC_ARG_ENABLE(eepro100,
  [  --enable-eepro100       enable Etherexpress Pro/100 driver])
if test "x$enable_eepro100" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_EEPRO100=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS eepro100.o"
fi

AC_ARG_ENABLE(epic100,
  [  --enable-epic100        enable SMC 83c170 EPIC/100 driver])
if test "x$enable_epic100" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_EPIC100=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS epic100.o"
fi

AC_ARG_ENABLE(3c507,
  [  --enable-3c507          enable 3Com507 driver])
if test "x$enable_3c507" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C507=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c507.o"
fi

AC_ARG_ENABLE(exos205,
  [  --enable-exos205        enable EXOS205 driver])
if test "x$enable_exos205" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_EXOS205=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS exos205.o"
fi

AC_ARG_ENABLE(ni5210,
  [  --enable-ni5210         enable Racal-Interlan NI5210 driver])
if test "x$enable_ni5210" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NI5210=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ni5210.o"
fi

AC_ARG_ENABLE(lance,
  [  --enable-lance          enable Lance PCI PCNet/32 driver])
if test "x$enable_lance" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_LANCE=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS lance.o"
fi

AC_ARG_ENABLE(ne2100,
  [  --enable-ne2100         enable Novell NE2100 driver])
if test "x$enable_ne2100" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NE2100=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ne2100.o"
fi

AC_ARG_ENABLE(ni6510,
  [  --enable-ni6510         enable Racal-Interlan NI6510 driver])
if test "x$enable_ni6510" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NI6510=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ni6510.o"
fi

AC_ARG_ENABLE(natsemi,
  [  --enable-natsemi        enable NatSemi DP8381x driver])
if test "x$enable_natsemi" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NATSEMI=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS natsemi.o"
fi

AC_ARG_ENABLE(ni5010,
  [  --enable-ni5010         enable Racal-Interlan NI5010 driver])
if test "x$enable_ni5010" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NI5010=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ni5010.o"
fi

AC_ARG_ENABLE(3c503,
  [  --enable-3c503          enable 3Com503 driver])
if test "x$enable_3c503" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_3C503=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS 3c503.o"
fi

AC_ARG_ENABLE(ne,
  [  --enable-ne             enable NE1000/2000 ISA driver])
if test "x$enable_ne" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NE=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ne.o"
fi

AC_ARG_ENABLE(ns8390,
  [  --enable-ns8390         enable NE2000 PCI driver])
if test "x$enable_ns8390" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_NS8390=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS ns8390.o"
fi

AC_ARG_ENABLE(wd,
  [  --enable-wd             enable WD8003/8013, SMC8216/8416 driver])
if test "x$enable_wd" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_WD=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS wd.o"
fi

AC_ARG_ENABLE(otulip,
  [  --enable-otulip         enable old Tulip driver])
if test "x$enable_otulip" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_OTULIP=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS otulip.o"
fi

AC_ARG_ENABLE(rtl8139,
  [  --enable-rtl8139        enable Realtek 8139 driver])
if test "x$enable_rtl8139" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_RTL8139=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS rtl8139.o"
fi

AC_ARG_ENABLE(sis900,
  [  --enable-sis900         enable SIS 900 and SIS 7016 driver])
if test "x$enable_sis900" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_SIS900=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS sis900.o"
fi

AC_ARG_ENABLE(sk-g16,
  [  --enable-sk-g16         enable Schneider and Koch G16 driver])
if test "x$enable_sk_g16" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_SK_G16=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS sk_g16.o"
fi

AC_ARG_ENABLE(smc9000,
  [  --enable-smc9000        enable SMC9000 driver])
if test "x$enable_smc9000" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_SMC9000=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS smc9000.o"
fi

AC_ARG_ENABLE(tiara,
  [  --enable-tiara          enable Tiara driver])
if test "x$enable_tiara" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_TIARA=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS tiara.o"
fi

AC_ARG_ENABLE(tulip,
  [  --enable-tulip          enable Tulip driver])
if test "x$enable_tulip" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_TULIP=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS tulip.o"
fi

AC_ARG_ENABLE(via-rhine,
  [  --enable-via-rhine      enable Rhine-I/II driver])
if test "x$enable_via_rhine" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_VIA_RHINE=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS via_rhine.o"
fi

AC_ARG_ENABLE(w89c840,
  [  --enable-w89c840        enable Winbond W89c840, Compex RL100-ATX driver])
if test "x$enable_w89c840" = xyes; then
  NET_CFLAGS="$NET_CFLAGS -DINCLUDE_W89C840=1"
  NETBOOT_DRIVERS="$NETBOOT_DRIVERS w89c840.o"
fi

dnl Check if the netboot support is turned on.
AM_CONDITIONAL(NETBOOT_SUPPORT, test "x$NET_CFLAGS" != x)
if test "x$NET_CFLAGS" != x; then
  FSYS_CFLAGS="$FSYS_CFLAGS -DFSYS_TFTP=1"
fi

dnl Extra options.
AC_ARG_ENABLE(3c503-shmem,
  [  --enable-3c503-shmem    use 3c503 shared memory mode])
if test "x$enable_3c503_shmem" = xyes; then
  NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DT503_SHMEM=1"
fi

AC_ARG_ENABLE(3c503-aui,
  [  --enable-3c503-aui      use AUI by default on 3c503 cards])
if test "x$enable_3c503_aui" = xyes; then
  NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DT503_AUI=1"
fi

AC_ARG_ENABLE(compex-rl2000-fix,
  [  --enable-compex-rl2000-fix
                          specify this if you have a Compex RL2000 PCI])
if test "x$enable_compex_rl2000_fix" = xyes; then
  NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DCOMPEX_RL2000_FIX=1"
fi

AC_ARG_ENABLE(smc9000-scan,
  [  --enable-smc9000-scan=LIST
                          probe for SMC9000 I/O addresses using LIST],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DSMC9000_SCAN=$enable_smc9000_scan"])

AC_ARG_ENABLE(ne-scan,
  [  --enable-ne-scan=LIST   probe for NE base address using LIST],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DNE_SCAN=$enable_ne_scan"],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DNE_SCAN=0x280,0x300,0x320,0x340"])

AC_ARG_ENABLE(wd-default-mem,
  [  --enable-wd-default-mem=MEM
                          set the default memory location for WD/SMC],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DWD_DEFAULT_MEM=$enable_wd_default_mem"],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DWD_DEFAULT_MEM=0xCC000"])

AC_ARG_ENABLE(cs-scan,
  [  --enable-cs-scan=LIST   probe for CS89x0 base address using LIST],
  [NET_EXTRAFLAGS="$NET_EXTRAFLAGS -DCS_SCAN=$enable_cs_scan"])

dnl Diskless
AC_ARG_ENABLE(diskless,
  [  --enable-diskless       enable diskless support])
AM_CONDITIONAL(DISKLESS_SUPPORT, test "x$enable_diskless" = xyes)

dnl Graphical splashscreen support
AC_ARG_ENABLE(graphics,
  [  --disable-graphics      disable graphics terminal support])
AM_CONDITIONAL(GRAPHICS_SUPPORT, test "x$enable_graphics" != xno)

dnl Hercules terminal
if test "x$platform" = xefi; then
  enable_hercules=no
else
  AC_ARG_ENABLE(hercules,
    [  --disable-hercules      disable hercules terminal support])
fi
AM_CONDITIONAL(HERCULES_SUPPORT, test "x$enable_hercules" != xno)

dnl Serial terminal
AC_ARG_ENABLE(serial,
  [  --disable-serial        disable serial terminal support])
AM_CONDITIONAL(SERIAL_SUPPORT, test "x$enable_serial" != xno)

dnl Simulation of the slowness of a serial device.
AC_ARG_ENABLE(serial-speed-simulation,
  [  --enable-serial-speed-simulation
                          simulate the slowness of a serial device])
AM_CONDITIONAL(SERIAL_SPEED_SIMULATION,
  test "x$enable_serial_speed_simulation" = xyes)

# Sanity check.
if test "x$enable_diskless" = xyes; then
  if test "x$NET_CFLAGS" = x; then
    AC_MSG_ERROR([You must enable at least one network driver])
  fi
fi

dnl Embed a menu string in GRUB itself.
AC_ARG_ENABLE(preset-menu,
  [  --enable-preset-menu=FILE
                          preset a menu file FILE in Stage 2])
if test "x$enable_preset_menu" = x; then
  :
else
  if test -r $enable_preset_menu; then
    grub_DEFINE_FILE(PRESET_MENU_STRING, [$enable_preset_menu],
    		     [Define if there is user specified preset menu string])
  else
    AC_MSG_ERROR([Cannot read the preset menu file $enable_preset_menu])
  fi
fi

dnl Build the example Multiboot kernel.
AC_ARG_ENABLE(example-kernel,
  [  --enable-example-kernel
                          build the example Multiboot kernel])
AM_CONDITIONAL(BUILD_EXAMPLE_KERNEL, test "x$enable_example_kernel" = xyes)

dnl Automatic Linux mem= option.
AC_ARG_ENABLE(auto-linux-mem-opt,
  [  --disable-auto-linux-mem-opt
                          don't pass Linux mem= option automatically])
if test "x$enable_auto_linux_mem_opt" = xno; then
  :
else
  AC_DEFINE(AUTO_LINUX_MEM_OPT, 1, [Define if you don't want to pass the mem= option to Linux])
fi

dnl Now substitute the variables.
AC_SUBST(FSYS_CFLAGS)
AC_SUBST(NET_CFLAGS)
AC_SUBST(NET_EXTRAFLAGS)
AC_SUBST(NETBOOT_DRIVERS)

dnl Because recent automake complains about CCASFLAGS, set it here.
CCASFLAGS='$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)'
AC_SUBST(CCASFLAGS)


dnl Output.
if test "x$platform" = xefi; then
  AC_CONFIG_FILES([efi/Makefile])
  AC_CONFIG_LINKS([efi/grub/cpu:efi/grub/$host_cpu])
fi
AC_CONFIG_FILES([Makefile stage1/Makefile stage2/Makefile \
		 docs/Makefile lib/Makefile util/Makefile \
		 grub/Makefile netboot/Makefile util/grub-crypt \
		 util/grub-image util/grub-install util/grub-md5-crypt \
		 util/grub-set-default \
		 util/grub-terminfo])
AC_OUTPUT
