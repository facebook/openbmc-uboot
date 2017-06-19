/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <image.h>
#include <time.h>
#else
#include <common.h>
#include <errno.h>
#include <mapmem.h>
#include <asm/io.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/

#include <u-boot/crc.h>
#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>

static void fit_get_debug(const void *fit, int noffset,
		char *prop_name, int err)
{
	debug("Can't get '%s' property from FIT 0x%08lx, node: offset %d, name %s (%s)\n",
	      prop_name, (ulong)fit, noffset, fit_get_name(fit, noffset, NULL),
	      fdt_strerror(err));
}

/**
 * calculate_hash - calculate and return hash for provided input data
 * @data: pointer to the input data
 * @data_len: data length
 * @algo: requested hash algorithm
 * @value: pointer to the char, will hold hash value data (caller must
 * allocate enough free space)
 * value_len: length of the calculated hash
 *
 * calculate_hash() computes input data hash according to the requested
 * algorithm.
 * Resulting hash value is placed in caller provided 'value' buffer, length
 * of the calculated hash is returned via value_len pointer argument.
 *
 * returns:
 *     0, on success
 *    -1, when algo is unsupported
 */
int calculate_hash(const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len)
{
	if (IMAGE_ENABLE_CRC32 && strcmp(algo, "crc32") == 0) {
		*((uint32_t *)value) = crc32_wd(0, data, data_len,
							CHUNKSZ_CRC32);
		*((uint32_t *)value) = cpu_to_uimage(*((uint32_t *)value));
		*value_len = 4;
	} else if (IMAGE_ENABLE_SHA1 && strcmp(algo, "sha1") == 0) {
		sha1_csum_wd((unsigned char *)data, data_len,
			     (unsigned char *)value, CHUNKSZ_SHA1);
		*value_len = 20;
	} else if (IMAGE_ENABLE_SHA256 && strcmp(algo, "sha256") == 0) {
		sha256_csum_wd((unsigned char *)data, data_len,
			       (unsigned char *)value, CHUNKSZ_SHA256);
		*value_len = SHA256_SUM_LEN;
	} else if (IMAGE_ENABLE_MD5 && strcmp(algo, "md5") == 0) {
		md5_wd((unsigned char *)data, data_len, value, CHUNKSZ_MD5);
		*value_len = 16;
	} else {
		debug("Unsupported hash alogrithm\n");
		return -1;
	}
	return 0;
}

/**
 * fit_image_hash_get_algo - get hash algorithm name
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @algo: double pointer to char, will hold pointer to the algorithm name
 *
 * fit_image_hash_get_algo() finds hash algorithm property in a given hash node.
 * If the property is found its data start address is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_algo(const void *fit, int noffset, char **algo)
{
	int len;

	*algo = (char *)fdt_getprop(fit, noffset, FIT_ALGO_PROP, &len);
	if (*algo == NULL) {
		fit_get_debug(fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_image_hash_get_value - get hash value and length
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: double pointer to uint8_t, will hold address of a hash value data
 * @value_len: pointer to an int, will hold hash data length
 *
 * fit_image_hash_get_value() finds hash value property in a given hash node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len)
{
	int len;

	*value = (uint8_t *)fdt_getprop(fit, noffset, FIT_VALUE_PROP, &len);
	if (*value == NULL) {
		fit_get_debug(fit, noffset, FIT_VALUE_PROP, len);
		*value_len = 0;
		return -1;
	}

	*value_len = len;
	return 0;
}

/**
 * fit_image_hash_get_ignore - get hash ignore flag
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @ignore: pointer to an int, will hold hash ignore flag
 *
 * fit_image_hash_get_ignore() finds hash ignore property in a given hash node.
 * If the property is found and non-zero, the hash algorithm is not verified by
 * u-boot automatically.
 *
 * returns:
 *     0, on ignore not found
 *     value, on ignore found
 */
static int fit_image_hash_get_ignore(const void *fit, int noffset, int *ignore)
{
	int len;
	int *value;

	value = (int *)fdt_getprop(fit, noffset, FIT_IGNORE_PROP, &len);
	if (value == NULL || len != sizeof(int))
		*ignore = 0;
	else
		*ignore = *value;

	return 0;
}

int fit_image_check_hash(const void *fit, int noffset, const void *data,
				size_t size, uint8_t *value, char **err_msgp)
{
	int value_len;
	char *algo;
	uint8_t *fit_value;
	int fit_value_len;
	int ignore;

	*err_msgp = NULL;

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}
	printf("%s", algo);

	if (IMAGE_ENABLE_IGNORE) {
		fit_image_hash_get_ignore(fit, noffset, &ignore);
		if (ignore) {
			printf("-skipped ");
			return 0;
		}
	}

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		*err_msgp = "Unsupported hash algorithm";
		return -1;
	}

	if (value_len != fit_value_len) {
		*err_msgp = "Bad hash value len";
		return -1;
	} else if (memcmp(value, fit_value, value_len) != 0) {
		*err_msgp = "Bad hash value";
		return -1;
	}

	return 0;
}
