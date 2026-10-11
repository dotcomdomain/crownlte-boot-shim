/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_CROWNLTE_FRAMEBUFFER_CONSOLE
#define DECON_F_BASE		0x16030000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int crownlte_init(void)
{
	/* Shared Exynos9810 DECON trigger. Scanout dimensions need live validation. */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info crownlte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 2960,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xcc000000
};

static const struct device crownlte_devices[] = {
	{ "simplefb", &crownlte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-crownlte",
	.ops = {
		.early_init = crownlte_init,
	},
	.devices = crownlte_devices,
	.num_devices = ARRAY_SIZE(crownlte_devices),
	.quirks = 0
};

#else
/* No register or framebuffer writes while the initial CPU mapping is unknown. */
struct board_data board_ops = {
	.name = "samsung-crownlte-display-free",
	.devices = NULL,
	.num_devices = 0,
	.quirks = 0,
};
#endif
