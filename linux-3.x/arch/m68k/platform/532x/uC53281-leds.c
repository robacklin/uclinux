/*
 * arch/m68knommu/platform/uC53281-leds.c
 *
 * Copyright (c) 2008 Arturus Networks Inc.
 *               by Oleksandr G Zhadan <www.ArcturusNetworks.com>
 *
 * arch dependent part of the Arcturus Networks uC53281 module LED driver
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/traps.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/mcftimer.h>
#include <asm/mcfsim.h>
#include <asm/mcfdma.h>
#include <asm/mcfwdebug.h>
#include <asm/leds.h>

static void dummy_leds_event(led_event_t evt)
{
}

void (*leds_event) (led_event_t) = dummy_leds_event;

#define UC53281_LED2		0x02
#define UC53281_LED3		0x04
#define UC53281_LED4		0x08
#define UC53281_LED5		0x10

static DEFINE_SPINLOCK(leds_lock);

/* To mask/unmask specific LEDs */

#define UC53281_LEDS_MASK	(UC53281_LED5)
#define UC53281_LEDS_UNMASK	(UC53281_LED2|UC53281_LED3|UC53281_LED4)

/*
    Small trick of the active_low usage:
     active_low will set/show the gpio direction pin value,
     0 - input/buttons , 1 - output/leds
*/

static struct gpio_led uC53281_gpio_led[] = {
	{
	 .name = "led2",
	 .gpio = 0x02,
	 .active_low = (UC53281_LEDS_MASK & UC53281_LED2),
	 },
	{
	 .name = "led3",
	 .gpio = 0x04,
	 .active_low = (UC53281_LEDS_MASK & UC53281_LED3),
	 },
	{
	 .name = "led4",
	 .gpio = 0x08,
	 .active_low = (UC53281_LEDS_MASK & UC53281_LED4),
	 },
	{
	 .name = "led5",
	 .gpio = 0x10,
	 .active_low = (UC53281_LEDS_MASK & UC53281_LED5),
	 },
};

static struct gpio_led_platform_data uC53281_gpio_led_platform_data = {
	.num_leds = ARRAY_SIZE(uC53281_gpio_led),
	.leds = uC53281_gpio_led,
};

static struct platform_device uC53281_gpio_led_platform_device = {
	.name = "uC53281:leds",
	.id = -1,
	.dev = {
		.platform_data = &uC53281_gpio_led_platform_data,
		}
};

static struct platform_device *uC53281_led_devices[] __initdata = {
	&uC53281_gpio_led_platform_device,
};

asmlinkage void uC53281_gpio_data_pin_set(unsigned char pin, int set)
{
	unsigned int flags;

	spin_lock_irqsave(&leds_lock, flags);
	if (set)
		MCF_EPORT_EPDR |= pin;
	else
		MCF_EPORT_EPDR &= ~pin;
	spin_unlock_irqrestore(&leds_lock, flags);
}

asmlinkage void uC53281_gpio_direction_pin_set(unsigned char pin, int set)
{
	unsigned int flags;

	spin_lock_irqsave(&leds_lock, flags);
	if (set)
		MCF_EPORT_EPDDR |= pin;
	else
		MCF_EPORT_EPDDR &= ~pin;
	spin_unlock_irqrestore(&leds_lock, flags);
}

void uC53281_gpio_led_pin_set_bn(unsigned char *name, int set)
{
	int i;
	for (i = 0; i < uC53281_gpio_led_platform_data.num_leds; i++) {
		if (!strcmp(uC53281_gpio_led[i].name, name)) {
			if (uC53281_gpio_led[i].active_low) {
				if (set == 'I') {	/* configure gpio pin as input */
					uC53281_gpio_led[i].active_low = 0;
					uC53281_gpio_direction_pin_set
					    (uC53281_gpio_led[i].gpio, 0);
				} else {
					uC53281_gpio_data_pin_set
					    (uC53281_gpio_led[i].gpio, set);
				}
			} else {
				if (set == 'O') {	/* configure gpio pin as output */
					uC53281_gpio_led[i].active_low = 1;
					uC53281_gpio_direction_pin_set
					    (uC53281_gpio_led[i].gpio, 1);
					uC53281_gpio_data_pin_set
					    (uC53281_gpio_led[i].gpio, set);
				}
			}
			break;
		}
	}
}

static void uC53281_leds_event(led_event_t evt)
{
	unsigned int flags;

	spin_lock_irqsave(&leds_lock, flags);

	switch (evt) {

	case led_led2_init:
		MCF_EPORT_EPDDR |= UC53281_LED2;
		break;
	case led_led2_exit:
		MCF_EPORT_EPDDR &= ~UC53281_LED2;
		break;
	case led_led2_on:
		MCF_EPORT_EPDR |= UC53281_LED2;
		break;
	case led_led2_off:
		MCF_EPORT_EPDR &= ~UC53281_LED2;
		break;
	case led_led3_init:
		MCF_EPORT_EPDDR |= UC53281_LED3;
		break;
	case led_led3_exit:
		MCF_EPORT_EPDDR &= ~UC53281_LED3;
		break;
	case led_led3_on:
		MCF_EPORT_EPDR |= UC53281_LED3;
		break;
	case led_led3_off:
		MCF_EPORT_EPDR &= ~UC53281_LED3;
		break;
	case led_led4_init:
		MCF_EPORT_EPDDR |= UC53281_LED4;
		break;
	case led_led4_exit:
		MCF_EPORT_EPDDR &= ~UC53281_LED4;
		break;
	case led_led4_on:
		MCF_EPORT_EPDR |= UC53281_LED4;
		break;
	case led_led4_off:
		MCF_EPORT_EPDR &= ~UC53281_LED4;
		break;
	case led_led5_init:
		MCF_EPORT_EPDDR |= UC53281_LED5;
		break;
	case led_led5_exit:
		MCF_EPORT_EPDDR &= ~UC53281_LED5;
		break;
	case led_led5_on:
		MCF_EPORT_EPDR |= UC53281_LED5;
		break;
	case led_led5_off:
		MCF_EPORT_EPDR &= ~UC53281_LED5;
		break;
	case led_panic:
		MCF_EPORT_EPDDR |=
		    (UC53281_LED2 | UC53281_LED3 | UC53281_LED4 | UC53281_LED5);
		MCF_EPORT_EPDR |=
		    (UC53281_LED2 | UC53281_LED3 | UC53281_LED4 | UC53281_LED5);
		mdelay(500);
		MCF_EPORT_EPDR &=
		    ~(UC53281_LED2 | UC53281_LED3 | UC53281_LED4 |
		      UC53281_LED5);
		mdelay(499);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&leds_lock, flags);
}

static int __init uC53281_platform_add_led_devs(void)
{
	extern void (*leds_event) (led_event_t);

	leds_event = uC53281_leds_event;

	leds_event(led_led2_exit);
	leds_event(led_led3_exit);
	leds_event(led_led4_exit);
	leds_event(led_led5_init);
	leds_event(led_led5_on);

	return platform_add_devices(uC53281_led_devices,
				    ARRAY_SIZE(uC53281_led_devices));
}

arch_initcall(uC53281_platform_add_led_devs);
