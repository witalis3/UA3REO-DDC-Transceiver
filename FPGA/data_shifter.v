module data_shifter(
data_in_I,
data_valid_I,
data_in_Q,
data_valid_Q,
distance,

data_out_I,
data_out_Q
);

parameter in_width = 88;
parameter out_width = 32;

input [(in_width-1):0] data_in_I;
input [(in_width-1):0] data_in_Q;
input data_valid_I;
input data_valid_Q;
input unsigned [7:0] distance;

output reg [(out_width-1):0] data_out_I=0;
output reg [(out_width-1):0] data_out_Q=0;

always @ (posedge data_valid_I)
begin
	data_out_I[(out_width-1):0] = data_in_I[(distance-1) -: out_width];
end

always @ (posedge data_valid_Q)
begin
	data_out_Q[(out_width-1):0] = data_in_Q[(distance-1) -: out_width];
end

endmodule
