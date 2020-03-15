module data_shifter(
	input [(in_width-1):0] data_in_I,
	input wire data_valid_I,
	input [(in_width-1):0] data_in_Q,
	input wire data_valid_Q,
	input unsigned [7:0] distance,
	input wire enabled,

	output reg [(out_width-1):0] data_out_I,
	output wire data_valid_out_I,
	output reg [(out_width-1):0] data_out_Q,
	output wire data_valid_out_Q
);

parameter in_width = 88;
parameter out_width = 32;
assign data_valid_out_I = data_valid_I & enabled;
assign data_valid_out_Q = data_valid_Q & enabled;

always @ (posedge data_valid_I)
begin
	if (enabled==1)
	begin
		data_out_I[(out_width-1):0] = data_in_I[(distance-1) -: out_width];
	end
	else
	begin
		data_out_I[(out_width-1):0] = 0;
	end
end

always @ (posedge data_valid_Q)
begin
	if (enabled==1)
	begin
		data_out_Q[(out_width-1):0] = data_in_Q[(distance-1) -: out_width];
	end
	else
	begin
		data_out_Q[(out_width-1):0] = 0;
	end
end

endmodule
