module DAC_corrector(
clk_in,
DATA_IN,

DATA_OUT
);

parameter in_width = 27;
parameter out_width = 14;
parameter shift = 27;

input clk_in;
input signed [(in_width-1):0] DATA_IN;

output reg unsigned [(out_width-1):0] DATA_OUT;

reg signed [(out_width-1):0] tmp=0;

always @ (posedge clk_in)
begin
	//получаем 14 бит
	tmp[(out_width-1):0] = DATA_IN[(shift-1) -: out_width];
	
	//инвертируем первый бит, получая unsigned из two's complement
	DATA_OUT[(out_width-1):0]={~tmp[(out_width-1)],tmp[(out_width-2):0]}; 
	
end


endmodule
