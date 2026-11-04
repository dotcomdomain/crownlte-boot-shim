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
/*
 * Boot-selection reset: sboot picks the next boot from PMU INFORM2/INFORM3
 * and does not clear them. A diagnostic image is entered with "reboot
 * recovery" (INFORM3 = 0x12345674), so if it hangs, every reset (including a
 * Power+VolDown force reset) boots the same image again. Write the 4.9
 * sec_reboot normal-boot values first thing, so any later reset returns to
 * the normal BOOT partition. This is the only register write made here.
 */
#define EXYNOS9810_PMU_BASE	0x14060000UL
#define PMU_INFORM2		0x808
#define PMU_INFORM3		0x80c
#define SEC_POWER_RESET		0x12345678
#define SEC_RESET_REASON_UNKNOWN	0x12345670

static int crownlte_normal_boot_reason(void)
{
	*(volatile unsigned int *)(EXYNOS9810_PMU_BASE + PMU_INFORM2) = SEC_POWER_RESET;
	*(volatile unsigned int *)(EXYNOS9810_PMU_BASE + PMU_INFORM3) = SEC_RESET_REASON_UNKNOWN;
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-crownlte-display-free",
	.ops = {
		.early_init = crownlte_normal_boot_reason,
	},
	.devices = NULL,
	.num_devices = 0,
	.quirks = 0,
};
#endif
