module vcxo_controller(
vcxo_clk_in,
tcxo_clk_in,
pwm_clk_in,
VCXO_correction,

freq_error,
pump,
PWM
);

parameter VCXO_freq = 12288000; //x10hz
parameter TCXO_freq = 1228800; //x10hz

input vcxo_clk_in;
input tcxo_clk_in;
input pwm_clk_in;
input signed [7:0] VCXO_correction;

output reg signed [23:0] freq_error = 0;
output reg pump = 0;
output reg signed [15:0] PWM = 16000;

reg [15:0] PWM_max = 32000;
reg signed [23:0] freq_error_now = 0;
reg signed [23:0] freq_error_prev = 0;
reg signed [23:0] freq_error_diff = 0;
reg signed [31:0] VCXO_counter = 0;
reg signed [31:0] TCXO_counter = 0;
reg [7:0] vcxo_cnt_state = 0; //0 - idle , 1 - work , 2 - reset
reg counter_resetted = 0;
reg locked = 0;
reg [7:0] state = 0;

//reg [15:0] PWM_counter = 0;
reg [15:0] PWM_counter_on = 0;
reg [15:0] PWM_counter_off = 0;
reg PWM_bol_on = 0;
reg PWM_bol_off = 0;
reg PWM_raven = 0;
reg PWM_flash_state = 0;
reg PWM_flash_on = 0;
reg PWM_flash_off = 0;

always @ (posedge pwm_clk_in)
begin
	//do PWM
	/*PWM_counter = PWM_counter + 16'd1;
	
	if(PWM_counter < PWM_max) //do flash pump
	begin
		if((PWM_flash_state == 0 && PWM_counter_on <= PWM) || (PWM_counter_off >= (PWM_max - PWM)))
		begin
			pump = 1;
			PWM_flash_state = 1;
			PWM_counter_on = PWM_counter_on + 16'd1;
		end
		else
		begin
			pump = 0;
			PWM_flash_state = 0;
			PWM_counter_off = PWM_counter_off + 16'd1;
		end
	end
	else
	begin
		PWM_counter = 0;
		PWM_counter_on = 0;
		PWM_counter_off = 0;
	end*/
	
	//do PWM
	if(PWM_flash_state == 1)
		PWM_flash_state = 0;
	else
		PWM_flash_state = 1;
		
	if(PWM_counter_on == 0 && PWM_counter_off == 0) //reset
	begin
		PWM_counter_on = PWM;
		PWM_counter_off = PWM_max - PWM;
	end
	else
	begin
		//
		if((PWM_counter_on >> 1) >= PWM_counter_off)
			PWM_bol_on = 1;
		else
			PWM_bol_on = 0;
		//
		if((PWM_counter_off >> 1) >= PWM_counter_on)
			PWM_bol_off = 1;
		else
			PWM_bol_off = 0;
		//
		if(PWM_bol_on == 0 && PWM_bol_off == 0)
			PWM_raven = 1;
		else
			PWM_raven = 0;
		//
		if(PWM_raven == 1 && PWM_flash_state == 1 && PWM_counter_on > 0)
			PWM_flash_on = 1;
		else
			PWM_flash_on = 0;
		//
		if(PWM_raven == 1 && PWM_flash_state == 0 && PWM_counter_off > 0)
			PWM_flash_off = 1;
		else
			PWM_flash_off = 0;
		//
		if(PWM_bol_on == 1)
		begin
			PWM_counter_on = PWM_counter_on - 16'd1;
			pump = 1;
		end
		else if(PWM_flash_on == 1)
		begin
			PWM_counter_on = PWM_counter_on - 16'd1;
			pump = 1;
		end
		//
		if(PWM_bol_off == 1)
		begin
			PWM_counter_off = PWM_counter_off - 16'd1;
			pump = 0;
		end
		else if(PWM_flash_off == 1)
		begin
			PWM_counter_off = PWM_counter_off - 16'd1;
			pump = 0;
		end
	end
end

always @ (posedge vcxo_clk_in)
begin
	if(vcxo_cnt_state == 1)
	begin
		VCXO_counter = VCXO_counter + 1;
		counter_resetted = 0;
	end
	else if(vcxo_cnt_state == 2)
	begin
		VCXO_counter = 0;
		counter_resetted = 1;
	end
end

always @ (posedge tcxo_clk_in)
begin
	if(vcxo_cnt_state == 2 && !counter_resetted)
	begin
		//wait VCXO counter reset
	end
	else
	begin
		if(state == 0)
		begin
			TCXO_counter = 0;
			vcxo_cnt_state = 1; //work
			state = 1;
		end
		else if(state == 1)
		begin
			TCXO_counter = TCXO_counter + 1;
			
			if(TCXO_counter >= TCXO_freq)
			begin
				vcxo_cnt_state = 0; //idle
				state = 2;
			end
		end
		else if(state == 2)
		begin
			freq_error_now = VCXO_counter - VCXO_freq + VCXO_correction;	
			freq_error_diff = freq_error_prev - freq_error_now;
			state = 3;
		end
		else if(state == 3)
		begin
			if(freq_error_diff == 0 || (locked == 0 && freq_error_diff < 50 && freq_error_diff > -50)) //pricision if locked
			begin
				//save
				freq_error = freq_error_now;
				
				//get lock!
				if(freq_error_diff == 0 && freq_error_now == 0)
					locked = 1;	
						
				//tune
				if(freq_error_now < -50 && locked == 0) //extra coarse
					PWM = PWM + 16'd250;
				else if(freq_error_now < -10) //coarse
					PWM = PWM + 16'd50;
				else if(freq_error_now < 0) //fine
					PWM = PWM + 16'd1;
				else if(freq_error_now > 50 && locked == 0) //extra coarse
					PWM = PWM - 16'd250;
				else if(freq_error_now > 10) //coarse
					PWM = PWM - 16'd50;
				else if(freq_error_now > 0) //fine
					PWM = PWM - 16'd1;
			end
			state = 4;
		end
		else if(state == 4)
		begin
			freq_error_prev = freq_error_now;
			vcxo_cnt_state = 2; //reset
			if(PWM > PWM_max)
				PWM = PWM_max;
			if(PWM < 0)
				PWM = 0;
			state = 0;
		end
	end
end

endmodule
