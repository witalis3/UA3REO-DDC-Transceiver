set_time_format -unit ns -decimal_places 3

create_clock -name "clock_crystal" -period 122.880MHz [get_ports {clk_sys_lvpecl_p}]
create_clock -name "clock_adc" -period 122.880MHz [get_ports {ADC_CLK}]
create_clock -name "clock_tcxo" -period 12.288MHz [get_ports {TCXO_CLK_IN}]
create_clock -name "clock_stm32" -period 25MHz [get_ports {STM32_CLK}]
create_clock -name "RX1_CICOMP_Q_clk" -period 96KHz {rx_ciccomp:RX1_CICOMP_Q|rx_ciccomp_0002:rx_ciccomp_inst|rx_ciccomp_0002_ast:rx_ciccomp_0002_ast_inst|auk_dspip_avalon_streaming_source_hpfir:source|data_valid}

set_clock_groups -asynchronous -group clock_crystal -group clock_adc -group clock_tcxo -group clock_stm32

derive_clock_uncertainty

derive_pll_clocks -create_base_clocks

set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -max 36ps [get_ports {DAC_CLK}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -min 0ps [get_ports {DAC_CLK}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -max 36ps [get_ports {DAC_OUTPUT[*]}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -min 0ps [get_ports {DAC_OUTPUT[*]}]

set_output_delay -clock clock_stm32 -max 36ps [get_ports {STM32_DATA_BUS[*]}]
set_output_delay -clock clock_stm32 -min 0ps [get_ports {STM32_DATA_BUS[*]}]

set_input_delay -clock clock_adc -max 36ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_adc -min 0ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_adc -max 36ps [get_ports ADC_OTR]
set_input_delay -clock clock_adc -min 0ps [get_ports ADC_OTR]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_SYNC]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_SYNC]

#set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_crystal}]
#set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_stm32}]
#set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_adc}]
#set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[2]}]
#set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
#set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {RX1_CICOMP_Q_clk}]
#set_false_path -from [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[2]}] -to [get_clocks {clock_stm32}]
#set_false_path -from [get_clocks {RX1_CICOMP_Q_clk}] -to [get_clocks {clock_stm32}]
#set_false_path -from [get_clocks {clock_adc}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
