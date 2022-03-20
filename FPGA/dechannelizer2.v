module dechannelizer2(
	input wire [(width-1):0] in_data_1,
	input wire [(width-1):0] in_data_2,
	input wire in_valid,
	input wire in_ready,
	input wire clk,
	input wire empty_1,
	input wire empty_2,

	output reg [(width-1):0] out_data = 'd0,
	output reg out_valid,
	output reg out_sop,
	output reg out_eop
);

parameter width = 32;

reg signed [(width-1):0] data_1_reg = 'd0;
reg signed [(width-1):0] data_2_reg = 'd0;
reg [2:0] state = 'd0;

always @(posedge clk)
begin
	if(state == 0 && in_valid && in_ready && !empty_1 && !empty_2)
	begin
		data_1_reg = in_data_1;
		data_2_reg = in_data_2;
		out_sop = 0;
		out_eop = 0;
		state = 'd1;
	end
	else if(state == 1)
	begin
		out_data = data_1_reg;
		out_valid = 1;
		out_sop = 1;
		out_eop = 0;
		state = 'd2;
	end
	else if(state == 2)
	begin
		out_data = data_2_reg;
		out_valid = 1;
		out_sop = 0;
		out_eop = 1;
		state = 'd3;
	end
	else if(state == 3)
	begin
		out_data = 'd0;
		out_valid = 0;
		out_sop = 0;
		out_eop = 0;
		state = 'd4;
	end
	else if(state[2:0] == 4 && !in_valid)
	begin
		state = 'd0;
	end
end

endmodule
