#include <common.h>
#include <init.h>
#include <driver.h>
#include <asm/cpu/cdefBF561.h>
#include <partition.h>
#include <fs.h>

static int ipe337_devices_init(void) {
	add_cfi_flash_device(-1, 0x20000000, 32 * 1024 * 1024, 0);
	add_mem_device("ram0", 0x0, 128 * 1024 * 1024,
		       IORESOURCE_MEM_WRITEABLE);

	/* Reset smc911x */
	*pFIO0_DIR = (1<<12);
	*pFIO0_FLAG_C = (1<<12);
	mdelay(100);
	*pFIO0_FLAG_S = (1<<12);

	add_generic_device("smc911x", -1, NULL, 0x24000000, 4096,
			   IORESOURCE_MEM, NULL);

	devfs_add_partition("nor0", 0x00000, 0x20000, PARTITION_FIXED, "self0");
	devfs_add_partition("nor0", 0x20000, 0x20000, PARTITION_FIXED, "env0");

	protect_file("/dev/env0", 1);

	return 0;
}

device_initcall(ipe337_devices_init);

static int blackfin_console_init(void)
{
	add_generic_device("blackfin_serial", -1, NULL, 0, 4096,
			   IORESOURCE_MEM, NULL);

	return 0;
}

console_initcall(blackfin_console_init);

