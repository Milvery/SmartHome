cmd_/home/ubuntu/newdisk/smart_home_item/fan-pwm2/Module.symvers := sed 's/ko$$/o/' /home/ubuntu/newdisk/smart_home_item/fan-pwm2/modules.order | scripts/mod/modpost -m  -E  -o /home/ubuntu/newdisk/smart_home_item/fan-pwm2/Module.symvers -e -i Module.symvers   -T -