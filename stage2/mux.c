/*
 * Terminal Multiplexer
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <shared.h>
#include <term.h>

#define term_foreach(i, func)					\
	for ((i) = 0; term_table[i].name != NULL; ++(i))	\
		if (current_term_map & (1 << (i)) &&		\
		    term_table[i].func != NULL)

static void mux_putchar (int c)
{
	unsigned i;

	term_foreach (i, putchar)
		term_table[i].putchar (c);
}

static int mux_checkkey (void)
{
	unsigned i;
	int status;

	term_foreach (i, checkkey)
		if ((status = term_table[i].checkkey ()) >= 0)
			return status;

	return -1;
}

static int mux_getkey (void)
{
	unsigned i;

	for (;;)
		term_foreach (i, checkkey)
			if (term_table[i].checkkey () >= 0)
				return term_table[i].getkey ();

	return -1;  /* never reached */
}

static int mux_keystatus (void)
{
	unsigned i;
	int status = 0;

	term_foreach (i, keystatus)
		status |= term_table[i].keystatus ();

	return status;
}

static int mux_getxy (void)
{
	unsigned i;

	term_foreach (i, getxy)
		/* return from first available terminal */
		return term_table[i].getxy ();

	return 0;
}

static void mux_gotoxy (int x, int y)
{
	unsigned i;

	term_foreach (i, gotoxy)
		term_table[i].gotoxy (x, y);
}

static void mux_cls (void)
{
	unsigned i;

	term_foreach (i, cls)
		term_table[i].cls ();
}

static void mux_setcolorstate (color_state state)
{
	unsigned i;

	term_foreach (i, setcolorstate)
		term_table[i].setcolorstate (state);
}

static void mux_setcolor (int normal_color, int highlight_color)
{
	unsigned i;

	term_foreach (i, setcolor)
		term_table[i].setcolor (normal_color, highlight_color);
}

static int mux_setcursor (int on)
{
	unsigned i;

	term_foreach (i, setcursor)
		term_table[i].setcursor (on);

	return on;
}

static int mux_startup (void)
{
	unsigned i, count = 0;

	term_foreach (i, putchar) {
		if (term_table[i].startup == NULL || term_table[i].startup ())
			++count;
		else
			/* if terminal fails to initialize, disable it */
			current_term_map &= ~(1 << i);
	}

	return count > 0;
}

static void mux_shutdown (void)
{
	unsigned i;

	term_foreach (i, shutdown)
		term_table[i].shutdown ();
}

struct term_entry mux_term = {
	"mux",
	0,
	24,
	mux_putchar,
	mux_checkkey,
	mux_getkey,
	mux_keystatus,
	mux_getxy,
	mux_gotoxy,
	mux_cls,
	mux_setcolorstate,
	mux_setcolor,
	mux_setcursor,
	mux_startup,
	mux_shutdown
};
