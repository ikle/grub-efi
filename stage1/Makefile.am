pkgdatadir = $(datadir)/$(PACKAGE)/$(host_cpu)-$(host_vendor)

AM_CCASFLAGS = $(STAGE1_CFLAGS)
LDFLAGS = $(LOADER_LDFLAGS) -Wl,-N,-Ttext,7C00

if !PLATFORM_EFI

nodist_pkgdata_DATA = stage1

CLEANFILES = $(nodist_pkgdata_DATA)

noinst_PROGRAMS = stage1.exec
stage1_exec_SOURCES = stage1.S stage1.h

SUFFIXES = .exec
.exec:
	$(OBJCOPY) -O binary -R .note -R .comment -R .note.gnu.build-id $< $@

endif
