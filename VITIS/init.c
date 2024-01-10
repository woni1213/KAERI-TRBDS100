#include <stdio.h>
#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"
#include "string.h"
#include "init.h"

void axi_init(int axi_base_addr, int adc_freq, int range, int ram_size)
{
	Xil_Out32((axi_base_addr + 8), adc_freq);
	Xil_Out32((axi_base_addr + 20), range);
	Xil_Out32((axi_base_addr + 12), ram_size);
}

float get_gain(int range, int ch)
{
	range = range - 1;

	float gain[3][8];

	gain[0][0] = 0.00001388;
	gain[0][1] = 0.00001386;
	gain[0][2] = 0.00001379;
	gain[0][3] = 0.00001387;
	gain[0][4] = 0.00001390;
	gain[0][5] = 0.00001376;
	gain[0][6] = 0.00001388;
	gain[0][7] = 0.00001390;

	gain[1][0] = 0.00013925;
	gain[1][1] = 0.00013872;
	gain[1][2] = 0.00013802;
	gain[1][3] = 0.00013865;
	gain[1][4] = 0.00013844;
	gain[1][5] = 0.00013764;
	gain[1][6] = 0.00013932;
	gain[1][7] = 0.00013953;

	gain[2][0] = 0.00138718;
	gain[2][1] = 0.00138291;
	gain[2][2] = 0.00137825;
	gain[2][3] = 0.00138483;
	gain[2][4] = 0.00137994;
	gain[2][5] = 0.00137510;
	gain[2][6] = 0.00138291;
	gain[2][7] = 0.00138867;

	return gain[range][ch];
}

float get_offset(int range, int ch)
{
	range = range - 1;

	float offset[3][8];

	offset[0][0] = 0.00164276;
	offset[0][1] = 0.00189002;
	offset[0][2] = 0.00164394;
	offset[0][3] = 0.00176144;
	offset[0][4] = 0.00163243;
	offset[0][5] = 0.00126147;
	offset[0][6] = 0.00117484;
	offset[0][7] = 0.00136834;

	offset[1][0] = 0.01700449;
	offset[1][1] = 0.01968249;
	offset[1][2] = 0.01650054;
	offset[1][3] = 0.01736250;
	offset[1][4] = 0.01375173;
	offset[1][5] = 0.01246368;
	offset[1][6] = 0.01181115;
	offset[1][7] = 0.01320930;

	offset[2][0] = 0.17046856;
	offset[2][1] = 0.19099570;
	offset[2][2] = 0.16615620;
	offset[2][3] = 0.17602708;
	offset[2][4] = 0.14167433;
	offset[2][5] = 0.12131398;
	offset[2][6] = 0.11908420;
	offset[2][7] = 0.13485573;

	return offset[range][ch];
}
