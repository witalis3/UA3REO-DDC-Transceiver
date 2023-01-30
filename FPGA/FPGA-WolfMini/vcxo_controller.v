module vcxo_controller(
vcxo_clk_in,
tcxo_clk_in,
freq_error,
VCXO_correction,
pump
);

input vcxo_clk_in;
input tcxo_clk_in;
output pump;
output reg signed [31:0] freq_error = 0;
input signed [15:0] VCXO_correction;
wire ref_80khz;
wire osc_80khz;
reg out10 = 0;
reg [10:0] count10 = 0;
always @ (posedge tcxo_clk_in)
    begin
        if (count10 == $signed(VCXO_correction))
             begin
                 count10 <= 0;
                 out10 <= !out10;
             end
         else count10 <= count10 + 1'b1;
    end
assign ref_80khz = out10;
reg out122 = 0;
reg [10:0] count122 = 0;
always @ (posedge vcxo_clk_in)
    begin
        if (count122 == 11'd1535)   // для опорника 61,44 подставить 11'd767, для 96 Мгц - 11'd1199, для 122,88 - 11'd1535
             begin
                 count122 <= 0;
                 out122 <= !out122;
             end
         else count122 <= count122 + 1'b1;
    end
assign osc_80khz = out122;

//Apply to EXOR phase detector
assign pump = ref_80khz ^ osc_80khz;

endmodule
