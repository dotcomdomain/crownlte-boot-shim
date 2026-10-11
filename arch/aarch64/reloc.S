/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include "asmdefs.h"

#ifdef CONFIG_LINUX_KRNL_HEADER_IMG
#include "linux-kernel-image-header.h"
#endif

.extern _reloc_start
.extern _reloc_file_size
.extern _start

ENTRY_ALIAS(_head)
#ifdef CONFIG_LINUX_KRNL_HEADER_IMG
	linux_image_header _reloc_entry
#else
	adr	x1, _relocated_uni
	b	 _reloc_entry
#endif

ENTRY_ALIAS(_rel_copy_base)
	.quad _reloc_start

ENTRY_ALIAS(_rel_entry_base)
	.quad _start

ENTRY_ALIAS(_rel_copy_size)
	.quad _reloc_file_size

ENTRY_ALIAS( _reloc_entry)
	/* Copy the complete raw file, including this header, but never BSS or
	 * stack. A partial overlap could overwrite this running copy loop. */
	adr	x4, _head
	ldr	x5, _rel_copy_base
	ldr	x6, _rel_copy_size
	cbz	x6, _dead
	tst	x6, #15
	b.ne	_dead
	tst	x4, #15
	b.ne	_dead
	tst	x5, #15
	b.ne	_dead
	adds	x7, x4, x6
	b.cs	_dead
	adds	x8, x5, x6
	b.cs	_dead
	cmp	x4, x5
	b.eq	_entry
	/* Half-open intervals are disjoint if src >= dst_end or dst >= src_end. */
	cmp	x4, x8
	b.hs	_copy
	cmp	x5, x7
	b.lo	_dead

_copy:
	ldp	x2, x3, [x4], #16
	stp	x2, x3, [x5], #16
	subs	x6, x6, #16
	b.ne	_copy

_entry:
	ldr	x5, _rel_entry_base
	br	x5

_dead: /* No stores or kernel handoff after an invalid relocation interval. */
	b	_dead

ENTRY_ALIAS(_relocated_uni)
