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

reg signed [27:0] shifted; //for shifting

always @(posedge clk_in)
begin
	shifted = DATA_IN[27:0] <<< distance;
	DATA_OUT = {~shifted[27], shifted[26:14]}; //unsigned из two's complement
end

endmodule
