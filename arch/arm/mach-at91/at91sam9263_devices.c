/*
 * arch/arm/mach-at91/at91sam9263_devices.c
 *
 *  Copyright (C) 2006 Atmel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include <common.h>
#include <sizes.h>
#include <asm/armlinux.h>
#include <asm/hardware.h>
#include <mach/at91_pmc.h>
#include <mach/at91sam9263_matrix.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/io.h>

#include "generic.h"

void at91_add_device_sdram(u32 size)
{
	arm_add_mem_device("ram0", AT91_CHIPSELECT_1, size);
	add_mem_device("sram0", AT91SAM9263_SRAM0_BASE,
			AT91SAM9263_SRAM0_SIZE, IORESOURCE_MEM_WRITEABLE);
	add_mem_device("sram1", AT91SAM9263_SRAM1_BASE,
			AT91SAM9263_SRAM1_SIZE, IORESOURCE_MEM_WRITEABLE);
}

/* --------------------------------------------------------------------
 *  USB Host
 * -------------------------------------------------------------------- */

#if defined(CONFIG_USB_OHCI)
void __init at91_add_device_usbh_ohci(struct at91_usbh_data *data)
{
	int i;

	if (!data)
		return;

	/* Enable VBus control for UHP ports */
	for (i = 0; i < data->ports; i++) {
		if (data->vbus_pin[i])
			at91_set_gpio_output(data->vbus_pin[i], 0);
	}

	add_generic_device("at91_ohci", -1, NULL, AT91SAM9263_UHP_BASE, 1024 * 1024,
			   IORESOURCE_MEM, data);
}
#else
void __init at91_add_device_usbh_ohci(struct at91_usbh_data *data) {}
#endif

/* --------------------------------------------------------------------
 *  USB Device (Gadget)
 * -------------------------------------------------------------------- */

#ifdef CONFIG_USB_GADGET_DRIVER_AT91
void __init at91_add_device_udc(struct at91_udc_data *data)
{
	if (data->vbus_pin > 0) {
		at91_set_gpio_input(data->vbus_pin, 0);
		at91_set_deglitch(data->vbus_pin, 1);
	}

	add_generic_device("at91_udc", -1, NULL, AT91SAM9263_BASE_UDP, SZ_16K,
			   IORESOURCE_MEM, data);
}
#else
void __init at91_add_device_udc(struct at91_udc_data *data) {}
#endif

#if defined(CONFIG_DRIVER_NET_MACB)
void at91_add_device_eth(int id, struct at91_ether_platform_data *data)
{
	if (!data)
		return;

	at91_set_A_periph(AT91_PIN_PE21, 0);	/* ETXCK_EREFCK */
	at91_set_B_periph(AT91_PIN_PC25, 0);	/* ERXDV */
	at91_set_A_periph(AT91_PIN_PE25, 0);	/* ERX0 */
	at91_set_A_periph(AT91_PIN_PE26, 0);	/* ERX1 */
	at91_set_A_periph(AT91_PIN_PE27, 0);	/* ERXER */
	at91_set_A_periph(AT91_PIN_PE28, 0);	/* ETXEN */
	at91_set_A_periph(AT91_PIN_PE23, 0);	/* ETX0 */
	at91_set_A_periph(AT91_PIN_PE24, 0);	/* ETX1 */
	at91_set_A_periph(AT91_PIN_PE30, 0);	/* EMDIO */
	at91_set_A_periph(AT91_PIN_PE29, 0);	/* EMDC */

	if (!(data->flags & AT91SAM_ETHER_RMII)) {
		at91_set_A_periph(AT91_PIN_PE22, 0);	/* ECRS */
		at91_set_B_periph(AT91_PIN_PC26, 0);	/* ECOL */
		at91_set_B_periph(AT91_PIN_PC22, 0);	/* ERX2 */
		at91_set_B_periph(AT91_PIN_PC23, 0);	/* ERX3 */
		at91_set_B_periph(AT91_PIN_PC27, 0);	/* ERXCK */
		at91_set_B_periph(AT91_PIN_PC20, 0);	/* ETX2 */
		at91_set_B_periph(AT91_PIN_PC21, 0);	/* ETX3 */
		at91_set_B_periph(AT91_PIN_PC24, 0);	/* ETXER */
	}

	add_generic_device("macb", 0, NULL, AT91SAM9263_BASE_EMAC, 0x1000,
			   IORESOURCE_MEM, data);
}
#else
void at91_add_device_eth(int id, struct at91_ether_platform_data *data) {}
#endif

