#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h> 

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sergio Tanzilli");
MODULE_DESCRIPTION("Driver for HC-SR04 ultrasonic sensor");

// Change these two lines to use differents GPIOs as default
#define HCSR04_ECHO		95 // J4.32 -   PC31
#define HCSR04_TRIGGER		91 // J4.30 -   PC27
//#define HCSR04_TEST  	 	5 // J4.28 -   PA5
 
static int gpio_irq=-1;
static int valid_value = 0;

static ktime_t echo_start;
static ktime_t echo_end;

static int pecho=HCSR04_ECHO;
static int ptrg=HCSR04_TRIGGER;

// Two parameters available: 
//   pecho is for the echo pin
//   ptrg is for the trigger pin
//
module_param(pecho, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pecho, "ECHO Pin for HC. Default pin 95 [PC31]");
module_param(ptrg, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ptrg, "TRIGGER Pin for HC. Default pin 91 [PC27]");
 
int get_gpio_Num( int gpioid)
{
        if ( gpioid >=64 && gpioid <= 95)
                return gpioid-64;

        if ( gpioid >=0 && gpioid <= 31)
                return gpioid;

        if ( gpioid >=43 && gpioid <= 46)
                return gpioid-32;

        return 64;
}

char get_gpio_Port( int gpioid)
{
        if ( gpioid >=64 && gpioid <= 95)
                return 'C';

        if ( gpioid >=0 && gpioid <= 31)
                return 'A';

        if ( gpioid >=43 && gpioid <= 46)
                return 'B';

        return 'Z';
}

// This function is called when you write something on /sys/class/hcsr04/value
static ssize_t hcsr04_value_write(struct class *class, struct class_attribute *attr, const char *buf, size_t len) {
	printk(KERN_INFO "Buffer len %d bytes\n", len);
	return len;
}

// This function is called when you read /sys/class/hcsr04/value
static ssize_t hcsr04_value_read(struct class *class, struct class_attribute *attr, char *buf) {
	int counter;

	// Send a 10uS impulse to the TRIGGER line
	gpio_set_value(ptrg,1);
	udelay(10);
	gpio_set_value(ptrg,0);
	valid_value=0;

	counter=0;
	while (valid_value==0) {
		// Out of range
		if (++counter>23200) {
			return sprintf(buf, "%d\n", -1);;
		}
		udelay(1);
	}
	
	//printk(KERN_INFO "Sub: %lld\n", ktime_to_us(ktime_sub(echo_end,echo_start)));
	return sprintf(buf, "%lld\n", ktime_to_us(ktime_sub(echo_end,echo_start)));;
}

// This function is called when you write something on /sys/class/hcsr04/value
static ssize_t hcsr04_config_write(struct class *class, struct class_attribute *attr, const char *buf, size_t len) {
        printk(KERN_INFO "Buffer len %d bytes\n", len);
        return len;
}

// This function is called when you read /sys/class/hcsr04/value
static ssize_t hcsr04_config_read(struct class *class, struct class_attribute *attr, char *buf) {

        //printk(KERN_INFO "Sub: %lld\n", ktime_to_us(ktime_sub(echo_end,echo_start)));
        return sprintf(buf, "Trigger:P%c%d[K%d],Echo:P%c%d[K%d]\n", get_gpio_Port(ptrg),get_gpio_Num(ptrg),ptrg,\
                                                                    get_gpio_Port(pecho),get_gpio_Num(pecho),pecho);
}

// Sysfs definitions for hcsr04 class
static struct class_attribute hcsr04_class_attrs[] = {
	__ATTR(value,	S_IRUGO | S_IWUSR, hcsr04_value_read, hcsr04_value_write),
	__ATTR(config,  S_IRUGO | S_IWUSR, hcsr04_config_read, hcsr04_config_write),
	__ATTR_NULL,
};

// Name of directory created in /sys/class
static struct class hcsr04_class = {
	.name =			"hcsr04_est",
	.owner =		THIS_MODULE,
	.class_attrs =	hcsr04_class_attrs,
};

// Interrupt handler on ECHO signal
static irqreturn_t gpio_isr(int irq, void *data)
{
	ktime_t ktime_dummy;

	//gpio_set_value(HCSR04_TEST,1);

	if (valid_value==0) {
		ktime_dummy=ktime_get();
		if (gpio_get_value(pecho)==1) {
			echo_start=ktime_dummy;
		} else {
			echo_end=ktime_dummy;
			valid_value=1;
		}
	}

	//gpio_set_value(HCSR04_TEST,0);
	return IRQ_HANDLED;
}

static int hcsr04_est_init(void)
{	
	int rtc;
	
	printk(KERN_INFO "HC-SR04 driver v0.34 initializing.\n");

	printk(KERN_INFO " Using ECHO pin: %d, TRIGGER pin: %d \n", pecho, ptrg);

	if (class_register(&hcsr04_class)<0) goto fail_cr;

	//rtc=gpio_request(HCSR04_TEST,"TEST");
	//if (rtc!=0) {
	//	printk(KERN_INFO "Error %d\n",__LINE__);
	//	goto fail;
	//}

	//rtc=gpio_direction_output(HCSR04_TEST,0);
	//if (rtc!=0) {
	//	printk(KERN_INFO "Error %d\n",__LINE__);
	//	goto fail;
	//}

	rtc=gpio_request(ptrg,"TRIGGER");
	if (rtc!=0) {
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	}

	rtc=gpio_request(pecho,"ECHO");
	if (rtc!=0) {
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	}

	rtc=gpio_direction_output(ptrg,0);
	if (rtc!=0) {
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	}

	rtc=gpio_direction_input(pecho);
	if (rtc!=0) {
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	}

	// http://lwn.net/Articles/532714/
	rtc=gpio_to_irq(pecho);
	if (rtc<0) {
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	} else {
		gpio_irq=rtc;
	}

	rtc = request_irq(gpio_irq, gpio_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED , "hc-sr04_est.trigger", NULL);

	if(rtc) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", rtc);
		goto fail;
	}


	return 0;

fail:
	class_unregister(&hcsr04_class);

fail_cr:
	return -1;

}
 
static void hcsr04_est_exit(void)
{
	if (gpio_irq!=-1) {	
		free_irq(gpio_irq, NULL);
	}
	gpio_free(ptrg);
	gpio_free(pecho);
	class_unregister(&hcsr04_class);
	printk(KERN_INFO "HC-SR04 disabled.\n");
}
 
module_init(hcsr04_est_init);
module_exit(hcsr04_est_exit);
