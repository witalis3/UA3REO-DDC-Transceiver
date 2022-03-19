-- ------------------------------------------------------------------------- 
-- High Level Design Compiler for Intel(R) FPGAs Version 18.1 (Release Build #625)
-- Quartus Prime development tool and MATLAB/Simulink Interface
-- 
-- Legal Notice: Copyright 2018 Intel Corporation.  All rights reserved.
-- Your use of  Intel Corporation's design tools,  logic functions and other
-- software and  tools, and its AMPP partner logic functions, and any output
-- files any  of the foregoing (including  device programming  or simulation
-- files), and  any associated  documentation  or information  are expressly
-- subject  to the terms and  conditions of the  Intel FPGA Software License
-- Agreement, Intel MegaCore Function License Agreement, or other applicable
-- license agreement,  including,  without limitation,  that your use is for
-- the  sole  purpose of  programming  logic devices  manufactured by  Intel
-- and  sold by Intel  or its authorized  distributors. Please refer  to the
-- applicable agreement for further details.
-- ---------------------------------------------------------------------------

-- VHDL created from tx_ciccomp_0002_rtl_core
-- VHDL created on Sat Mar 19 12:41:58 2022


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.NUMERIC_STD.all;
use IEEE.MATH_REAL.all;
use std.TextIO.all;
use work.dspba_library_package.all;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;
LIBRARY lpm;
USE lpm.lpm_components.all;

entity tx_ciccomp_0002_rtl_core is
    port (
        xIn_v : in std_logic_vector(0 downto 0);  -- sfix1
        xIn_c : in std_logic_vector(7 downto 0);  -- sfix8
        xIn_0 : in std_logic_vector(23 downto 0);  -- sfix24
        xOut_v : out std_logic_vector(0 downto 0);  -- ufix1
        xOut_c : out std_logic_vector(7 downto 0);  -- ufix8
        xOut_0 : out std_logic_vector(39 downto 0);  -- sfix40
        clk : in std_logic;
        areset : in std_logic
    );
end tx_ciccomp_0002_rtl_core;

architecture normal of tx_ciccomp_0002_rtl_core is

    attribute altera_attribute : string;
    attribute altera_attribute of normal : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF; -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 10037; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 15400; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 12020; -name MESSAGE_DISABLE 12030; -name MESSAGE_DISABLE 12010; -name MESSAGE_DISABLE 12110; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 113007";
    
    signal GND_q : STD_LOGIC_VECTOR (0 downto 0);
    signal VCC_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_xIn_0_13_q : STD_LOGIC_VECTOR (23 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_13_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_count : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_run_preEnaQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_out : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_enableQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_ctrl : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_memread_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_compute_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_15_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_16_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_17_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_i : SIGNED (6 downto 0);
    attribute preserve : boolean;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count0_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count1_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count1_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count1_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_a : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_o : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_memr0_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr0_ia : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_aa : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_ab : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_iq : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_ca0_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_ca0_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_ca0_i : signal is true;
    signal u0_m0_wo0_cm0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_aseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_aseq_eq : std_logic;
    signal d_u0_m0_wo0_aseq_q_17_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_aseq_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_eq : std_logic;
    signal u0_m0_wo0_oseq_gated_reg_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multhi_a0 : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multhi_b0 : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multhi_s1 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multhi_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_replace_multhi_q : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multlo_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multlo_b0 : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multlo_s1 : STD_LOGIC_VECTOR (27 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_multlo_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_replace_multlo_q : STD_LOGIC_VECTOR (26 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16_q : STD_LOGIC_VECTOR (26 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_a : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_i : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_o : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_q : STD_LOGIC_VECTOR (18 downto 0);
    signal d_u0_m0_wo0_accum_p1_of_3_q_19_q : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_i : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_q : STD_LOGIC_VECTOR (18 downto 0);
    signal d_u0_m0_wo0_accum_p2_of_3_q_19_q : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_a : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_b : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_i : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_o : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_q : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_a : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_o : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_q : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q_18_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_gated_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_in : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_b : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_c : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_shiftup_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_shiftup_qint : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel0_0_b : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_0_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_BitJoin_for_q_q : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_b : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_BitJoin_for_c_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q : STD_LOGIC_VECTOR (1 downto 0);

begin


    -- VCC(CONSTANT,1)@0
    VCC_q <= "1";

    -- u0_m0_wo0_run(ENABLEGENERATOR,13)@10 + 2
    u0_m0_wo0_run_ctrl <= u0_m0_wo0_run_out & xIn_v & u0_m0_wo0_run_enableQ;
    u0_m0_wo0_run_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_run_enable_c : SIGNED(6 downto 0);
        variable u0_m0_wo0_run_inc : SIGNED(1 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_run_q <= "0";
            u0_m0_wo0_run_enable_c := TO_SIGNED(62, 7);
            u0_m0_wo0_run_enableQ <= "0";
            u0_m0_wo0_run_count <= "00";
            u0_m0_wo0_run_inc := (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_run_out = "1") THEN
                IF (u0_m0_wo0_run_enable_c(6) = '1') THEN
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c - (-63);
                ELSE
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c + (-1);
                END IF;
                u0_m0_wo0_run_enableQ <= STD_LOGIC_VECTOR(u0_m0_wo0_run_enable_c(6 downto 6));
            ELSE
                u0_m0_wo0_run_enableQ <= "0";
            END IF;
            CASE (u0_m0_wo0_run_ctrl) IS
                WHEN "000" | "001" => u0_m0_wo0_run_inc := "00";
                WHEN "010" | "011" => u0_m0_wo0_run_inc := "11";
                WHEN "100" => u0_m0_wo0_run_inc := "00";
                WHEN "101" => u0_m0_wo0_run_inc := "01";
                WHEN "110" => u0_m0_wo0_run_inc := "11";
                WHEN "111" => u0_m0_wo0_run_inc := "00";
                WHEN OTHERS => 
            END CASE;
            u0_m0_wo0_run_count <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_run_count) + SIGNED(u0_m0_wo0_run_inc));
            u0_m0_wo0_run_q <= u0_m0_wo0_run_out;
        END IF;
    END PROCESS;
    u0_m0_wo0_run_preEnaQ <= u0_m0_wo0_run_count(1 downto 1);
    u0_m0_wo0_run_out <= u0_m0_wo0_run_preEnaQ and VCC_q;

    -- u0_m0_wo0_memread(DELAY,14)@12
    u0_m0_wo0_memread : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_run_q, xout => u0_m0_wo0_memread_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_compute(DELAY,16)@12
    u0_m0_wo0_compute : dspba_delay
    GENERIC MAP ( width => 1, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => u0_m0_wo0_compute_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_15(DELAY,117)@12 + 3
    d_u0_m0_wo0_compute_q_15 : dspba_delay
    GENERIC MAP ( width => 1, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_compute_q, xout => d_u0_m0_wo0_compute_q_15_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_aseq(SEQUENCE,33)@15 + 1
    u0_m0_wo0_aseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_aseq_c : SIGNED(8 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_aseq_c := "000000000";
            u0_m0_wo0_aseq_q <= "0";
            u0_m0_wo0_aseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_15_q = "1") THEN
                IF (u0_m0_wo0_aseq_c = "000000000") THEN
                    u0_m0_wo0_aseq_eq <= '1';
                ELSE
                    u0_m0_wo0_aseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_aseq_eq = '1') THEN
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c + 63;
                ELSE
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c - 1;
                END IF;
                u0_m0_wo0_aseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_aseq_c(8 downto 8));
            END IF;
        END IF;
    END PROCESS;

    -- d_u0_m0_wo0_compute_q_16(DELAY,118)@15 + 1
    d_u0_m0_wo0_compute_q_16 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_15_q, xout => d_u0_m0_wo0_compute_q_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_ca0(COUNTER,27)@12
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_ca0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca0_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_compute_q = "1") THEN
                u0_m0_wo0_ca0_i <= u0_m0_wo0_ca0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca0_i, 6)));

    -- u0_m0_wo0_cm0(LOOKUP,31)@12 + 1
    u0_m0_wo0_cm0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_cm0_q <= "0000000000";
        ELSIF (clk'EVENT AND clk = '1') THEN
            CASE (u0_m0_wo0_ca0_q) IS
                WHEN "000000" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000001" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000010" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000011" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000100" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000101" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000110" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "000111" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "001000" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "001001" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "001010" => u0_m0_wo0_cm0_q <= "1111111111";
                WHEN "001011" => u0_m0_wo0_cm0_q <= "0000000010";
                WHEN "001100" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "001101" => u0_m0_wo0_cm0_q <= "0000000011";
                WHEN "001110" => u0_m0_wo0_cm0_q <= "1111111101";
                WHEN "001111" => u0_m0_wo0_cm0_q <= "0000000011";
                WHEN "010000" => u0_m0_wo0_cm0_q <= "1111111111";
                WHEN "010001" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "010010" => u0_m0_wo0_cm0_q <= "0000001000";
                WHEN "010011" => u0_m0_wo0_cm0_q <= "1111110000";
                WHEN "010100" => u0_m0_wo0_cm0_q <= "0000011001";
                WHEN "010101" => u0_m0_wo0_cm0_q <= "1111011101";
                WHEN "010110" => u0_m0_wo0_cm0_q <= "0000101010";
                WHEN "010111" => u0_m0_wo0_cm0_q <= "1111010010";
                WHEN "011000" => u0_m0_wo0_cm0_q <= "0000101010";
                WHEN "011001" => u0_m0_wo0_cm0_q <= "1111100101";
                WHEN "011010" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "011011" => u0_m0_wo0_cm0_q <= "0000110011";
                WHEN "011100" => u0_m0_wo0_cm0_q <= "1110000011";
                WHEN "011101" => u0_m0_wo0_cm0_q <= "0011100110";
                WHEN "011110" => u0_m0_wo0_cm0_q <= "1010001000";
                WHEN "011111" => u0_m0_wo0_cm0_q <= "0111111111";
                WHEN "100000" => u0_m0_wo0_cm0_q <= "0111111111";
                WHEN "100001" => u0_m0_wo0_cm0_q <= "1010001000";
                WHEN "100010" => u0_m0_wo0_cm0_q <= "0011100110";
                WHEN "100011" => u0_m0_wo0_cm0_q <= "1110000011";
                WHEN "100100" => u0_m0_wo0_cm0_q <= "0000110011";
                WHEN "100101" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "100110" => u0_m0_wo0_cm0_q <= "1111100101";
                WHEN "100111" => u0_m0_wo0_cm0_q <= "0000101010";
                WHEN "101000" => u0_m0_wo0_cm0_q <= "1111010010";
                WHEN "101001" => u0_m0_wo0_cm0_q <= "0000101010";
                WHEN "101010" => u0_m0_wo0_cm0_q <= "1111011101";
                WHEN "101011" => u0_m0_wo0_cm0_q <= "0000011001";
                WHEN "101100" => u0_m0_wo0_cm0_q <= "1111110000";
                WHEN "101101" => u0_m0_wo0_cm0_q <= "0000001000";
                WHEN "101110" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "101111" => u0_m0_wo0_cm0_q <= "1111111111";
                WHEN "110000" => u0_m0_wo0_cm0_q <= "0000000011";
                WHEN "110001" => u0_m0_wo0_cm0_q <= "1111111101";
                WHEN "110010" => u0_m0_wo0_cm0_q <= "0000000011";
                WHEN "110011" => u0_m0_wo0_cm0_q <= "1111111110";
                WHEN "110100" => u0_m0_wo0_cm0_q <= "0000000010";
                WHEN "110101" => u0_m0_wo0_cm0_q <= "1111111111";
                WHEN "110110" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "110111" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111000" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111001" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111010" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111011" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111100" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111101" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111110" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN "111111" => u0_m0_wo0_cm0_q <= "0000000000";
                WHEN OTHERS => -- unreachable
                               u0_m0_wo0_cm0_q <= (others => '-');
            END CASE;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra0_count1(COUNTER,22)@12
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count1_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count1_i <= u0_m0_wo0_wi0_r0_ra0_count1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count1_i, 7)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_inner(COUNTER,19)@12
    -- low=-1, high=62, step=-1, init=62
    u0_m0_wo0_wi0_r0_ra0_count0_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= TO_SIGNED(62, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count0_inner_i(6 downto 6) = "1") THEN
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 65;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_inner_i, 7)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_run(LOGICAL,20)@12
    u0_m0_wo0_wi0_r0_ra0_count0_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count0_inner_q(6 downto 6));

    -- u0_m0_wo0_wi0_r0_ra0_count0(COUNTER,21)@12
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count0_i <= u0_m0_wo0_wi0_r0_ra0_count0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_i, 7)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0(ADD,23)@12 + 1
    u0_m0_wo0_wi0_r0_ra0_add_0_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra0_count0_q);
    u0_m0_wo0_wi0_r0_ra0_add_0_0_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra0_count1_q);
    u0_m0_wo0_wi0_r0_ra0_add_0_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_0_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_0_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_0_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_0_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_add_0_0_q <= u0_m0_wo0_wi0_r0_ra0_add_0_0_o(7 downto 0);

    -- u0_m0_wo0_wi0_r0_ra0_resize(BITSELECT,24)@13
    u0_m0_wo0_wi0_r0_ra0_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_q(5 downto 0));
    u0_m0_wo0_wi0_r0_ra0_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_resize_in(5 downto 0));

    -- d_xIn_0_13(DELAY,115)@10 + 3
    d_xIn_0_13 : dspba_delay
    GENERIC MAP ( width => 24, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_0, xout => d_xIn_0_13_q, clk => clk, aclr => areset );

    -- d_in0_m0_wi0_wo0_assign_id1_q_13(DELAY,116)@10 + 3
    d_in0_m0_wi0_wo0_assign_id1_q_13 : dspba_delay
    GENERIC MAP ( width => 1, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_v, xout => d_in0_m0_wi0_wo0_assign_id1_q_13_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_wi0_r0_wa0(COUNTER,25)@13
    -- low=0, high=63, step=1, init=63
    u0_m0_wo0_wi0_r0_wa0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa0_i <= TO_UNSIGNED(63, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_in0_m0_wi0_wo0_assign_id1_q_13_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa0_i <= u0_m0_wo0_wi0_r0_wa0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa0_i, 6)));

    -- u0_m0_wo0_wi0_r0_memr0(DUALMEM,26)@13
    u0_m0_wo0_wi0_r0_memr0_ia <= STD_LOGIC_VECTOR(d_xIn_0_13_q);
    u0_m0_wo0_wi0_r0_memr0_aa <= u0_m0_wo0_wi0_r0_wa0_q;
    u0_m0_wo0_wi0_r0_memr0_ab <= u0_m0_wo0_wi0_r0_ra0_resize_b;
    u0_m0_wo0_wi0_r0_memr0_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 24,
        widthad_a => 6,
        numwords_a => 64,
        width_b => 24,
        widthad_b => 6,
        numwords_b => 64,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        address_reg_b => "CLOCK0",
        indata_reg_b => "CLOCK0",
        wrcontrol_wraddress_reg_b => "CLOCK0",
        rdcontrol_reg_b => "CLOCK0",
        byteena_reg_b => "CLOCK0",
        outdata_reg_b => "CLOCK0",
        outdata_aclr_b => "NONE",
        clock_enable_input_a => "NORMAL",
        clock_enable_input_b => "NORMAL",
        clock_enable_output_b => "NORMAL",
        read_during_write_mode_mixed_ports => "DONT_CARE",
        power_up_uninitialized => "FALSE",
        init_file => "UNUSED",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        clock0 => clk,
        address_a => u0_m0_wo0_wi0_r0_memr0_aa,
        data_a => u0_m0_wo0_wi0_r0_memr0_ia,
        wren_a => d_in0_m0_wi0_wo0_assign_id1_q_13_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr0_ab,
        q_b => u0_m0_wo0_wi0_r0_memr0_iq
    );
    u0_m0_wo0_wi0_r0_memr0_q <= u0_m0_wo0_wi0_r0_memr0_iq(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select(BITSELECT,113)@13
    u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr0_q(23 downto 17));
    u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr0_q(16 downto 0));

    -- u0_m0_wo0_mtree_mult1_0_replace_multlo(MULT,46)@13 + 2
    u0_m0_wo0_mtree_mult1_0_replace_multlo_a0 <= '0' & u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_c;
    u0_m0_wo0_mtree_mult1_0_replace_multlo_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_q);
    u0_m0_wo0_mtree_mult1_0_replace_multlo_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_replace_multlo_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 10,
        lpm_widthp => 28,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_replace_multlo_a0,
        datab => u0_m0_wo0_mtree_mult1_0_replace_multlo_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_replace_multlo_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_replace_multlo_s1
    );
    u0_m0_wo0_mtree_mult1_0_replace_multlo_q <= u0_m0_wo0_mtree_mult1_0_replace_multlo_s1(26 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel0_0(BITSELECT,102)@15
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel0_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_replace_multlo_q(18 downto 0));

    -- u0_m0_wo0_mtree_mult1_0_replace_multhi(MULT,45)@13 + 2
    u0_m0_wo0_mtree_mult1_0_replace_multhi_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_replace_split_high_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_replace_multhi_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_q);
    u0_m0_wo0_mtree_mult1_0_replace_multhi_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_replace_multhi_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 7,
        lpm_widthb => 10,
        lpm_widthp => 17,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_replace_multhi_a0,
        datab => u0_m0_wo0_mtree_mult1_0_replace_multhi_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_replace_multhi_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_replace_multhi_s1
    );
    u0_m0_wo0_mtree_mult1_0_replace_multhi_q <= u0_m0_wo0_mtree_mult1_0_replace_multhi_s1;

    -- u0_m0_wo0_mtree_mult1_0_replace_shiftup(BITSHIFT,47)@15
    u0_m0_wo0_mtree_mult1_0_replace_shiftup_qint <= u0_m0_wo0_mtree_mult1_0_replace_multhi_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_0_replace_shiftup_q <= u0_m0_wo0_mtree_mult1_0_replace_shiftup_qint(33 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select(BITSELECT,114)@15
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_replace_shiftup_q(18 downto 0));
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_replace_shiftup_q(33 downto 19));

    -- u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2(ADD,67)@15 + 1
    u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel0_0_b);
    u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_c(0) <= u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_o(19);
    u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_q <= u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_o(18 downto 0);

    -- u0_m0_wo0_accum_p1_of_3(ADD,58)@16 + 1
    u0_m0_wo0_accum_p1_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_q);
    u0_m0_wo0_accum_p1_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p1_of_3_q);
    u0_m0_wo0_accum_p1_of_3_i <= u0_m0_wo0_accum_p1_of_3_a;
    u0_m0_wo0_accum_p1_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p1_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_16_q = "1") THEN
                IF (u0_m0_wo0_aseq_q = "1") THEN
                    u0_m0_wo0_accum_p1_of_3_o <= u0_m0_wo0_accum_p1_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p1_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p1_of_3_a) + UNSIGNED(u0_m0_wo0_accum_p1_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p1_of_3_c(0) <= u0_m0_wo0_accum_p1_of_3_o(19);
    u0_m0_wo0_accum_p1_of_3_q <= u0_m0_wo0_accum_p1_of_3_o(18 downto 0);

    -- d_u0_m0_wo0_aseq_q_17(DELAY,121)@16 + 1
    d_u0_m0_wo0_aseq_q_17 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_aseq_q, xout => d_u0_m0_wo0_aseq_q_17_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_17(DELAY,119)@16 + 1
    d_u0_m0_wo0_compute_q_17 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_16_q, xout => d_u0_m0_wo0_compute_q_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel1_1(BITSELECT,73)@17
    u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q(14 downto 14));

    -- d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16(DELAY,123)@15 + 1
    d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16 : dspba_delay
    GENERIC MAP ( width => 27, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_replace_multlo_q, xout => d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1(BITSELECT,105)@16
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16_q(26 downto 26));

    -- u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_0(BITSELECT,104)@16
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_replace_multlo_q_16_q(26 downto 19));

    -- u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_BitJoin_for_c(BITJOIN,112)@16
    u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_BitJoin_for_c_q <= u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_tessel1_0_b;

    -- d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16(DELAY,127)@15 + 1
    d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c, xout => d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2(ADD,68)@16 + 1
    u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_cin <= u0_m0_wo0_mtree_mult1_0_replace_add_p1_of_2_c;
    u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((15 downto 15 => d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16_q(14)) & d_u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_a_tessel0_0_merged_bit_select_c_16_q) & '1');
    u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((15 downto 15 => u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_BitJoin_for_c_q(14)) & u0_m0_wo0_mtree_mult1_0_replace_add_BitSelect_for_b_BitJoin_for_c_q) & u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_cin(0));
    u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q <= u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_o(15 downto 1);

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c(BITJOIN,77)@17
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q;

    -- u0_m0_wo0_accum_p2_of_3(ADD,59)@17 + 1
    u0_m0_wo0_accum_p2_of_3_cin <= u0_m0_wo0_accum_p1_of_3_c;
    u0_m0_wo0_accum_p2_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q) & '1';
    u0_m0_wo0_accum_p2_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p2_of_3_q) & u0_m0_wo0_accum_p2_of_3_cin(0);
    u0_m0_wo0_accum_p2_of_3_i <= u0_m0_wo0_accum_p2_of_3_a;
    u0_m0_wo0_accum_p2_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p2_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_17_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_17_q = "1") THEN
                    u0_m0_wo0_accum_p2_of_3_o <= u0_m0_wo0_accum_p2_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p2_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p2_of_3_a) + UNSIGNED(u0_m0_wo0_accum_p2_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p2_of_3_c(0) <= u0_m0_wo0_accum_p2_of_3_o(20);
    u0_m0_wo0_accum_p2_of_3_q <= u0_m0_wo0_accum_p2_of_3_o(19 downto 1);

    -- d_u0_m0_wo0_aseq_q_18(DELAY,122)@17 + 1
    d_u0_m0_wo0_aseq_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_aseq_q_17_q, xout => d_u0_m0_wo0_aseq_q_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_18(DELAY,120)@17 + 1
    d_u0_m0_wo0_compute_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_17_q, xout => d_u0_m0_wo0_compute_q_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q_18(DELAY,126)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q_18 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q, xout => d_u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel2_0(BITSELECT,78)@18
    u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_replace_add_p2_of_2_q_18_q(14 downto 14));

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d(BITJOIN,80)@18
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b;

    -- u0_m0_wo0_accum_p3_of_3(ADD,60)@18 + 1
    u0_m0_wo0_accum_p3_of_3_cin <= u0_m0_wo0_accum_p2_of_3_c;
    u0_m0_wo0_accum_p3_of_3_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((2 downto 2 => u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q(1)) & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q) & '1');
    u0_m0_wo0_accum_p3_of_3_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((2 downto 2 => u0_m0_wo0_accum_p3_of_3_q(1)) & u0_m0_wo0_accum_p3_of_3_q) & u0_m0_wo0_accum_p3_of_3_cin(0));
    u0_m0_wo0_accum_p3_of_3_i <= u0_m0_wo0_accum_p3_of_3_a;
    u0_m0_wo0_accum_p3_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p3_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_18_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_18_q = "1") THEN
                    u0_m0_wo0_accum_p3_of_3_o <= u0_m0_wo0_accum_p3_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p3_of_3_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_accum_p3_of_3_a) + SIGNED(u0_m0_wo0_accum_p3_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p3_of_3_q <= u0_m0_wo0_accum_p3_of_3_o(2 downto 1);

    -- d_u0_m0_wo0_accum_p2_of_3_q_19(DELAY,125)@18 + 1
    d_u0_m0_wo0_accum_p2_of_3_q_19 : dspba_delay
    GENERIC MAP ( width => 19, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p2_of_3_q, xout => d_u0_m0_wo0_accum_p2_of_3_q_19_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_accum_p1_of_3_q_19(DELAY,124)@17 + 2
    d_u0_m0_wo0_accum_p1_of_3_q_19 : dspba_delay
    GENERIC MAP ( width => 19, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_3_q, xout => d_u0_m0_wo0_accum_p1_of_3_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitJoin_for_q(BITJOIN,61)@19
    u0_m0_wo0_accum_BitJoin_for_q_q <= u0_m0_wo0_accum_p3_of_3_q & d_u0_m0_wo0_accum_p2_of_3_q_19_q & d_u0_m0_wo0_accum_p1_of_3_q_19_q;

    -- GND(CONSTANT,0)@0
    GND_q <= "0";

    -- u0_m0_wo0_oseq(SEQUENCE,35)@17 + 1
    u0_m0_wo0_oseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_oseq_c : SIGNED(8 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_c := "000111111";
            u0_m0_wo0_oseq_q <= "0";
            u0_m0_wo0_oseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_17_q = "1") THEN
                IF (u0_m0_wo0_oseq_c = "000000000") THEN
                    u0_m0_wo0_oseq_eq <= '1';
                ELSE
                    u0_m0_wo0_oseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_oseq_eq = '1') THEN
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c + 63;
                ELSE
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c - 1;
                END IF;
                u0_m0_wo0_oseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_c(8 downto 8));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_oseq_gated(LOGICAL,36)@18
    u0_m0_wo0_oseq_gated_q <= u0_m0_wo0_oseq_q and d_u0_m0_wo0_compute_q_18_q;

    -- u0_m0_wo0_oseq_gated_reg(REG,37)@18 + 1
    u0_m0_wo0_oseq_gated_reg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= "0";
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_gated_q);
        END IF;
    END PROCESS;

    -- xOut(PORTOUT,42)@19 + 1
    xOut_v <= u0_m0_wo0_oseq_gated_reg_q;
    xOut_c <= STD_LOGIC_VECTOR("0000000" & GND_q);
    xOut_0 <= u0_m0_wo0_accum_BitJoin_for_q_q;

END normal;
