module DAC_corrector(
clk_in,
DATA_IN,
distance,

DATA_OUT
);

parameter in_width = 27;
parameter out_width = 14;

input clk_in;
input signed [(in_width-1):0] DATA_IN;
input unsigned [7:0] distance;

output reg unsigned [(out_width-1):0] DATA_OUT;

wire signed [(in_width-1):0] shifted; //for shift
wire signed [(in_width-1):0] tmp; //for rounding
wire signed [(out_width-1):0] tmp2; // for truncate

assign shifted = DATA_IN[(in_width-1):0] <<< distance;

assign tmp = shifted[(in_width-1):0] + { {(out_width){1'b0}}, shifted[(in_width-out_width)], {(in_width-out_width-1){!shifted[(in_width-out_width)]}}};
assign tmp2 = tmp[(in_width-1):(in_width-out_width)];

always @(posedge clk_in)
	DATA_OUT <= {~tmp2[(out_width-1)],tmp2[(out_width-2):0]}; //инвертируем первый бит, получая unsigned из two's complement

endmodule
