#include <stdio.h>
#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"
#include "string.h"
#include "init.h"

/*
 * 72시간 10Hz 트리거일 경우 2,592,000번 Count
 * Range 테스트
 * Threshold 테스트
 * 누적 테스트
 * DIO 테스트
 */

#define DIO_AXI_BASS_ADDR	0x40000000
#define ADC_A_AXI_BASS_ADDR	0x40001000
#define ADC_B_AXI_BASS_ADDR	0x40002000
#define ADC_C_AXI_BASS_ADDR	0x40003000
#define ADC_D_AXI_BASS_ADDR	0x40004000
#define ADC_E_AXI_BASS_ADDR	0x40005000
#define ADC_F_AXI_BASS_ADDR	0x40006000
#define ADC_G_AXI_BASS_ADDR	0x40007000
#define ADC_H_AXI_BASS_ADDR	0x40008000

int main()
{
	int menu;
	int dummy;
	long long axi_data;
	int cal_data;
	int i;
	int interlock;
	int do_port;
	float current_data;
	int cycle_cnt;
	float cycle_acc_data;
	float pulse_acc_data;
	float gain_test;
	float offset_test;

	int axi_base_addr;
	int dio_axi_base_addr = DIO_AXI_BASS_ADDR;
	int adc_freq = 200;
	int range;
	int axi_ch;
	int ram_size = 500;
	int threshold;
	float threshold_mA;

	float gain;
	float offset;

	printf("Range?\n");
	scanf("%d", &range);
	printf("range : %d\n", range);

	printf("ADC CH?\n");
	scanf("%d", &axi_ch);
	printf("ADC CH : %d\n", axi_ch);

	if (axi_ch == 0)
		axi_base_addr = ADC_A_AXI_BASS_ADDR;

	else if (axi_ch == 1)
		axi_base_addr = ADC_B_AXI_BASS_ADDR;

	else if (axi_ch == 2)
		axi_base_addr = ADC_C_AXI_BASS_ADDR;

	else if (axi_ch == 3)
		axi_base_addr = ADC_D_AXI_BASS_ADDR;

	else if (axi_ch == 4)
		axi_base_addr = ADC_E_AXI_BASS_ADDR;

	else if (axi_ch == 5)
		axi_base_addr = ADC_F_AXI_BASS_ADDR;

	else if (axi_ch == 6)
		axi_base_addr = ADC_G_AXI_BASS_ADDR;

	else if (axi_ch == 7)
		axi_base_addr = ADC_H_AXI_BASS_ADDR;

	axi_init(axi_base_addr, adc_freq, range, ram_size);

	gain = get_gain(range, axi_ch);
	offset = get_offset(range, axi_ch);

	sleep(1);

	printf("\n");
	printf("0 : Real Time Current Data\n");
	printf("1 : RAM Read\n");
	printf("2 : ACC Test\n");
	printf("3 : DO Test\n");
	printf("4 : DI Test\n");
	printf("5 : Count Test\n");
	printf("7 : Threshold Test\n");
	printf("8 : Calibration Test\n");
	printf("9 : ADC Raw Data\n");
	printf("--------------------------\n");

	printf("Mode?\n");
	scanf("%d", &menu);
	printf("%d\n", menu);

	while(1)
	{
		if (menu == 0)	// Real Time Current Data
		{
			axi_data = Xil_In32(axi_base_addr);

			current_data = (float)(((float)axi_data * gain) + offset);

			printf("%f\n", current_data);
			usleep(100000);
		}

		else if (menu == 1)		// RAM Read
		{
			scanf("%d", &dummy);
			printf("Press Anykey\n");

			for (i = 0; i < ram_size; i++)
			{
				Xil_Out32((axi_base_addr + 16), i);
				axi_data = Xil_In32(axi_base_addr + 4);

				current_data = (float)(((float)axi_data * gain) + offset);

				printf("%f\n", current_data);
				usleep(10000);
			}
			printf("\n");
		}

		else if (menu == 2)		// ACC Flag
		{
			Xil_Out32((axi_base_addr + 24), 1);

			cycle_cnt = Xil_In32(axi_base_addr + 32);

			axi_data = Xil_In32(axi_base_addr + 28);
			cycle_acc_data = (float)(((float)axi_data * gain) + offset);

			axi_data = Xil_In32(axi_base_addr + 36);
			axi_data = axi_data << 32;
			pulse_acc_data = ((float)axi_data * gain) + offset;

			axi_data = Xil_In32(axi_base_addr + 40);
			pulse_acc_data = ((float)axi_data * gain) + offset + pulse_acc_data;

			if (cycle_cnt >= 2592000)
			{
				printf("CNT : %d\n", cycle_cnt);
				printf("Cycle : %f\n", cycle_acc_data);
				printf("Pulse : %f\n", pulse_acc_data);
				printf("-----END-----\n");

				return 0;
			}

			else
			{
				printf("CNT : %d\n", cycle_cnt);
				printf("Cycle : %f\n", cycle_acc_data);
				printf("Pulse : %f\n", pulse_acc_data);
				printf("\n");
			}

			sleep(1);

		}

		else if (menu == 3)		// DO Test
		{
			Xil_Out32((0x40001000 + 48), 123456);
			Xil_Out32((0x40002000 + 48), 123456);
			Xil_Out32((0x40003000 + 48), 123456);
			Xil_Out32((0x40004000 + 48), 123456);
			Xil_Out32((0x40005000 + 48), 123456);
			Xil_Out32((0x40006000 + 48), 123456);
			Xil_Out32((0x40007000 + 48), 123456);
			Xil_Out32((0x40008000 + 48), 123456);

			printf("DO?\n");
			scanf("%d", &do_port);
			printf("%d\n", do_port);

			Xil_Out32((dio_axi_base_addr + 4), do_port);
		}

		else if (menu == 4)		// DI Test
		{
			i = Xil_In32(dio_axi_base_addr);

			printf("DI : %d\n", i);
			sleep(1);
		}

		else if (menu == 5)
		{
			axi_init(ADC_A_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_B_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_C_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_D_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_E_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_F_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_G_AXI_BASS_ADDR, adc_freq, range, ram_size);
			axi_init(ADC_H_AXI_BASS_ADDR, adc_freq, range, ram_size);

			Xil_Out32((ADC_A_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_B_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_C_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_D_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_E_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_F_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_G_AXI_BASS_ADDR + 24), 1);
			Xil_Out32((ADC_H_AXI_BASS_ADDR + 24), 1);

			while (1)
			{
				cycle_cnt = Xil_In32(ADC_A_AXI_BASS_ADDR + 32);
				printf("A CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_B_AXI_BASS_ADDR + 32);
				printf("B CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_C_AXI_BASS_ADDR + 32);
				printf("C CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_D_AXI_BASS_ADDR + 32);
				printf("D CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_E_AXI_BASS_ADDR + 32);
				printf("E CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_F_AXI_BASS_ADDR + 32);
				printf("F CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_G_AXI_BASS_ADDR + 32);
				printf("G CNT : %d\n", cycle_cnt);

				cycle_cnt = Xil_In32(ADC_H_AXI_BASS_ADDR + 32);
				printf("H CNT : %d\n", cycle_cnt);

				printf("\n");
				sleep(1);
			}
		}

		else if (menu == 7)
		{
			printf("Threshold? (mA)\n");
			scanf("%f", &threshold_mA);

			threshold = (int)((threshold_mA - offset) / gain);

			printf("%f mA -> %d\n", threshold_mA, threshold);

			if (threshold <= 0)
				threshold = 0;

			Xil_Out32((0x40001000 + 48), threshold);
			Xil_Out32((0x40002000 + 48), threshold);
			Xil_Out32((0x40003000 + 48), threshold);
			Xil_Out32((0x40004000 + 48), threshold);
			Xil_Out32((0x40005000 + 48), threshold);
			Xil_Out32((0x40006000 + 48), threshold);
			Xil_Out32((0x40007000 + 48), threshold);
			Xil_Out32((0x40008000 + 48), threshold);

			Xil_Out32((dio_axi_base_addr + 8), 170);	// 1010_1010

			i = 0;

			while(1)
			{
				Xil_Out32((axi_base_addr + 24), 1);

				cycle_cnt = Xil_In32(axi_base_addr + 32);

				axi_data = Xil_In32(axi_base_addr + 28);
				cycle_acc_data = (float)(((float)axi_data * gain) + offset);

				axi_data = Xil_In32(axi_base_addr + 36);
				axi_data = axi_data << 32;
				pulse_acc_data = ((float)axi_data * gain) + offset;

				axi_data = Xil_In32(axi_base_addr + 40);
				pulse_acc_data = ((float)axi_data * gain) + offset + pulse_acc_data;

				interlock = Xil_In32(axi_base_addr + 52);

				if (i == 100)
				{
					printf("CNT : %d\n", cycle_cnt);
					i = 0;
				}

				if (interlock == 1)
				{
					printf("--------------------------\n");
					printf("CNT : %d\n", cycle_cnt);
					printf("Cycle : %f\n", cycle_acc_data);
					printf("Pulse : %f\n", pulse_acc_data);
					printf("Threshold : %f\n", threshold_mA);
					printf("--------------------------\n");
					printf("\n");

					return 0;
				}

				i++;

				usleep(1000);
			}
		}

		else if (menu == 8)		// Cal Value Test
		{
			printf("GAIN?\n");
			scanf("%f", &gain_test);
			printf("OFFSET?\n");
			scanf("%f", &offset_test);

			printf("GAIN : %f\n", gain_test);
			printf("OFFSET : %f\n", offset_test);

			while(1)
			{
				axi_data = Xil_In32(axi_base_addr);

				current_data = (float)(((float)axi_data * gain_test) + offset_test);

				printf("%f\n", current_data);
				sleep(1);
			}
		}

		else if (menu == 9)		// ADC Raw Data
		{
			cal_data = Xil_In32(axi_base_addr);

			printf("%d\n", cal_data);
			sleep(1);
		}
	}
	return 0;
}
