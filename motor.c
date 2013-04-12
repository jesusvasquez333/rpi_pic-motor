#include <bcm2835.h>
#include <stdio.h>


#define MOTOR_CS	RPI_V2_GPIO_P1_22
#define MOTOR_DIR	RPI_V2_GPIO_P1_24
#define MOTOR_STEP	RPI_V2_GPIO_P1_26

int help(char *command)
{
	printf("Steper motor driver based on Raspberry Pi + PIC (Version 1)\n");
	printf("Developed by Jesus Vasquez.\n");
	printf("Usage: %s <SPEED> <ACCE> <ACCE_F>\n Where:\n  <SPEED> 0 = 1875 rpm; 1 = 937,5 rpm; ... ; 7 = 14,6 rpm; ... ; 15 = 0,06 rpm.\n  <ACCE> 0 = without acceleration curve; 1 = with acceleration curve. and\n  <ACCE_F> = acceleration factor (1..5)\n", command);
	return 1;
}

int main(int argc, char **argv)
{

	uint8_t microstep_index, step_index;
	uint8_t direction = 0;
	uint8_t speed;
	uint16_t microstep_time;
	int microstep_count, microstep_togo, microstep_input;
	uint8_t speed_var = 8;
	uint16_t delay_var, delay_var_final;
	uint16_t speedchange_count = 0;
	uint8_t use_acceleration, acceleration_factor;

	char *speed_str[] = {"1875 rpm", "937,5 rpm", "468,75 rpm", "234,36 rpm", "117,19 rpm", "58,59 rpm", "29,30 rpm", "14,65 rpm", "7,32 rpm", "3,66 rpm", "1,83 rpm", "0,92 rpm", "0,46 rpm", "0,23 rpm", "0,11 rpm", "0,06 rpm"};

	if (argc != 4)
		return help(argv[0]);

	speed = atoi(argv[1]); 
	if (speed > 15)
		return help(argv[0]);

	use_acceleration = atoi(argv[2]);
	if (use_acceleration > 1)
		return help(argv[0]);

	acceleration_factor = atoi(argv[3]); 
	if ((acceleration_factor  < 1) | (acceleration_factor  > 5))
		return help(argv[0]);

    if (!bcm2835_init())
    {
    	printf("Error during BCM2835_INIT() function\n");
		return 1;
	}

	if (use_acceleration)
	{
		if (speed < 8)
			delay_var = 256;
		else
			delay_var = (1 << speed);

		delay_var_final = (1 << speed);
	}
	else
		delay_var = (1 << speed);

	microstep_time = 150 * delay_var;

	printf("Steper motor driver based on Raspberry Pi + PIC (Version 1)\n");
	printf("Developed by Jesus Vasquez.\n");
	printf("Speed selected = %s\n", speed_str[speed]);
	printf("Use acceleration curve = %d\n", use_acceleration);
	printf("Acceleration factor = %d\n", acceleration_factor);

    // Set the pin to be an output
    bcm2835_gpio_fsel(MOTOR_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(MOTOR_DIR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(MOTOR_STEP, BCM2835_GPIO_FSEL_OUTP);

    microstep_count = 0;
    microstep_togo = 0;

    bcm2835_gpio_write(MOTOR_CS, HIGH);

    while (1)
    {

	direction = (microstep_count < microstep_togo);
	bcm2835_gpio_write(MOTOR_DIR, direction);
	bcm2835_delayMicroseconds(500);

		if (microstep_count != microstep_togo)
		{
			bcm2835_gpio_write(MOTOR_CS, LOW);

			bcm2835_gpio_write(MOTOR_STEP, HIGH);
			bcm2835_delayMicroseconds(microstep_time);
			bcm2835_gpio_write(MOTOR_STEP, LOW);
			bcm2835_delayMicroseconds(microstep_time);

			if(direction)
				microstep_count++;
			else
				microstep_count--;

			if (use_acceleration)
			{
				if (delay_var > delay_var_final)
				{
					if (++speedchange_count >= acceleration_factor)
					{
						speedchange_count = 0;
						delay_var--;
						microstep_time = 10 * delay_var;
					}
				}
			}

		}
		else
		{
			bcm2835_gpio_write(MOTOR_CS, HIGH);
			printf("Current position: %d\n", microstep_count);
			printf("Number of microstep to go?: ");
			scanf("%d",&microstep_input);
			microstep_togo += microstep_input;
			printf("Going to position %d ...\n\n", microstep_togo);

			if (use_acceleration)
			{
				if (speed < 8)
					delay_var = 256;
				else
					delay_var = (1 << speed);

				delay_var_final = (1 << speed);
				
				microstep_time = 150 * delay_var;
			}
				
		}
		
    }
    bcm2835_close();
    return 0;
}

