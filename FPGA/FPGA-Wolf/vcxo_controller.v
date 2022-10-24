module vcxo_controller(
vcxo_clk_in,
tcxo_clk_in,
pwm_clk_in,
VCXO_correction,
tx,

freq_error,
pump,
PWM
);

parameter VCXO_freq = 12288000; //x10hz
parameter TCXO_freq = 1228800; //x10hz

input vcxo_clk_in;
input tcxo_clk_in;
input pwm_clk_in;
input signed [15:0] VCXO_correction;
input tx;

output reg signed [31:0] freq_error = 99999;
output reg pump = 0;
output reg signed [31:0] PWM = 20000;

//reg signed [31:0] PWM_RX = 20000;
//reg signed [31:0] PWM_TX = 20000;
reg signed [31:0] PWM_new = 20000;
reg signed [31:0] freq_error_now = 0;
reg [31:0] VCXO_counter = 0;
reg [31:0] VCXO_counter_result = 0;
reg [31:0] TCXO_counter = 0;
reg [2:0] vcxo_cnt_state = 0; //0 - idle , 1 - work , 2 - reset
reg [2:0] vcxo_cnt_need_state = 0; //0 - idle , 1 - work , 2 - reset
reg [7:0] state = 0;
reg counter_resetted = 0;
reg counter_idle = 0;
//reg tx_switched = 0;

reg signed [31:0] PWM_max = 40000;
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
	if(vcxo_cnt_state != vcxo_cnt_need_state)
	begin
		vcxo_cnt_state <= vcxo_cnt_need_state;
	end
	else 
	begin
		if(vcxo_cnt_state == 0 && counter_idle == 0) //idle, get results
		begin
			VCXO_counter_result <= VCXO_counter;
			counter_idle <= 1;
		end
		if(vcxo_cnt_state == 1) //count
		begin
			VCXO_counter <= VCXO_counter + 'd1;
			counter_resetted <= 0;
			counter_idle <= 0;
		end
		if(vcxo_cnt_state == 2 && counter_resetted == 0) //reset
		begin
			VCXO_counter <= 0;
			counter_resetted <= 1;
		end
	end
end

always @ (posedge tcxo_clk_in)
begin
	if((vcxo_cnt_state != vcxo_cnt_need_state)
		|| (vcxo_cnt_need_state == 2 && !counter_resetted)
		|| (vcxo_cnt_need_state == 0 && !counter_idle)
		|| (vcxo_cnt_need_state == 1 && (counter_idle || counter_resetted))
	)
	begin
		//wait VCXO counter state set
	end
	else
	begin
		if(state == 0)
		begin
			TCXO_counter <= 0;
			vcxo_cnt_need_state <= 1; //work
			state <= 1;
		end
		else if(state == 1)
		begin
			if(TCXO_counter >= TCXO_freq)
			begin
				vcxo_cnt_need_state <= 0; //idle
				state <= 2;
			end
			else
			begin
				TCXO_counter <= TCXO_counter + 1;
			end
		end
		else if(state == 2)
		begin
			freq_error_now <= VCXO_counter_result - VCXO_freq + $signed(VCXO_correction);
			state <= 3;
		end
		else if(state == 3)
		begin	
			//tune
			//if(tx == 0)
			//begin
			//	if ($signed(freq_error_now) < 10 || $signed(freq_error_now) > 10)
			//		PWM_new <= $signed(PWM_RX) - $signed(freq_error_now);
			//	else if ($signed(freq_error_now) < 0)
			//		PWM_new <= $signed(PWM_RX) + 1;
			//	else if ($signed(freq_error_now) > 0)
			//		PWM_new <= $signed(PWM_RX) + 1;
			//end
			//else if(tx == 1 && $signed(freq_error_now) < 0 || $signed(freq_error_now) > 0)
			//begin
			//	if ($signed(freq_error_now) < 10 || $signed(freq_error_now) > 10)
			//		PWM_new <= $signed(PWM_TX) - $signed(freq_error_now);
			//	else if ($signed(freq_error_now) < 0)
			//		PWM_new <= $signed(PWM_TX) + 1;
			//	else if ($signed(freq_error_now) > 0)
			//		PWM_new <= $signed(PWM_TX) + 1;
			//		
			//	tx_switched <= 1;
			//end
			
			if ($signed(freq_error_now) < 10 || $signed(freq_error_now) > 10)
				PWM_new <= $signed(PWM) - $signed(freq_error_now);
			else if ($signed(freq_error_now) < 0)
				PWM_new <= $signed(PWM) + 1;
			else if ($signed(freq_error_now) > 0)
				PWM_new <= $signed(PWM) + 1;
				
			freq_error <= freq_error_now;
			
			state = 4;
		end
		else if(state == 4)
		begin	
			if ($signed(PWM_new) > 1 && $signed(PWM_new) < $signed(PWM_max) && $signed(PWM) != $signed(PWM_new))
				PWM <= PWM_new;
			
			//if (tx == 0 && tx_switched == 0)
			//begin
			//	PWM_RX <= PWM_new;
			//	PWM_TX <= PWM_new;
			//end
			//else if (tx == 0 && tx_switched == 1)
			//	PWM_RX <= PWM_new;
			//else if (tx == 1)
			//	PWM_TX <= PWM_new;
				
			vcxo_cnt_need_state <= 2; //reset
			state <= 0;
		end
	end
end

endmodule
