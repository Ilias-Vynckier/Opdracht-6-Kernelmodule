/*
 * Basic kernel module using some GPIOs to drive LEDs.
 *
 * Author:
 * 	Stefan Wendler (devnull@kaltpost.de)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>

static int outputs[3] = {-1, -1, -1};
static int level[3] = {-1, -1, -1};
static int togglespeed[3] = {-1, -1, -1};
static int arr_argc = 0;

static unsigned long timer_interval_ns0 = 1; // 25kHz = 40.000ns
static unsigned long timer_interval_ns1 = 1; // 25kHz = 40.000ns
static unsigned long timer_interval_ns2 = 1; // 25kHz = 40.000ns

static struct hrtimer hr_timer0;
static struct hrtimer hr_timer1;
static struct hrtimer hr_timer2;

static int pin_value = 0;
static int dinknumb0 = 0;
static int dinknumb1 = 0;
static int dinknumb2 = 0;

module_param_array(outputs, int, &arr_argc, 0000);
MODULE_PARM_DESC(outputs, "An array of integers");

module_param_array(level, int, &arr_argc, 0000);
MODULE_PARM_DESC(level, "An array of integers");

module_param_array(togglespeed, int, &arr_argc, 0000);
MODULE_PARM_DESC(togglespeed, "An array of integers");

/*
 * Struct defining pins, direction and inital state 
 */
static struct gpio leds[] = {
    {4, GPIOF_OUT_INIT_HIGH, "LED 1"},
    {25, GPIOF_OUT_INIT_HIGH, "LED 2"},
    {24, GPIOF_OUT_INIT_HIGH, "LED 3"},
};

/*
 * Timer function called periodically
 */
enum hrtimer_restart timer_callback0(struct hrtimer *timer_for_restart)
{
    ktime_t currtime;
    ktime_t interval;

    currtime = ktime_get();
    interval = ktime_set(timer_interval_ns0, 0);

    hrtimer_forward(timer_for_restart, currtime, interval);

    printk(KERN_INFO "DINK %d curtime %lld\n", dinknumb0++, currtime);
    gpio_set_value(leds[0].gpio, pin_value);
    pin_value = !pin_value;
    return HRTIMER_RESTART;
}

enum hrtimer_restart timer_callback1(struct hrtimer *timer_for_restart)
{
    ktime_t currtime;
    ktime_t interval;

    currtime = ktime_get();
    interval = ktime_set(timer_interval_ns1, 0);

    hrtimer_forward(timer_for_restart, currtime, interval);

    printk(KERN_INFO "DINK1 %d curtime %lld\n", dinknumb1++, currtime);
    gpio_set_value(leds[1].gpio, pin_value);
    pin_value = !pin_value;
    return HRTIMER_RESTART;
}

enum hrtimer_restart timer_callback2(struct hrtimer *timer_for_restart)
{
    ktime_t currtime;
    ktime_t interval;

    currtime = ktime_get();
    interval = ktime_set(timer_interval_ns2, 0);

    hrtimer_forward(timer_for_restart, currtime, interval);

    printk(KERN_INFO "DINK2 %d curtime %lld\n", dinknumb2++, currtime);
    gpio_set_value(leds[2].gpio, pin_value);
    pin_value = !pin_value;
    return HRTIMER_RESTART;
}

/* Task handle to identify thread */
static struct task_struct *ts0 = NULL;
static struct task_struct *ts1 = NULL;
static struct task_struct *ts2 = NULL;

/*
 * Thread to blink LED 2
 */
static int led_thread0(int i)
{
    printk(KERN_INFO "%s\n", __func__);
    hrtimer_init(&hr_timer0, CLOCK_MONOTONIC, HRTIMER_MODE_REL); /// meer timers toevoegen
    hr_timer0.function = &timer_callback0;

    gpio_set_value(leds[0].gpio, level[i]);
    if (!(togglespeed[i] == 0))
    {
        timer_interval_ns0 = togglespeed[i],
        hrtimer_start(&hr_timer0, togglespeed[i], HRTIMER_MODE_REL);
    }
    return 0;
}

/*
 * Thread to blink LED 2
 */
