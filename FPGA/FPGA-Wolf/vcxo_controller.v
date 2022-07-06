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
input signed [7:0] VCXO_correction;
input tx;

output reg signed [31:0] freq_error = 0;
output reg pump = 0;
output reg signed [31:0] PWM = 16000;

reg signed [31:0] freq_error_now = 0;
reg signed [31:0] freq_error_prev = 0;
reg signed [31:0] freq_error_diff = 0;
reg [31:0] VCXO_counter = 0;
reg [31:0] VCXO_counter_result = 0;
reg [31:0] TCXO_counter = 0;
reg [2:0] vcxo_cnt_state = 0; //0 - idle , 1 - work , 2 - reset
reg [2:0] vcxo_cnt_need_state = 0; //0 - idle , 1 - work , 2 - reset
reg [7:0] state = 0;
reg counter_resetted = 0;
reg counter_idle = 0;

reg [31:0] PWM_max = 32000;
reg [31:0] PWM_counter = 0;

always @ (posedge pwm_clk_in)
begin
	//do PWM
	PWM_counter = PWM_counter + 'd1;
	if(PWM_counter >= PWM_max)
		PWM_counter = 0;
		
	if(PWM_counter < $signed(PWM))
		pump = 1;
	else
		pump = 0;
end

always @ (posedge vcxo_clk_in)
begin
	if(vcxo_cnt_state != vcxo_cnt_need_state)
	begin
		vcxo_cnt_state = vcxo_cnt_need_state;
	end
	else if(vcxo_cnt_state == 0) //idle, get results
	begin
		VCXO_counter_result = VCXO_counter;
		counter_idle = 1;
	end
	else if(vcxo_cnt_state == 1) //count
	begin
		VCXO_counter = VCXO_counter + 'd1;
		counter_resetted = 0;
		counter_idle = 0;
	end
	else if(vcxo_cnt_state == 2) //reset
	begin
		VCXO_counter = 0;
		counter_resetted = 1;
	end
end

always @ (posedge tcxo_clk_in)
begin
	if(tx)
	begin
		vcxo_cnt_need_state = 0;
		state = 0;
	end
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
			TCXO_counter = 0;
			vcxo_cnt_need_state = 1; //work
			state = 1;
		end
		else if(state == 1)
		begin
			if(TCXO_counter >= TCXO_freq)
			begin
				vcxo_cnt_need_state = 0; //idle
				state = 2;
			end
			else
			begin
				TCXO_counter = TCXO_counter + 1;
			end
		end
		else if(state == 2)
		begin
			freq_error_now = VCXO_counter_result - VCXO_freq + $signed(VCXO_correction);
			freq_error_diff = freq_error_prev - freq_error_now;
			state = 3;
		end
		else if(state == 3)
		begin
			if(freq_error_diff == 0) //check mesure
			begin		
				//tune
				if(freq_error_now < -10 || freq_error_now > 10)
				begin
					PWM = $signed(PWM) - ($signed(freq_error_now) <<< 1);
					freq_error = freq_error_now;
				end
				else if(freq_error_now < -1 || freq_error_now > 1)
				begin
					PWM = $signed(PWM) - $signed(freq_error_now);
					freq_error = freq_error_now;
				end
				else
				begin
					freq_error = 0;
				end
			end
			
			if($signed(PWM) > PWM_max)
				PWM = $signed(PWM_max);
			if($signed(PWM) < $signed(1))
				PWM = $signed(1);
					
			state = 4;
		end
		else if(state == 4)
		begin
			freq_error_prev = freq_error_now;
			vcxo_cnt_need_state = 2; //reset
			state = 0;
		end
	end
end

endmodule
