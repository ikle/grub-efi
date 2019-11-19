/*
 * Integrity Checker
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <shared.h>

static const struct hash *hash;
static char expected[64];

static int hex2int (int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;

	errnum = ERR_NUMBER_PARSING;
	return -1;
}

static int read_octet (const char *from, void *to)
{
	unsigned char *p = to;
	int h, l;

	if ((h = hex2int (from[0])) < 0)
		return 0;

	if ((l = hex2int (from[1])) < 0)
		return 0;

	*p = (h << 4) | l;
	return 1;
}

static int read_hash (const char *from)
{
	char *p;
	unsigned count;

	if (hash->len > sizeof (expected)) {
		errnum = ERR_WONT_FIT;
		return 0;
	}

	for (
		p = expected, count = hash->len;
		count > 0;
		from += 2, ++p, --count
	) {
		for (; *from == ' ' || *from == '\t'; ++from) {}

		if (!read_octet (from, p))
			return 0;
	}

	for (; *from == ' ' || *from == '\t'; ++from) {}

	if (*from != '\0') {
		errnum = ERR_BAD_ARGUMENT;
		return 0;
	}

	return 1;
}

int check_init (const char *conf)
{
	if (grub_memcmp (conf, "md5", 3) == 0) {
		hash = &md5_hash;
		return read_hash (conf + 3);
	}

	if (grub_memcmp (conf, "sha256", 6) == 0) {
		hash = &sha256_hash;
		return read_hash (conf + 6);
	}

	if (grub_memcmp (conf, "sha512", 6) == 0) {
		hash = &sha512_hash;
		return read_hash (conf + 6);
	}

	errnum = ERR_BAD_ARGUMENT;
	return 0;
}

int check_file (const char *filename)
{
	char buf[512];
	int len;
	const char *result;

	if (hash == NULL)
		return 1;

	if (!grub_open ((char *) filename))
		return 0;

	hash->init ();

	while ((len = grub_read (buf, sizeof (buf))) > 0)
		hash->update (buf, len);

	result = hash->finish ();
	grub_close ();

	if (grub_memcmp (expected, result, hash->len) != 0) {
		errnum = ERR_BAD_CHECKSUM;
		hash = NULL;
		return 0;
	}

	hash = NULL;
	return 1;
}
