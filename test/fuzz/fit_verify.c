/*
 * (C) Copyright 2018 Facebook
 *
 * fit_verify: Attempt a FIT verification for fuzzing purposes.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "mkimage.h"
#include "fit_common.h"
#include <image.h>
#include <u-boot/crc.h>

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	if (Size < sizeof(struct fdt_header)) {
		return 0;
	}

	const char* fit = (const char*)Data;
	if (fdt_check_header(fit) != 0) {
		return 0;
	}

	if (Size != (size_t)fdt_totalsize(fit)) {
		return 0;
	}

	/* If your version is < 0x11 then you have no structure lengths. */
	if (fdt_version(fit) < 0x11) {
		return 0;
	}

	/* It is our job to sanity check the offsets. */
	int dt_struct = (int)fdt_off_dt_struct(fit);
	int dt_strings = (int)fdt_off_dt_strings(fit);
	if (dt_struct < 0 || dt_strings < 0) {
		return 0;
	}

	/* Same applies to the sizes and the potential for overflowing */
	int dt_struct_size = (int)fdt_size_dt_struct(fit);
	int dt_strings_size = (int)fdt_size_dt_strings(fit);
	if (dt_struct_size < 0 || dt_strings_size < 0) {
		return 0;
	}

	size_t total_struct = dt_struct + dt_struct_size;
	size_t total_strings = dt_strings + dt_strings_size;
	if (total_struct > Size || total_strings > Size) {
		return 0;
	}

	/* Node path to keys */
	int images_path = fdt_path_offset(fit, "/images");
	if (images_path < 0) {
		return 0;
	}

	/* After the first image (uboot) expect to find the subordinate store. */
	int subordinate = fdt_first_subnode(fit, images_path);
	if (subordinate < 0) {
		return 0;
	}

	/* Access the data and data-size to call image verify directly. */
	int data_size = 0;
	const char* data_prop = fdt_getprop(fit, subordinate, "data", &data_size);
	if (data_prop != 0 && data_size > 0) {
		/* This can return success if no keys were attempted. */
		int verified = 0;
		if (fit_image_verify_required_sigs(fit, subordinate, data_prop,
				data_size, fit, &verified)) {
			return 0;
		}
	}

	return 0;
}
