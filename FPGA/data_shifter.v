module data_shifter(
	input wire [(in_width-1):0] data_in_I,
	input wire [(in_width-1):0] data_in_Q,
	input unsigned [7:0] distance,

	output wire [(out_width-1):0] data_out_I,
	output wire [(out_width-1):0] data_out_Q
);

parameter in_width = 88;
parameter out_width = 32;
assign data_out_I[(out_width-1):0] = data_in_I[(distance-1) -: out_width];
assign data_out_Q[(out_width-1):0] = data_in_Q[(distance-1) -: out_width];

endmodule