#if defined(CONFIG_NAND_ATMEL)
static struct resource nand_resources[] = {
	[0] = {
		.start	= AT91_CHIPSELECT_3,
		.size	= SZ_256M,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91_BASE_SYS + AT91_ECC0,
		.size	= 512,
		.flags	= IORESOURCE_MEM,
	}
};

void at91_add_device_nand(struct atmel_nand_data *data)
{
	unsigned long csa;

	if (!data)
		return;

	csa = at91_sys_read(AT91_MATRIX_EBI0CSA);
	at91_sys_write(AT91_MATRIX_EBI0CSA, csa | AT91_MATRIX_EBI0_CS3A_SMC_SMARTMEDIA);

	/* enable pin */
	if (data->enable_pin)
		at91_set_gpio_output(data->enable_pin, 1);

	/* ready/busy pin */
	if (data->rdy_pin)
		at91_set_gpio_input(data->rdy_pin, 1);

	/* card detect pin */
	if (data->det_pin)
		at91_set_gpio_input(data->det_pin, 1);

	add_generic_device_res("atmel_nand", -1, nand_resources,
			       ARRAY_SIZE(nand_resources), data);
}
#else
void at91_add_device_nand(struct atmel_nand_data *data) {}
#endif

/* --------------------------------------------------------------------
 *  SPI
 * -------------------------------------------------------------------- */

#if defined(CONFIG_DRIVER_SPI_ATMEL)
static int spi0_standard_cs[4] = { AT91_PIN_PA5, AT91_PIN_PA3, AT91_PIN_PA4, AT91_PIN_PB11 };

static int spi1_standard_cs[4] = { AT91_PIN_PB15, AT91_PIN_PB16, AT91_PIN_PB17, AT91_PIN_PB18 };

static struct at91_spi_platform_data spi_pdata[] = {
	[0] = {
		.chipselect = spi0_standard_cs,
		.num_chipselect = ARRAY_SIZE(spi0_standard_cs),
	},
	[1] = {
		.chipselect = spi1_standard_cs,
		.num_chipselect = ARRAY_SIZE(spi1_standard_cs),
	},
};

void at91_add_device_spi(int spi_id, struct at91_spi_platform_data *pdata)
{
	int i;
	int cs_pin;
	resource_size_t start = ~0;

	BUG_ON(spi_id > 1);

	if (!pdata)
		pdata = &spi_pdata[spi_id];

	for (i = 0; i < pdata->num_chipselect; i++) {
		cs_pin = pdata->chipselect[i];

		/* enable chip-select pin */
		if (cs_pin > 0)
			at91_set_gpio_output(cs_pin, 1);
	}

	/* Configure SPI bus(es) */
	switch (spi_id) {
	case 0:
		start = AT91SAM9263_BASE_SPI0;
		at91_set_B_periph(AT91_PIN_PA0, 0);	/* SPI0_MISO */
		at91_set_B_periph(AT91_PIN_PA1, 0);	/* SPI0_MOSI */
		at91_set_B_periph(AT91_PIN_PA2, 0);	/* SPI0_SPCK */
		break;
	case 1:
		start = AT91SAM9263_BASE_SPI1;
		at91_set_A_periph(AT91_PIN_PB12, 0);	/* SPI1_MISO */
		at91_set_A_periph(AT91_PIN_PB13, 0);	/* SPI1_MOSI */
		at91_set_A_periph(AT91_PIN_PB14, 0);	/* SPI1_SPCK */
		break;
	}

	add_generic_device("atmel_spi", spi_id, NULL, start, SZ_16K,
			   IORESOURCE_MEM, pdata);
}
#else
void __init at91_add_device_spi(int spi_id, struct at91_spi_platform_data *pdata) {}
#endif

static inline void configure_dbgu_pins(void)
{
	at91_set_A_periph(AT91_PIN_PC30, 0);		/* DRXD */
	at91_set_A_periph(AT91_PIN_PC31, 1);		/* DTXD */
}

static inline void configure_usart0_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PA26, 1);		/* TXD0 */
	at91_set_A_periph(AT91_PIN_PA27, 0);		/* RXD0 */

	if (pins & ATMEL_UART_RTS)
		at91_set_A_periph(AT91_PIN_PA28, 0);	/* RTS0 */
	if (pins & ATMEL_UART_CTS)
		at91_set_A_periph(AT91_PIN_PA29, 0);	/* CTS0 */
}

