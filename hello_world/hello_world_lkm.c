//
// Created by Rudi Muliawan on 21/10/21.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define AUTHOR_NAME "RUDI_MULIAWAN"
#define MODULE_DESC "HELLO_WORLD_LKM"


int hello_world_init_module(void)
{
    printk(KERN_INFO "Hello World Module is Successfuly Loaded\n");
    return 0;
}


void hello_world_cleanup_module(void)
{
    printk(KERN_INFO "Goodbye Hello World Module\n");
}


module_init(hello_world_init_module);
module_exit(hello_world_cleanup_module);

MODULE_AUTHOR(AUTHOR_NAME);
MODULE_DESCRIPTION(MODULE_DESC);
MODULE_LICENSE("GPL");
