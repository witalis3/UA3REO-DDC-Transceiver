module DAC_corrector(
clk_in,
DATA_IN,
DATA_OUT
);

input clk_in;
input signed [27:0] DATA_IN;
output reg unsigned [13:0] DATA_OUT;

reg signed [13:0] tmp=0;

always @ (posedge clk_in)
begin
	tmp[13:0]=DATA_IN[27:14]; //получаем 14 бит
	DATA_OUT[13:0]={~tmp[13],tmp[12:0]}; //инвертируем первый бит, получая unsigned из two's complement
end


endmodule
