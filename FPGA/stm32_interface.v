module stm32_interface(
clk_in,
RX1_I,
RX1_Q,
RX2_I,
RX2_Q,
DATA_SYNC,
ADC_OTR,
DAC_OTR,
ADC_IN,
adcclk_in,
FLASH_data_in,
FLASH_busy,
IQ_valid,
VCXO_error,

DATA_BUS,
NCO1_freq,
preamp_enable,
rx1,
tx,
TX_I,
TX_Q,
reset_n,
stage_debug,
FLASH_data_out,
FLASH_enable,
FLASH_continue_read,
ADC_PGA,
ADC_RAND,
ADC_SHDN,
ADC_DITH,
unused,
CICFIR_GAIN,
TX_CICFIR_GAIN,
DAC_GAIN,
ADC_OFFSET,
NCO2_freq,
rx2,
tx_iq_valid,
VCXO_correction,
DAC_div0,
DAC_div1,
DAC_hp1,
DAC_hp2,
DAC_x4,
DCDC_freq,
TX_NCO_freq
);

input clk_in;
input signed [31:0] RX1_I;
input signed [31:0] RX1_Q;
input signed [31:0] RX2_I;
input signed [31:0] RX2_Q;
input DATA_SYNC;
input ADC_OTR;
input DAC_OTR;
input signed [15:0] ADC_IN;
input adcclk_in;
input unsigned [7:0] FLASH_data_in;
input FLASH_busy;
input IQ_valid;
input signed [23:0] VCXO_error;

output reg unsigned [31:0] NCO1_freq = 242347;
output reg unsigned [31:0] NCO2_freq = 242347;
output reg unsigned [31:0] TX_NCO_freq = 242347;
output reg preamp_enable = 0;
output reg rx1 = 1;
output reg rx2 = 0;
output reg tx = 0;
output reg reset_n = 1;
output reg signed [31:0] TX_I = 'd0;
output reg signed [31:0] TX_Q = 'd0;
output reg [15:0] stage_debug = 0;
output reg unsigned [7:0] FLASH_data_out = 0;
output reg FLASH_enable = 0;
output reg FLASH_continue_read = 0;
output reg ADC_PGA = 0;
output reg ADC_RAND = 0;
output reg ADC_SHDN = 1;
output reg ADC_DITH = 0;
output reg unsigned [7:0] unused = 0;
output reg unsigned [7:0] CICFIR_GAIN = 32;
output reg unsigned [7:0] TX_CICFIR_GAIN = 32;
output reg unsigned [7:0] DAC_GAIN = 32;
output reg signed [15:0] ADC_OFFSET = 'd0;
output reg tx_iq_valid = 0;
output reg signed [7:0] VCXO_correction = 'd0;
output reg DAC_div0 = 0;
output reg DAC_div1 = 0;
output reg DAC_hp1 = 0;
output reg DAC_hp2 = 0;
output reg DAC_x4 = 0;
output reg DCDC_freq = 0;

inout [7:0] DATA_BUS;
reg   [7:0] DATA_BUS_OUT;
reg         DATA_BUS_OE; // 1 - out 0 - in
assign DATA_BUS = DATA_BUS_OE ? DATA_BUS_OUT : 8'bZ ;

parameter rx_buffer_length = (8 - 1);
reg signed [15:0] k = 'd1;
reg signed [31:0] BUFFER_RX1_I [0:rx_buffer_length];
reg signed [31:0] BUFFER_RX1_Q [0:rx_buffer_length];
reg signed [31:0] BUFFER_RX2_I [0:rx_buffer_length];
reg signed [31:0] BUFFER_RX2_Q [0:rx_buffer_length];
reg signed [15:0] BUFFER_RX_head = 'd0;
reg signed [15:0] BUFFER_RX_tail = 'd0;
reg signed [31:0] REG_RX1_I;
reg signed [31:0] REG_RX1_Q;
reg signed [31:0] REG_RX2_I;
reg signed [31:0] REG_RX2_Q;
reg signed [31:0] I_HOLD;
reg signed [31:0] Q_HOLD;
reg signed [15:0] ADC_MIN;
reg signed [15:0] ADC_MAX;
reg ADC_MINMAX_RESET;
reg sync_reset_n = 1;
reg unsigned [7:0] BUS_TEST;

