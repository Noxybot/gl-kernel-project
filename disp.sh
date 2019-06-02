#!/bin/bash
echo 0 > /sys/class/graphics/fbcon/cursor_blink
echo 355 > /sys/class/gpio/export
echo rising > /sys/class/gpio/gpio355/edge
echo 1 > /sys/class/gpio/gpio355/active_low

get_button_status()
{
    local status=$(cat /sys/class/gpio/gpio355/value)
    echo "$status"
}

show_mpu()
{
    temp_c=$(cat /sys/class/mpu6050/temp_c)
    temp_f=$(cat /sys/class/mpu6050/temp_f)
    gyro_x=$(cat /sys/class/mpu6050/gyro_x)
    gyro_y=$(cat /sys/class/mpu6050/gyro_y)
    gyro_z=$(cat /sys/class/mpu6050/gyro_z)
    accel_x=$(cat /sys/class/mpu6050/accel_x)
    accel_y=$(cat /sys/class/mpu6050/accel_y)
    accel_z=$(cat /sys/class/mpu6050/accel_z)

    printf -v res "MPU6050\n\nTEMP C: $temp_c\nTEMP F: $temp_f
                   \n\nGYRO X: $gyro_x\nGYRO Y: $gyro_y\nGYRO Z: $gyro_z
                   \n\nACCEL X: $accel_x\nACCEL Y: $accel_y\nACCEL Z: $accel_z"
    /home/user/FONT "${res@E}"
	
}

show_bmp()
{
    temp=$(cat /sys/class/bmp180/temp)
    press_in_pa=$(cat /sys/class/bmp180/press_in_pa)
    press_mm_rt_st=$(cat /sys/class/bmp180/press_in_mm_rt_st)

    printf -v res "BMP180\n\nTEMP C: $temp\n\nPRESSURE: $press_in_pa Pa
			\nPRESSURE: $press_mm_rt_st\n"
    /home/user/FONT "${res@E}"
}


while [ true ]
do
	if [ $(get_button_status) -ne 0 ]
	then
	   while [ true ]
	   do
		show_mpu
		sleep 0.1
		if [ $(get_button_status) -ne 0 ]
		then
		   break
		fi
	   done
	fi
	show_bmp
	sleep 0.1
done
echo 355 > /sys/class/gpio/unexport


