#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc58bbbf, "module_layout" },
	{ 0xfe990052, "gpio_free" },
	{ 0x7d4ed6c4, "gpiod_set_raw_value" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x1e0c9203, "gpiod_direction_output_raw" },
	{ 0xf2f8b814, "gpio_to_desc" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xf08fbc8d, "of_get_named_gpio_flags" },
	{ 0xb30ffa07, "of_get_child_by_name" },
	{ 0xc5850110, "printk" },
	{ 0x208794b, "of_find_compatible_node" },
};

MODULE_INFO(depends, "");