static int led_thread1(int i)
{
    printk(KERN_INFO "%s\n", __func__);
    hrtimer_init(&hr_timer1, CLOCK_MONOTONIC, HRTIMER_MODE_REL); /// meer timers toevoegen
    hr_timer1.function = &timer_callback1;

    gpio_set_value(leds[1].gpio, level[i]);
    if (!(togglespeed[i] == 0))
    {
        timer_interval_ns1 = togglespeed[i],
        hrtimer_start(&hr_timer1, togglespeed[i], HRTIMER_MODE_REL);
    }
    return 0;
}

/*
 * Thread to blink LED 2
 */
static int led_thread2(int i)
{
    printk(KERN_INFO "%s\n", __func__);
    hrtimer_init(&hr_timer2, CLOCK_MONOTONIC, HRTIMER_MODE_REL); /// meer timers toevoegen
    hr_timer2.function = &timer_callback2;

    gpio_set_value(leds[2].gpio, level[i]);
    if (!(togglespeed[i] == 0))
    {
        timer_interval_ns2 = togglespeed[i],
        hrtimer_start(&hr_timer2, togglespeed[i], HRTIMER_MODE_REL);
    }
    return 0;
}
/*
 * Module init function
 */
static int __init gpiomod_init(void)
{
    int ret = 0, i;
    ktime_t interval;

    printk(KERN_INFO "%s\n", __func__);

    //ret = gpio_request_array(leds, ARRAY_SIZE(leds));
    //register LED GPIOs, turn LEDs on
    ret = gpio_request_array(leds, ARRAY_SIZE(leds));

    /* init timer, add timer function */
    //interval = ktime_set(0, timer_interval_ns0);

    /*hrtimer_init(&hr_timer1, CLOCK_MONOTONIC, HRTIMER_MODE_REL); /// meer timers toevoegen
    hrtimer_init(&hr_timer2, CLOCK_MONOTONIC, HRTIMER_MODE_REL); /// meer timers toevoegen
    
    hr_timer1.function = &timer_callback1;
    hr_timer2.function = &timer_callback2;*/

    if (ret)
    {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
    }

    for (i = 0; i < (sizeof outputs / sizeof(int)); i++)
    {
        printk(KERN_INFO "outputs[%d] = %d\n", i, outputs[i]);

        switch (outputs[i])
        {
        case 4:
            //button_irqs[0]=1;

            ts0 = kthread_create(led_thread0(i), NULL, "led_thread");
            if (ts0)
            {
                wake_up_process(ts0);
            }
            else
            {
                printk(KERN_ERR "Unable to create thread\n");
            }
            break;
        case 25:
            ts1 = kthread_create(led_thread1(i), NULL, "led_thread");
            if (ts1)
            {
                wake_up_process(ts1);
            }
            else
            {
                printk(KERN_ERR "Unable to create thread\n");
            }
            break;
        case 24:
            ts2 = kthread_create(led_thread2(i), NULL, "led_thread");
            if (ts2)
            {
                wake_up_process(ts2);
            }
            else
            {
                printk(KERN_ERR "Unable to create thread\n");
            }
            break;

        default:
            break;
        }
    }

    for (i = 0; i < (sizeof level / sizeof(int)); i++)
    {
        printk(KERN_INFO "level[%d] = %d\n", i, level[i]);
    }

    for (i = 0; i < (sizeof level / sizeof(int)); i++)
    {
        printk(KERN_INFO "togglespeed[%d] = %d\n", i, togglespeed[i]);
    }

    printk(KERN_INFO "KAK\n");

    return ret;
}

/*
 * Module exit function
 */
static void __exit gpiomod_exit(void)
{
    int i;
    int ret = 0;

    printk(KERN_INFO "%s\n", __func__);

     // terminate thread
	if(ts0) {
		kthread_stop(ts0);
	}

    // terminate thread
	if(ts1) {
		kthread_stop(ts1);
	}

    // terminate thread
	if(ts2) {
		kthread_stop(ts2);
	}

    ret = hrtimer_cancel(&hr_timer0);
    ret = hrtimer_cancel(&hr_timer1);
    ret = hrtimer_cancel(&hr_timer2);
    if (ret)
    {
        printk("Failed to cancel tiemr.\n");
    }

     

    // turn all LEDs off
    for (i = 0; i < ARRAY_SIZE(leds); i++)
    {
        gpio_set_value(leds[i].gpio, 0);
    }

    // unregister all GPIOs
    gpio_free_array(leds, ARRAY_SIZE(leds));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Wendler");
MODULE_DESCRIPTION("Basic Linux Kernel module using GPIOs to drive LEDs");

module_init(gpiomod_init);
module_exit(gpiomod_exit);