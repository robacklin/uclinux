#ifndef __LINUX_SPI_MMC_SPI_H
#define __LINUX_SPI_MMC_SPI_H

#include <linux/spi/spi.h>
#include <linux/interrupt.h>

struct device;
struct mmc_host;

/* something to put in platform_data of a device being used
 * to manage an MMC/SD card slot
 *
 * REVISIT this isn't spi-specific, any card slot should be
 * able to handle it
 */
struct mmc_spi_platform_data {
	/* driver activation and (optional) card detect irq */
	int (*init)(struct device *,
		irqreturn_t (*)(int, void *),
		void *);
	void (*exit)(struct device *, void *);

	/* how long to debounce card detect, in jiffies */
	unsigned long detect_delay;

	/* sense switch on sd cards */
	int (*get_ro)(struct device *);

	/*
	 * If board does not use CD interrupts, driver can optimize polling
	 * using this function.
	 */
	int (*get_cd)(struct device *);

	/* Capabilities to pass into mmc core (e.g. MMC_CAP_NEEDS_POLL). */
	unsigned long caps;

	/* how long to debounce card detect, in msecs */
	u16 detect_delay;

	/* power management */
	unsigned int ocr_mask;			/* available voltages */
	void (*setpower)(struct device *, unsigned int);
};

#ifdef CONFIG_OF
extern struct mmc_spi_platform_data *mmc_spi_get_pdata(struct spi_device *spi);
extern void mmc_spi_put_pdata(struct spi_device *spi);
#else
static inline struct mmc_spi_platform_data *
mmc_spi_get_pdata(struct spi_device *spi)
{
	return spi->dev.platform_data;
}
static inline void mmc_spi_put_pdata(struct spi_device *spi) {}
#endif /* CONFIG_OF */

#endif /* __LINUX_SPI_MMC_SPI_H */
