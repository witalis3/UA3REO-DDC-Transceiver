module channelizer2(
	input wire [(width-1):0] in_data,
	input wire in_valid,
	input wire [1:0] in_error,
	input wire [0:0] channel,

	output reg [(width-1):0] out_data_1 = 'd0,
	output wire [1:0] out_error_1,
	output reg [(width-1):0] out_data_2 = 'd0,
	output wire [1:0] out_error_2,
	output wire out_valid_1,
	output wire out_valid_2
);

parameter width = 32;

assign out_error_1 = in_error; 
assign out_error_2 = in_error;

assign out_valid_1 = (channel == 'd0) && in_valid;
assign out_valid_2 = (channel == 'd1) && in_valid;

always @(posedge out_valid_1)
	out_data_1 <= in_data;
always @(posedge out_valid_2)
	out_data_2 <= in_data;
	
endmodule