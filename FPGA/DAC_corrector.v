module DAC_corrector(
clk_in,
DATA_IN,
distance,

DATA_OUT
);

input clk_in;
input signed [27:0] DATA_IN;
input unsigned [7:0] distance;

output reg unsigned [13:0] DATA_OUT;

wire signed [27:0] shifted; //for shifting
wire signed [27:0] rounded; //for rounding
wire signed [27:0] added; // for adding

assign shifted = DATA_IN[27:0] <<< distance;
assign rounded = shifted[27:0] + { {(14){1'b0}}, !shifted[27], {(28-14-1){shifted[27]}}};
assign added = $signed(rounded[27:(28-14)]) + $signed('d8191);

always @(posedge clk_in)
begin
	DATA_OUT = added[13:0]; //unsigned из two's complement
end

endmodule
