# clocks for PowerPlay analysis
create_clock -period 30 -name CLOCK_50 [get_ports CLOCK_50]
set_input_delay -clock CLOCK_50 -max 3 [all_inputs]
set_input_delay -clock CLOCK_50 -min 2 [all_inputs]

derive_pll_clocks
derive_clock_uncertainty
