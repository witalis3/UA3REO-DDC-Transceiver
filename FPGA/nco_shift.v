module nco_shift (
	in,
	out
);

input	[15:0] in;
output [13:0] out;

assign out[13:0]=in[15:2];
	
endmodule
