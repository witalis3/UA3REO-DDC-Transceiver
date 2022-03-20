
module tx_cic (
	in_error,
	in_valid,
	in_ready,
	in_data,
	in_startofpacket,
	in_endofpacket,
	out0_data,
	out1_data,
	out_error,
	out_valid,
	out_ready,
	clk,
	reset_n);	

	input	[1:0]	in_error;
	input		in_valid;
	output		in_ready;
	input	[15:0]	in_data;
	input		in_startofpacket;
	input		in_endofpacket;
	output	[13:0]	out0_data;
	output	[13:0]	out1_data;
	output	[1:0]	out_error;
	output		out_valid;
	input		out_ready;
	input		clk;
	input		reset_n;
endmodule