always @ (posedge IQ_valid)
begin
	BUFFER_RX1_I[BUFFER_RX_head][31:0] = RX1_I[31:0];
	BUFFER_RX1_Q[BUFFER_RX_head][31:0] = RX1_Q[31:0];
	BUFFER_RX2_I[BUFFER_RX_head][31:0] = RX2_I[31:0];
	BUFFER_RX2_Q[BUFFER_RX_head][31:0] = RX2_Q[31:0];
	if(BUFFER_RX_head >= rx_buffer_length)
		BUFFER_RX_head = 0;
	else
		BUFFER_RX_head = BUFFER_RX_head + 16'd1;
end

always @ (posedge clk_in)
begin
	//начало передачи
	if (DATA_SYNC == 1)
	begin
		DATA_BUS_OE = 0;
		ADC_MINMAX_RESET = 0;
		FLASH_continue_read = 0;
		
		if(DATA_BUS[7:0] == 'd0) //BUS TEST
		begin
			k = 500;
		end
		else if(DATA_BUS[7:0] == 'd1) //GET PARAMS
		begin
			k = 100;
		end
		else if(DATA_BUS[7:0] == 'd2) //SEND PARAMS
		begin
			DATA_BUS_OE = 1;
			k = 200;
		end
		else if(DATA_BUS[7:0] == 'd3) //TX IQ
		begin
			tx_iq_valid = 0;
			k = 300;
		end
		else if(DATA_BUS[7:0] == 'd4) //RX IQ
		begin
			DATA_BUS_OE = 1;
			k = 400;
		end
		else if(DATA_BUS[7:0] == 'd5) //RESET ON
		begin
			sync_reset_n = 0;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd6) //RESET OFF
		begin
			sync_reset_n = 1;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd7) //FPGA FLASH READ
		begin
			FLASH_enable = 0;
			k = 700;
		end
		else if(DATA_BUS[7:0] == 'd8) //GET INFO
		begin
			DATA_BUS_OE = 1;
			k = 800;
		end
	end
	else if (k == 100) //GET PARAMS
	begin
		rx1 = DATA_BUS[0:0];
		rx2 = DATA_BUS[1:1];
		tx = DATA_BUS[2:2];
		ADC_DITH = DATA_BUS[3:3];
		ADC_SHDN = DATA_BUS[4:4];
		ADC_RAND = DATA_BUS[5:5];
		ADC_PGA = DATA_BUS[6:6];
		preamp_enable = DATA_BUS[7:7];
		//clear TX chain
		if(tx == 0)
		begin
			I_HOLD[7:0] = 8'd0;
			Q_HOLD[7:0] = 8'd0;
			TX_I[31:0] = 32'd0;
			TX_Q[31:0] = 32'd0;
			tx_iq_valid = 1;
		end
		//
		k = 101;
	end
	else if (k == 101)
	begin
		NCO1_freq[31:24] = DATA_BUS[7:0];
		k = 102;
	end
	else if (k == 102)
	begin
		NCO1_freq[23:16] = DATA_BUS[7:0];
		k = 103;
	end
	else if (k == 103)
	begin
		NCO1_freq[15:8] = DATA_BUS[7:0];
		k = 104;
	end
	else if (k == 104)
	begin
		NCO1_freq[7:0] = DATA_BUS[7:0];
		k = 105;
	end
	else if (k == 105)
	begin
		NCO2_freq[31:24] = DATA_BUS[7:0];
		k = 106;
	end
	else if (k == 106)
	begin
		NCO2_freq[23:16] = DATA_BUS[7:0];
		k = 107;
	end
	else if (k == 107)
	begin
		NCO2_freq[15:8] = DATA_BUS[7:0];
		k = 108;
	end
	else if (k == 108)
	begin
		NCO2_freq[7:0] = DATA_BUS[7:0];
		k = 109;
	end
	else if (k == 109)
	begin
		CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 110;
	end
	else if (k == 110)
	begin
		TX_CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 111;
	end
	else if (k == 111)
	begin
		DAC_GAIN[7:0] = DATA_BUS[7:0];
		k = 112;
	end
	else if (k == 112)
	begin
		ADC_OFFSET[15:8] = DATA_BUS[7:0];
		k = 113;
	end
	else if (k == 113)
	begin
		ADC_OFFSET[7:0] = DATA_BUS[7:0];
		k = 114;
	end
	else if (k == 114)
	begin
		VCXO_correction[7:0] = DATA_BUS[7:0];
		k = 115;
	end
	else if (k == 115)
	begin
		DAC_div0 = DATA_BUS[0:0];
		DAC_div1 = DATA_BUS[1:1];
		DAC_hp1 = DATA_BUS[2:2];
		DAC_hp2 = DATA_BUS[3:3];
		DAC_x4 = DATA_BUS[4:4];
		DCDC_freq = DATA_BUS[5:5];
		k = 116;
	end
	else if (k == 116)
	begin
		TX_NCO_freq[31:24] = DATA_BUS[7:0];
		k = 117;
	end
	else if (k == 117)
	begin
		TX_NCO_freq[23:16] = DATA_BUS[7:0];
		k = 118;
	end
	else if (k == 118)
	begin
		TX_NCO_freq[15:8] = DATA_BUS[7:0];
		k = 119;
	end
	else if (k == 119)
	begin
		TX_NCO_freq[7:0] = DATA_BUS[7:0];
		k = 999;
	end
	else if (k == 200) //SEND PARAMS
	begin
		DATA_BUS_OUT[0:0] = ADC_OTR;
		DATA_BUS_OUT[1:1] = DAC_OTR;
		k = 201;
	end
	else if (k == 201)
	begin
		DATA_BUS_OUT[7:0] = ADC_MIN[15:8];
		k = 202;
	end
	else if (k == 202)
	begin
		DATA_BUS_OUT[7:0] = ADC_MIN[7:0];
		k = 203;
	end
	else if (k == 203)
	begin
		DATA_BUS_OUT[7:0] = ADC_MAX[15:8];
		k = 204;
	end
	else if (k == 204)
	begin
		DATA_BUS_OUT[7:0] = ADC_MAX[7:0];
		ADC_MINMAX_RESET=1;
		k = 205;
	end
	else if (k == 205)
	begin
		DATA_BUS_OUT[7:0] = VCXO_error[23:16];
		k = 206;
	end
	else if (k == 206)
	begin
		DATA_BUS_OUT[7:0] = VCXO_error[15:8];
		k = 207;
	end
	else if (k == 207)
	begin
		DATA_BUS_OUT[7:0] = VCXO_error[7:0];
		k = 999;
	end
	else if (k == 300) //TX IQ
	begin
		Q_HOLD[31:24] = DATA_BUS[7:0];
		k = 301;
	end
	else if (k == 301)
	begin
		Q_HOLD[23:16] = DATA_BUS[7:0];
		k = 302;
	end
	else if (k == 302)
	begin
		Q_HOLD[15:8] = DATA_BUS[7:0];
		k = 303;
	end
	else if (k == 303)
	begin
		Q_HOLD[7:0] = DATA_BUS[7:0];
		k = 304;
	end
	else if (k == 304)
	begin
		I_HOLD[31:24] = DATA_BUS[7:0];
		k = 305;
	end
	else if (k == 305)
	begin
		I_HOLD[23:16] = DATA_BUS[7:0];
		k = 306;
	end
	else if (k == 306)
	begin
		I_HOLD[15:8] = DATA_BUS[7:0];
		k = 307;
	end
	else if (k == 307)
	begin
		I_HOLD[7:0] = DATA_BUS[7:0];
		TX_I[31:0] = I_HOLD[31:0];
		TX_Q[31:0] = Q_HOLD[31:0];
		tx_iq_valid = 1;
		k = 999;
	end
	else if (k == 400) //RX1 IQ
	begin
		
		if(BUFFER_RX_tail == BUFFER_RX_head) //догнал буффер
		begin	
			REG_RX1_I[31:0] = 'd0;
			REG_RX1_Q[31:0] = 'd0;
			REG_RX2_I[31:0] = 'd0;
			REG_RX2_Q[31:0] = 'd0;
		end
		else
		begin
			REG_RX1_I[31:0] = BUFFER_RX1_I[BUFFER_RX_tail][31:0];
			REG_RX1_Q[31:0] = BUFFER_RX1_Q[BUFFER_RX_tail][31:0];
			REG_RX2_I[31:0] = BUFFER_RX2_I[BUFFER_RX_tail][31:0];
			REG_RX2_Q[31:0] = BUFFER_RX2_Q[BUFFER_RX_tail][31:0];
			
			if(BUFFER_RX_tail >= rx_buffer_length)
				BUFFER_RX_tail = 0;
			else
				BUFFER_RX_tail = BUFFER_RX_tail + 16'd1;
		end
		
		I_HOLD = REG_RX1_I;
		Q_HOLD = REG_RX1_Q;
		DATA_BUS_OUT[7:0] = Q_HOLD[31:24];
		k = 401;
	end
	else if (k == 401)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[23:16];
		k = 402;
	end
	else if (k == 402)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[15:8];
		k = 403;
	end
	else if (k == 403)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[7:0];
		k = 404;
	end
	else if (k == 404)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[31:24];
		k = 405;
	end
	else if (k == 405)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[23:16];
		k = 406;
	end
	else if (k == 406)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[15:8];
		k = 407;
	end
	else if (k == 407)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[7:0];
		if(rx2 == 1)
			k = 408;
		else
			k = 400;
	end
	else if (k == 408) //RX2 IQ
	begin
		I_HOLD = REG_RX2_I;
		Q_HOLD = REG_RX2_Q;
		DATA_BUS_OUT[7:0] = Q_HOLD[31:24];
		k = 409;
	end
	else if (k == 409)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[23:16];
		k = 410;
	end
	else if (k == 410)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[15:8];
		k = 411;
	end
	else if (k == 411)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[7:0];
		k = 412;
	end
	else if (k == 412)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[31:24];
		k = 413;
	end
	else if (k == 413)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[23:16];
		k = 414;
	end
	else if (k == 414)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[15:8];
		k = 415;
	end
	else if (k == 415)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[7:0];
		k = 400;
	end
	else if (k == 500) //BUS TEST
	begin
		BUS_TEST[7:0] = DATA_BUS[7:0];
		k = 501;
	end
	else if (k == 501)
	begin
		DATA_BUS_OE = 1;
		DATA_BUS_OUT[7:0] = BUS_TEST[7:0];
		k = 500;
	end
	else if (k == 700) //FPGA FLASH READ - SEND COMMAND
	begin
		DATA_BUS_OE = 0;
		FLASH_data_out[7:0] = DATA_BUS[7:0];
		if(FLASH_enable == 0)
			FLASH_enable = 1;
		else
			FLASH_continue_read = 1;
		k = 701;
	end
	else if (k == 701) //FPGA FLASH READ - READ ANSWER
	begin
		FLASH_continue_read = 0;
		DATA_BUS_OE = 1;
		if(FLASH_busy)
			DATA_BUS_OUT[7:0] = 'd255;
		else
			DATA_BUS_OUT[7:0] = FLASH_data_in[7:0];
		k = 700;
	end
	else if (k == 800) //GET INFO
	begin
		DATA_BUS_OUT[7:0] = 'd2; //flash id 1
		k = 801;
	end
	else if (k == 801)
	begin
		DATA_BUS_OUT[7:0] = 'd3; //flash id 2
		k = 802;
	end
	else if (k == 802)
	begin
		DATA_BUS_OUT[7:0] = 'd0; //flash id 3
		k = 999;
	end
	stage_debug=k;
end

always @ (posedge adcclk_in)
begin
	//ADC MIN-MAX
	if(ADC_MINMAX_RESET == 1)
	begin
		ADC_MIN = 'd32000;
		ADC_MAX = -16'd32000;
	end
	if(ADC_MAX<ADC_IN)
	begin
		ADC_MAX=ADC_IN;
	end
	if(ADC_MIN>ADC_IN)
	begin
		ADC_MIN=ADC_IN;
	end
end

always @ (negedge adcclk_in)
begin
	//RESET SYNC
	reset_n = sync_reset_n;
end

endmodule
