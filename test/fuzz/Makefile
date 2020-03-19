#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

HOSTCFLAGS += -g -fsanitize=fuzzer,address -fsanitize-coverage=trace-cmp

hostprogs-y = fit_verify

srctree = ../..

FIT_SIG_OBJS-y := $(srctree)/common/image-sig.o

# The following files are synced with upstream DTC.
# Use synced versions from scripts/dtc/libfdt/.
LIBFDT_SRCS_SYNCED := fdt.c fdt_wip.c fdt_sw.c fdt_rw.c \
		fdt_strerror.c fdt_empty_tree.c fdt_addresses.c
# The following files are locally modified for U-Boot (unfotunately).
# Use U-Boot own versions from lib/libfdt/.
LIBFDT_SRCS_UNSYNCED := fdt_ro.c fdt_region.c

LIBFDT_OBJS := $(addprefix $(srctree)/lib/libfdt/, $(patsubst %.c, %.o, $(LIBFDT_SRCS_SYNCED))) \
	       $(addprefix $(srctree)/lib/libfdt/, $(patsubst %.c, %.o, $(LIBFDT_SRCS_UNSYNCED)))

RSA_OBJS-y := $(addprefix $(srctree)/lib/rsa/, \
					rsa-sign.o rsa-verify.o rsa-checksum.o \
					rsa-mod-exp.o)

# common objs for dumpimage and mkimage
dumpimage-mkimage-objs := \
			$(FIT_SIG_OBJS-y) \
			$(srctree)/common/bootm.o \
			$(srctree)/lib/crc32.o \
			$(srctree)/tools/default_image.o \
			$(srctree)/lib/fdtdec_common.o \
			$(srctree)/lib/fdtdec.o \
			$(srctree)/tools/fit_common.o \
			$(srctree)/tools/fit_image.o \
			$(srctree)/common/image-fit.o \
			$(srctree)/tools/image-host.o \
			$(srctree)/common/image.o \
			$(srctree)/tools/imagetool.o \
			$(srctree)/lib/md5.o \
			$(srctree)/tools/os_support.o \
			$(srctree)/lib/sha1.o \
			$(srctree)/lib/sha256.o \
			$(srctree)/common/hash.o \
			$(LIBFDT_OBJS) \
			$(RSA_OBJS-y)

HOSTCFLAGS_fit_image.o += -DMKIMAGE_DTC=\"$(CONFIG_MKIMAGE_DTC_PATH)\"

fit_verify-objs   := $(dumpimage-mkimage-objs) fit_verify.o

HOST_EXTRACFLAGS	+= -DCONFIG_FIT_SIGNATURE

HOSTLOADLIBES_mkimage += \
	$(shell pkg-config --libs libssl libcrypto 2> /dev/null || echo "-lssl -lcrypto")

HOSTLOADLIBES_fit_verify := $(HOSTLOADLIBES_mkimage) -fsanitize=fuzzer,address

clean-dirs := lib common

always := $(hostprogs-y)

#HOST_EXTRACFLAGS += -include $(srctree)/include/linux/libfdt_env.h \
		$(patsubst -I%,-idirafter%, $(filter -I%, $(UBOOTINCLUDE))) \
		-I$(srctree)/lib/libfdt \
		-I$(srctree)/include/linux \
		-I$(srctree)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES \
		-D_GNU_SOURCE

HOST_EXTRACFLAGS += -include ../include/compiler.h \
		$(patsubst -I%,-idirafter%, $(filter -I%, $(UBOOTINCLUDE))) \
		-Iscripts/dtc/libfdt \
		-Itools \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES \
		-D_GNU_SOURCE

clean-files += .strip
