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
 * Boot selection (sboot N960FXXU9FVG2, sdb sha256 09d0dd96...):
 *
 * sboot does not boot from INFORM3 directly. Early in mach_board_main
 * (0x856e8), s5p_check_reboot_mode (0xd3a88) copies the mode nibble of
 * INFORM3 = 0x1234567X into the PARAM env REBOOT_MODE (index 0, saved to the
 * PARAM partition) and clears INFORM3. The boot choice then reads
 * REBOOT_MODE only: 4 = RECOVERY ("bootmode=2"), 0 = BOOT. "reboot recovery"
 * on 4.9 (INFORM3 = 0x12345674) therefore leaves REBOOT_MODE = 4 in flash,
 * and only a later reset that still carries INFORM3 = 0x12345670 into sboot
 * turns it back to 0.
 *
 * INFORM2/INFORM3 survive the PMU software reset (the diagnostic restart
 * returns to Android this way). A Power+VolDown force reset does not carry
 * them into sboot (with DEBUG_LEVEL LOW sboot configures the PMIC manual
 * reset as a cold cycle, set_mrstb 0x85628; observed RST_STAT = PINRESET
 * 0x10000, reset reason MPON), so sboot sees REBOOT_MODE = 4 and boots this
 * image again.
 *
 * So: always write the 4.9 sec_reboot normal values (INFORM2 0x12345678,
 * INFORM3 0x12345670). If this boot was not entered by a software reset
 * (RST_STAT bit 29 clear: force reset, cold power-on, key combo), do not run
 * the diagnostic; issue the PMU software reset that the 4.9 and diagnostic
 * restart paths use. sboot then takes INFORM3 = 0x12345670, resets
 * REBOOT_MODE to 0 itself (exactly as after a normal Android reboot) and
 * boots the normal BOOT partition. That reset is a software reset, so a
 * second pass through here never bounces again.
 *
 * Writes made here: INFORM2, INFORM3 and (bounce only) SWRESET. No storage.
 * The bounce is skipped, and the diagnostic boots as before, if sboot left
 * the DFD dump armed (RESET_SEQUENCER_CONFIGURATION bit 0) or a readback
 * fails.
 */
#define EXYNOS9810_PMU_BASE	0x14060000UL
#define PMU_SWRESET		0x400
#define PMU_RST_STAT		0x404
#define PMU_DFD			0x500	/* RESET_SEQUENCER_CONFIGURATION */
#define PMU_INFORM2		0x808
#define PMU_INFORM3		0x80c
#define RST_STAT_SWRESET	(1U << 29)
#define DFD_EDPCSR_DUMP_EN	(1U << 0)
#define SEC_POWER_RESET		0x12345678
#define SEC_RESET_REASON_UNKNOWN	0x12345670

static inline unsigned int pmu_read(unsigned long off)
{
	return *(volatile unsigned int *)(EXYNOS9810_PMU_BASE + off);
}

static inline void pmu_write(unsigned long off, unsigned int val)
{
	*(volatile unsigned int *)(EXYNOS9810_PMU_BASE + off) = val;
}

static int crownlte_normal_boot_reason(void)
{
	unsigned int rst_stat = pmu_read(PMU_RST_STAT);
	volatile unsigned long spin;

	pmu_write(PMU_INFORM2, SEC_POWER_RESET);
	pmu_write(PMU_INFORM3, SEC_RESET_REASON_UNKNOWN);

	if (rst_stat & RST_STAT_SWRESET)
		return 0;	/* entered by "reboot recovery": run the diagnostic */
	if (pmu_read(PMU_DFD) & DFD_EDPCSR_DUMP_EN)
		return 0;
	if (pmu_read(PMU_INFORM2) != SEC_POWER_RESET ||
	    pmu_read(PMU_INFORM3) != SEC_RESET_REASON_UNKNOWN)
		return 0;

	__asm__ volatile("dsb sy" ::: "memory");
	pmu_write(PMU_SWRESET, 1);
	__asm__ volatile("dsb sy" ::: "memory");

	/* The reset should take effect at once; if not, boot as before. */
	for (spin = 0; spin < 200000000UL; spin++)
		;
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