static inline void configure_usart1_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PD0, 1);		/* TXD1 */
	at91_set_A_periph(AT91_PIN_PD1, 0);		/* RXD1 */

	if (pins & ATMEL_UART_RTS)
		at91_set_B_periph(AT91_PIN_PD7, 0);	/* RTS1 */
	if (pins & ATMEL_UART_CTS)
		at91_set_B_periph(AT91_PIN_PD8, 0);	/* CTS1 */
}

static inline void configure_usart2_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PD2, 1);		/* TXD2 */
	at91_set_A_periph(AT91_PIN_PD3, 0);		/* RXD2 */

	if (pins & ATMEL_UART_RTS)
		at91_set_B_periph(AT91_PIN_PD5, 0);	/* RTS2 */
	if (pins & ATMEL_UART_CTS)
		at91_set_B_periph(AT91_PIN_PD6, 0);	/* CTS2 */
}

struct device_d * __init at91_register_uart(unsigned id, unsigned pins)
{
	resource_size_t start;

	switch (id) {
		case 0:		/* DBGU */
			configure_dbgu_pins();
			start = AT91_BASE_SYS + AT91_DBGU;
			break;
		case 1:
			configure_usart0_pins(pins);
			start = AT91SAM9263_BASE_US0;
			break;
		case 2:
			configure_usart1_pins(pins);
			start = AT91SAM9263_BASE_US1;
			break;
		case 3:
			configure_usart2_pins(pins);
			start = AT91SAM9263_BASE_US2;
			break;
		default:
			return NULL;
	}

	return add_generic_device("atmel_usart", id, NULL, start, 4096,
			   IORESOURCE_MEM, NULL);
}

#if defined(CONFIG_MCI_ATMEL)
/* Consider only one slot : slot 0 */
void at91_add_device_mci(short mmc_id, struct atmel_mci_platform_data *data)
{
	resource_size_t start;

	if (!data)
		return;

	/* need bus_width */
	if (!data->bus_width)
		return;

	/* input/irq */
	if (data->detect_pin) {
		at91_set_gpio_input(data->detect_pin, 1);
		at91_set_deglitch(data->detect_pin, 1);
	}

	if (data->wp_pin)
		at91_set_gpio_input(data->wp_pin, 1);

	if (mmc_id == 0) {		/* MCI0 */
		start = AT91SAM9263_BASE_MCI0;

		/* CLK */
		at91_set_A_periph(AT91_PIN_PA12, 0);

		if (data->slot_b) {
			/* CMD */
			at91_set_A_periph(AT91_PIN_PA16, 1);

			/* DAT0, maybe DAT1..DAT3 */
			at91_set_A_periph(AT91_PIN_PA17, 1);
			if (data->bus_width == 4) {
				at91_set_A_periph(AT91_PIN_PA18, 1);
				at91_set_A_periph(AT91_PIN_PA19, 1);
				at91_set_A_periph(AT91_PIN_PA20, 1);
			}
		} else {
			/* CMD */
			at91_set_A_periph(AT91_PIN_PA1, 1);

			/* DAT0, maybe DAT1..DAT3 */
			at91_set_A_periph(AT91_PIN_PA0, 1);
			if (data->bus_width == 4) {
				at91_set_A_periph(AT91_PIN_PA3, 1);
				at91_set_A_periph(AT91_PIN_PA4, 1);
				at91_set_A_periph(AT91_PIN_PA5, 1);
			}
		}
	} else {			/* MCI1 */
		start = AT91SAM9263_BASE_MCI1;

		if (data->slot_b) {
			/* CMD */
			at91_set_A_periph(AT91_PIN_PA21, 1);

			/* DAT0, maybe DAT1..DAT3 */
			at91_set_A_periph(AT91_PIN_PA22, 1);
			if (data->bus_width == 4) {
				at91_set_A_periph(AT91_PIN_PA23, 1);
				at91_set_A_periph(AT91_PIN_PA24, 1);
				at91_set_A_periph(AT91_PIN_PA25, 1);
			}
		} else {
			/* CMD */
			at91_set_A_periph(AT91_PIN_PA7, 1);

			/* DAT0, maybe DAT1..DAT3 */
			at91_set_A_periph(AT91_PIN_PA8, 1);
			if (data->bus_width == 4) {
				at91_set_A_periph(AT91_PIN_PA9, 1);
				at91_set_A_periph(AT91_PIN_PA10, 1);
				at91_set_A_periph(AT91_PIN_PA11, 1);
			}
		}
	}

	add_generic_device("atmel_mci", mmc_id, NULL, start, 4096,
			   IORESOURCE_MEM, data);
}
#else
void at91_add_device_mci(short mmc_id, struct atmel_mci_platform_data *data) {}
#endif

