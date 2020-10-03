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

-- VHDL created from tx_ciccomp_rtl_core
-- VHDL created on Sun Oct 04 02:14:11 2020


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

entity tx_ciccomp_rtl_core is
    port (
        xIn_v : in std_logic_vector(0 downto 0);  -- sfix1
        xIn_c : in std_logic_vector(7 downto 0);  -- sfix8
        xIn_0 : in std_logic_vector(31 downto 0);  -- sfix32
        xOut_v : out std_logic_vector(0 downto 0);  -- ufix1
        xOut_c : out std_logic_vector(7 downto 0);  -- ufix8
        xOut_0 : out std_logic_vector(63 downto 0);  -- sfix64
        clk : in std_logic;
        areset : in std_logic
    );
end tx_ciccomp_rtl_core;

architecture normal of tx_ciccomp_rtl_core is

    attribute altera_attribute : string;
    attribute altera_attribute of normal : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF; -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 10037; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 15400; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 12020; -name MESSAGE_DISABLE 12030; -name MESSAGE_DISABLE 12010; -name MESSAGE_DISABLE 12110; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 113007";
    
    signal GND_q : STD_LOGIC_VECTOR (0 downto 0);
    signal VCC_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_xIn_0_13_q : STD_LOGIC_VECTOR (31 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_13_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_count : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_run_preEnaQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_out : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_enableQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_ctrl : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_memread_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_compute_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_22_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_23_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_24_q : STD_LOGIC_VECTOR (0 downto 0);
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
    signal u0_m0_wo0_wi0_r0_memr0_ia : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_aa : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_ab : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_iq : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_ca0_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_ca0_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_ca0_i : signal is true;
    signal u0_m0_wo0_cm0_q : STD_LOGIC_VECTOR (25 downto 0);
    signal u0_m0_wo0_aseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_aseq_eq : std_logic;
    signal d_u0_m0_wo0_aseq_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_aseq_q_22_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_aseq_q_23_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_aseq_q_24_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_eq : std_logic;
    signal u0_m0_wo0_oseq_gated_reg_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a_0_b_16_q : STD_LOGIC_VECTOR (16 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a_1_b_17_q : STD_LOGIC_VECTOR (16 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_b_0_b_16_q : STD_LOGIC_VECTOR (16 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_b_1_b_17_q : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b0_a0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b0_b0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b0_s1 : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b0_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_a0_b0_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b0_a0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b0_b0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b0_s1 : STD_LOGIC_VECTOR (34 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b0_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_a1_b0_q : STD_LOGIC_VECTOR (33 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16_q : STD_LOGIC_VECTOR (33 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b1_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b1_b0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b1_s1 : STD_LOGIC_VECTOR (34 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a0_b1_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_a0_b1_q : STD_LOGIC_VECTOR (33 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16_q : STD_LOGIC_VECTOR (33 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b1_a0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b1_b0 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b1_s1 : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a1_b1_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_a1_b1_q : STD_LOGIC_VECTOR (33 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_align_1_q_19_q : STD_LOGIC_VECTOR (51 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_align_1_q_20_q : STD_LOGIC_VECTOR (51 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_align_1_q_21_q : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_a : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_i : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_o : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p1_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_accum_p1_of_5_q_25_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_i : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_accum_p2_of_5_q_25_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_i : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p3_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_accum_p3_of_5_q_25_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_i : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p4_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_accum_p4_of_5_q_25_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_a : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_b : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_i : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_o : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p5_of_5_q : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_a : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_o : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q_18_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q_18_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_a : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_o : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_q : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q : STD_LOGIC_VECTOR (18 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_a : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_o : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_a : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_b : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_o : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_a : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_o : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q_24_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d_20_q : STD_LOGIC_VECTOR (3 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c_19_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e_22_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d_21_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c_20_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_gated_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_in : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_b : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_b_0_in : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_b_0_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_b_1_in : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_b_1_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a_0_in : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a_0_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a_1_in : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_a_1_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_0_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_0_qint : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel0_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel1_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_0_b : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel0_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel1_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_0_b : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_2_q : STD_LOGIC_VECTOR (67 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_2_qint : STD_LOGIC_VECTOR (67 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel1_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel2_0_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_0_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_BitJoin_for_q_q : STD_LOGIC_VECTOR (63 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitJoin_for_q_q : STD_LOGIC_VECTOR (34 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_b : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_c : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_e_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel3_0_b : STD_LOGIC_VECTOR (12 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel3_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_BitJoin_for_d_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_BitJoin_for_d_q : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_BitJoin_for_d_q : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in : STD_LOGIC_VECTOR (59 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_BitJoin_for_e_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_1_q : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_1_qint : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_e_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_f_q : STD_LOGIC_VECTOR (3 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel0_0_b : STD_LOGIC_VECTOR (14 downto 0);

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

    -- d_u0_m0_wo0_compute_q_19(DELAY,223)@12 + 7
    d_u0_m0_wo0_compute_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 7, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_compute_q, xout => d_u0_m0_wo0_compute_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_aseq(SEQUENCE,33)@19 + 1
    u0_m0_wo0_aseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_aseq_c : SIGNED(8 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_aseq_c := "000000000";
            u0_m0_wo0_aseq_q <= "0";
            u0_m0_wo0_aseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_19_q = "1") THEN
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

    -- d_u0_m0_wo0_compute_q_20(DELAY,224)@19 + 1
    d_u0_m0_wo0_compute_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_19_q, xout => d_u0_m0_wo0_compute_q_20_q, clk => clk, aclr => areset );

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

    -- d_xIn_0_13(DELAY,221)@10 + 3
    d_xIn_0_13 : dspba_delay
    GENERIC MAP ( width => 32, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_0, xout => d_xIn_0_13_q, clk => clk, aclr => areset );

    -- d_in0_m0_wi0_wo0_assign_id1_q_13(DELAY,222)@10 + 3
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
        width_a => 32,
        widthad_a => 6,
        numwords_a => 64,
        width_b => 32,
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
    u0_m0_wo0_wi0_r0_memr0_q <= u0_m0_wo0_wi0_r0_memr0_iq(31 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_b_1(BITSELECT,46)@13
    u0_m0_wo0_mtree_mult1_0_b_1_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((33 downto 32 => u0_m0_wo0_wi0_r0_memr0_q(31)) & u0_m0_wo0_wi0_r0_memr0_q));
    u0_m0_wo0_mtree_mult1_0_b_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_b_1_in(33 downto 17));

    -- d_u0_m0_wo0_mtree_mult1_0_b_1_b_17(DELAY,236)@13 + 4
    d_u0_m0_wo0_mtree_mult1_0_b_1_b_17 : dspba_delay
    GENERIC MAP ( width => 17, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_b_1_b, xout => d_u0_m0_wo0_mtree_mult1_0_b_1_b_17_q, clk => clk, aclr => areset );

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
            u0_m0_wo0_cm0_q <= "00000000000000000010011001";
        ELSIF (clk'EVENT AND clk = '1') THEN
            CASE (u0_m0_wo0_ca0_q) IS
                WHEN "000000" => u0_m0_wo0_cm0_q <= "00000000000000000010011001";
                WHEN "000001" => u0_m0_wo0_cm0_q <= "11111111111111111011000101";
                WHEN "000010" => u0_m0_wo0_cm0_q <= "00000000000000001000010010";
                WHEN "000011" => u0_m0_wo0_cm0_q <= "11111111111111110011110000";
                WHEN "000100" => u0_m0_wo0_cm0_q <= "00000000000000001111110001";
                WHEN "000101" => u0_m0_wo0_cm0_q <= "11111111111111101111111111";
                WHEN "000110" => u0_m0_wo0_cm0_q <= "00000000000000000111001110";
                WHEN "000111" => u0_m0_wo0_cm0_q <= "00000000000000010100111100";
                WHEN "001000" => u0_m0_wo0_cm0_q <= "11111111111110101010100101";
                WHEN "001001" => u0_m0_wo0_cm0_q <= "00000000000011010011111111";
                WHEN "001010" => u0_m0_wo0_cm0_q <= "11111111111001001001111110";
                WHEN "001011" => u0_m0_wo0_cm0_q <= "00000000001100101110111000";
                WHEN "001100" => u0_m0_wo0_cm0_q <= "11111111101001111100101101";
                WHEN "001101" => u0_m0_wo0_cm0_q <= "00000000100100001010110110";
                WHEN "001110" => u0_m0_wo0_cm0_q <= "11111111000111001011110000";
                WHEN "001111" => u0_m0_wo0_cm0_q <= "00000001010110000100110000";
                WHEN "010000" => u0_m0_wo0_cm0_q <= "11111110000001100001011100";
                WHEN "010001" => u0_m0_wo0_cm0_q <= "00000010110100111101000001";
                WHEN "010010" => u0_m0_wo0_cm0_q <= "11111100000010111111001011";
                WHEN "010011" => u0_m0_wo0_cm0_q <= "00000101011010101000101111";
                WHEN "010100" => u0_m0_wo0_cm0_q <= "11111000101101010011110100";
                WHEN "010101" => u0_m0_wo0_cm0_q <= "00001001101010110010101101";
                WHEN "010110" => u0_m0_wo0_cm0_q <= "11110011010101101110110011";
                WHEN "010111" => u0_m0_wo0_cm0_q <= "00010000011010001011000111";
                WHEN "011000" => u0_m0_wo0_cm0_q <= "11101010111000011001001001";
                WHEN "011001" => u0_m0_wo0_cm0_q <= "00011011000100100111001100";
                WHEN "011010" => u0_m0_wo0_cm0_q <= "11011101010001101001000011";
                WHEN "011011" => u0_m0_wo0_cm0_q <= "00101100110001110111010001";
                WHEN "011100" => u0_m0_wo0_cm0_q <= "11000101100001111111101011";
                WHEN "011101" => u0_m0_wo0_cm0_q <= "01001101101110111011101111";
                WHEN "011110" => u0_m0_wo0_cm0_q <= "10010111001010001011010111";
                WHEN "011111" => u0_m0_wo0_cm0_q <= "01111111111111111111111111";
                WHEN "100000" => u0_m0_wo0_cm0_q <= "01111111111111111111111111";
                WHEN "100001" => u0_m0_wo0_cm0_q <= "10010111001010001011010111";
                WHEN "100010" => u0_m0_wo0_cm0_q <= "01001101101110111011101111";
                WHEN "100011" => u0_m0_wo0_cm0_q <= "11000101100001111111101011";
                WHEN "100100" => u0_m0_wo0_cm0_q <= "00101100110001110111010001";
                WHEN "100101" => u0_m0_wo0_cm0_q <= "11011101010001101001000011";
                WHEN "100110" => u0_m0_wo0_cm0_q <= "00011011000100100111001100";
                WHEN "100111" => u0_m0_wo0_cm0_q <= "11101010111000011001001001";
                WHEN "101000" => u0_m0_wo0_cm0_q <= "00010000011010001011000111";
                WHEN "101001" => u0_m0_wo0_cm0_q <= "11110011010101101110110011";
                WHEN "101010" => u0_m0_wo0_cm0_q <= "00001001101010110010101101";
                WHEN "101011" => u0_m0_wo0_cm0_q <= "11111000101101010011110100";
                WHEN "101100" => u0_m0_wo0_cm0_q <= "00000101011010101000101111";
                WHEN "101101" => u0_m0_wo0_cm0_q <= "11111100000010111111001011";
                WHEN "101110" => u0_m0_wo0_cm0_q <= "00000010110100111101000001";
                WHEN "101111" => u0_m0_wo0_cm0_q <= "11111110000001100001011100";
                WHEN "110000" => u0_m0_wo0_cm0_q <= "00000001010110000100110000";
                WHEN "110001" => u0_m0_wo0_cm0_q <= "11111111000111001011110000";
                WHEN "110010" => u0_m0_wo0_cm0_q <= "00000000100100001010110110";
                WHEN "110011" => u0_m0_wo0_cm0_q <= "11111111101001111100101101";
                WHEN "110100" => u0_m0_wo0_cm0_q <= "00000000001100101110111000";
                WHEN "110101" => u0_m0_wo0_cm0_q <= "11111111111001001001111110";
                WHEN "110110" => u0_m0_wo0_cm0_q <= "00000000000011010011111111";
                WHEN "110111" => u0_m0_wo0_cm0_q <= "11111111111110101010100101";
                WHEN "111000" => u0_m0_wo0_cm0_q <= "00000000000000010100111100";
                WHEN "111001" => u0_m0_wo0_cm0_q <= "00000000000000000111001110";
                WHEN "111010" => u0_m0_wo0_cm0_q <= "11111111111111101111111111";
                WHEN "111011" => u0_m0_wo0_cm0_q <= "00000000000000001111110001";
                WHEN "111100" => u0_m0_wo0_cm0_q <= "11111111111111110011110000";
                WHEN "111101" => u0_m0_wo0_cm0_q <= "00000000000000001000010010";
                WHEN "111110" => u0_m0_wo0_cm0_q <= "11111111111111111011000101";
                WHEN "111111" => u0_m0_wo0_cm0_q <= "00000000000000000010011001";
                WHEN OTHERS => -- unreachable
                               u0_m0_wo0_cm0_q <= (others => '-');
            END CASE;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_mtree_mult1_0_a_1(BITSELECT,44)@13
    u0_m0_wo0_mtree_mult1_0_a_1_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((33 downto 26 => u0_m0_wo0_cm0_q(25)) & u0_m0_wo0_cm0_q));
    u0_m0_wo0_mtree_mult1_0_a_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_a_1_in(33 downto 17));

    -- d_u0_m0_wo0_mtree_mult1_0_a_1_b_17(DELAY,234)@13 + 4
    d_u0_m0_wo0_mtree_mult1_0_a_1_b_17 : dspba_delay
    GENERIC MAP ( width => 17, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_a_1_b, xout => d_u0_m0_wo0_mtree_mult1_0_a_1_b_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_a1_b1(MULT,50)@17 + 2
    u0_m0_wo0_mtree_mult1_0_a1_b1_a0 <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a_1_b_17_q);
    u0_m0_wo0_mtree_mult1_0_a1_b1_b0 <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_b_1_b_17_q);
    u0_m0_wo0_mtree_mult1_0_a1_b1_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_a1_b1_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 17,
        lpm_widthb => 17,
        lpm_widthp => 34,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_a1_b1_a0,
        datab => u0_m0_wo0_mtree_mult1_0_a1_b1_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_a1_b1_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_a1_b1_s1
    );
    u0_m0_wo0_mtree_mult1_0_a1_b1_q <= u0_m0_wo0_mtree_mult1_0_a1_b1_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_2(BITSHIFT,54)@19
    u0_m0_wo0_mtree_mult1_0_align_2_qint <= u0_m0_wo0_mtree_mult1_0_a1_b1_q & "0000000000000000000000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_2_q <= u0_m0_wo0_mtree_mult1_0_align_2_qint(67 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select(BITSELECT,219)@19
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_2_q(59 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in(14 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in(29 downto 15));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in(44 downto 30));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_in(59 downto 45));

    -- u0_m0_wo0_mtree_mult1_0_a_0(BITSELECT,43)@13
    u0_m0_wo0_mtree_mult1_0_a_0_in <= u0_m0_wo0_cm0_q(16 downto 0);
    u0_m0_wo0_mtree_mult1_0_a_0_b <= u0_m0_wo0_mtree_mult1_0_a_0_in(16 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_a0_b1(MULT,49)@13 + 2
    u0_m0_wo0_mtree_mult1_0_a0_b1_a0 <= '0' & u0_m0_wo0_mtree_mult1_0_a_0_b;
    u0_m0_wo0_mtree_mult1_0_a0_b1_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_b_1_b);
    u0_m0_wo0_mtree_mult1_0_a0_b1_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_a0_b1_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 17,
        lpm_widthp => 35,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_a0_b1_a0,
        datab => u0_m0_wo0_mtree_mult1_0_a0_b1_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_a0_b1_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_a0_b1_s1
    );
    u0_m0_wo0_mtree_mult1_0_a0_b1_q <= u0_m0_wo0_mtree_mult1_0_a0_b1_s1(33 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel0_0(BITSELECT,157)@15
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel0_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_a0_b1_q(14 downto 0));

    -- u0_m0_wo0_mtree_mult1_0_b_0(BITSELECT,45)@13
    u0_m0_wo0_mtree_mult1_0_b_0_in <= u0_m0_wo0_wi0_r0_memr0_q(16 downto 0);
    u0_m0_wo0_mtree_mult1_0_b_0_b <= u0_m0_wo0_mtree_mult1_0_b_0_in(16 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_a1_b0(MULT,48)@13 + 2
    u0_m0_wo0_mtree_mult1_0_a1_b0_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_a_1_b);
    u0_m0_wo0_mtree_mult1_0_a1_b0_b0 <= '0' & u0_m0_wo0_mtree_mult1_0_b_0_b;
    u0_m0_wo0_mtree_mult1_0_a1_b0_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_a1_b0_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 17,
        lpm_widthb => 18,
        lpm_widthp => 35,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_a1_b0_a0,
        datab => u0_m0_wo0_mtree_mult1_0_a1_b0_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_a1_b0_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_a1_b0_s1
    );
    u0_m0_wo0_mtree_mult1_0_a1_b0_q <= u0_m0_wo0_mtree_mult1_0_a1_b0_s1(33 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel0_0(BITSELECT,150)@15
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel0_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_a1_b0_q(14 downto 0));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3(ADD,80)@15 + 1
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel0_0_b);
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel0_0_b);
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_c(0) <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_o(15);
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_o(14 downto 0);

    -- d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16(DELAY,239)@15 + 1
    d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16 : dspba_delay
    GENERIC MAP ( width => 34, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_a0_b1_q, xout => d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel1_0(BITSELECT,159)@16
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel1_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16_q(29 downto 15));

    -- d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16(DELAY,237)@15 + 1
    d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16 : dspba_delay
    GENERIC MAP ( width => 34, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_a1_b0_q, xout => d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel1_0(BITSELECT,152)@16
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel1_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16_q(29 downto 15));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3(ADD,81)@16 + 1
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_cin <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_c;
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel1_0_b) & '1';
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel1_0_b) & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_cin(0);
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_c(0) <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_o(16);
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_o(15 downto 1);

    -- d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17(DELAY,240)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17 : dspba_delay
    GENERIC MAP ( width => 34, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_16_q, xout => d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_1(BITSELECT,162)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_1_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17_q(33 downto 33));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_0(BITSELECT,161)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a0_b1_q_17_q(33 downto 30));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_BitJoin_for_d(BITJOIN,163)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_BitJoin_for_d_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_1_b & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_tessel2_0_b;

    -- d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17(DELAY,238)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17 : dspba_delay
    GENERIC MAP ( width => 34, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_16_q, xout => d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_1(BITSELECT,155)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_1_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17_q(33 downto 33));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_0(BITSELECT,154)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_a1_b0_q_17_q(33 downto 30));

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_BitJoin_for_d(BITJOIN,156)@17
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_BitJoin_for_d_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_1_b & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_tessel2_0_b;

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3(ADD,82)@17 + 1
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_cin <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_c;
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((5 downto 5 => u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_BitJoin_for_d_q(4)) & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_a_BitJoin_for_d_q) & '1');
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((5 downto 5 => u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_BitJoin_for_d_q(4)) & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitSelect_for_b_BitJoin_for_d_q) & u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_cin(0));
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_o(5 downto 1);

    -- d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q_18(DELAY,249)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q_18 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q, xout => d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q_18(DELAY,248)@16 + 2
    d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q_18 : dspba_delay
    GENERIC MAP ( width => 15, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q, xout => d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitJoin_for_q(BITJOIN,83)@18
    u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitJoin_for_q_q <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p3_of_3_q & d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p2_of_3_q_18_q & d_u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_p1_of_3_q_18_q;

    -- u0_m0_wo0_mtree_mult1_0_align_1(BITSHIFT,53)@18
    u0_m0_wo0_mtree_mult1_0_align_1_qint <= u0_m0_wo0_mtree_mult1_0_addcol_1_add_0_0_BitJoin_for_q_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_1_q <= u0_m0_wo0_mtree_mult1_0_align_1_qint(51 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel0_0(BITSELECT,173)@18
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel0_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_1_q(14 downto 0));

    -- d_u0_m0_wo0_mtree_mult1_0_b_0_b_16(DELAY,235)@13 + 3
    d_u0_m0_wo0_mtree_mult1_0_b_0_b_16 : dspba_delay
    GENERIC MAP ( width => 17, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_b_0_b, xout => d_u0_m0_wo0_mtree_mult1_0_b_0_b_16_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_a_0_b_16(DELAY,233)@13 + 3
    d_u0_m0_wo0_mtree_mult1_0_a_0_b_16 : dspba_delay
    GENERIC MAP ( width => 17, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_a_0_b, xout => d_u0_m0_wo0_mtree_mult1_0_a_0_b_16_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_a0_b0(MULT,47)@16 + 2
    u0_m0_wo0_mtree_mult1_0_a0_b0_a0 <= d_u0_m0_wo0_mtree_mult1_0_a_0_b_16_q;
    u0_m0_wo0_mtree_mult1_0_a0_b0_b0 <= d_u0_m0_wo0_mtree_mult1_0_b_0_b_16_q;
    u0_m0_wo0_mtree_mult1_0_a0_b0_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_a0_b0_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 17,
        lpm_widthb => 17,
        lpm_widthp => 34,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "UNSIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_a0_b0_a0,
        datab => u0_m0_wo0_mtree_mult1_0_a0_b0_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_a0_b0_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_a0_b0_s1
    );
    u0_m0_wo0_mtree_mult1_0_a0_b0_q <= u0_m0_wo0_mtree_mult1_0_a0_b0_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_0(BITSHIFT,52)@18
    u0_m0_wo0_mtree_mult1_0_align_0_qint <= u0_m0_wo0_mtree_mult1_0_a0_b0_q;
    u0_m0_wo0_mtree_mult1_0_align_0_q <= u0_m0_wo0_mtree_mult1_0_align_0_qint(33 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select(BITSELECT,218)@18
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_0_q(14 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_0_q(29 downto 15));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_0_q(33 downto 30));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4(ADD,91)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel0_0_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_o(15);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_o(14 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5(ADD,104)@19 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_q);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_o(15);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_o(14 downto 0);

    -- u0_m0_wo0_accum_p1_of_5(ADD,66)@20 + 1
    u0_m0_wo0_accum_p1_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_q);
    u0_m0_wo0_accum_p1_of_5_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p1_of_5_q);
    u0_m0_wo0_accum_p1_of_5_i <= u0_m0_wo0_accum_p1_of_5_a;
    u0_m0_wo0_accum_p1_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p1_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_20_q = "1") THEN
                IF (u0_m0_wo0_aseq_q = "1") THEN
                    u0_m0_wo0_accum_p1_of_5_o <= u0_m0_wo0_accum_p1_of_5_i;
                ELSE
                    u0_m0_wo0_accum_p1_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p1_of_5_a) + UNSIGNED(u0_m0_wo0_accum_p1_of_5_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p1_of_5_c(0) <= u0_m0_wo0_accum_p1_of_5_o(15);
    u0_m0_wo0_accum_p1_of_5_q <= u0_m0_wo0_accum_p1_of_5_o(14 downto 0);

    -- d_u0_m0_wo0_aseq_q_21(DELAY,229)@20 + 1
    d_u0_m0_wo0_aseq_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_aseq_q, xout => d_u0_m0_wo0_aseq_q_21_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_21(DELAY,225)@20 + 1
    d_u0_m0_wo0_compute_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_20_q, xout => d_u0_m0_wo0_compute_q_21_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c_20(DELAY,253)@19 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c_20 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c_20_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_align_1_q_19(DELAY,241)@18 + 1
    d_u0_m0_wo0_mtree_mult1_0_align_1_q_19 : dspba_delay
    GENERIC MAP ( width => 52, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_align_1_q, xout => d_u0_m0_wo0_mtree_mult1_0_align_1_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel1_0(BITSELECT,175)@19
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel1_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_align_1_q_19_q(29 downto 15));

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c_19(DELAY,251)@18 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c_19 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4(ADD,92)@19 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_4_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_a <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_c_19_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel1_0_b) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_o(16);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_o(15 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5(ADD,105)@20 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_cin <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_5_c;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_b <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_c_20_q) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_o(16);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_o(15 downto 1);

    -- u0_m0_wo0_accum_p2_of_5(ADD,67)@21 + 1
    u0_m0_wo0_accum_p2_of_5_cin <= u0_m0_wo0_accum_p1_of_5_c;
    u0_m0_wo0_accum_p2_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_q) & '1';
    u0_m0_wo0_accum_p2_of_5_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p2_of_5_q) & u0_m0_wo0_accum_p2_of_5_cin(0);
    u0_m0_wo0_accum_p2_of_5_i <= u0_m0_wo0_accum_p2_of_5_a;
    u0_m0_wo0_accum_p2_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p2_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_21_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_21_q = "1") THEN
                    u0_m0_wo0_accum_p2_of_5_o <= u0_m0_wo0_accum_p2_of_5_i;
                ELSE
                    u0_m0_wo0_accum_p2_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p2_of_5_a) + UNSIGNED(u0_m0_wo0_accum_p2_of_5_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p2_of_5_c(0) <= u0_m0_wo0_accum_p2_of_5_o(16);
    u0_m0_wo0_accum_p2_of_5_q <= u0_m0_wo0_accum_p2_of_5_o(15 downto 1);

    -- d_u0_m0_wo0_aseq_q_22(DELAY,230)@21 + 1
    d_u0_m0_wo0_aseq_q_22 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_aseq_q_21_q, xout => d_u0_m0_wo0_aseq_q_22_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_22(DELAY,226)@21 + 1
    d_u0_m0_wo0_compute_q_22 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_21_q, xout => d_u0_m0_wo0_compute_q_22_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d_21(DELAY,254)@19 + 2
    d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d_21 : dspba_delay
    GENERIC MAP ( width => 15, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d_21_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_align_1_q_20(DELAY,242)@19 + 1
    d_u0_m0_wo0_mtree_mult1_0_align_1_q_20 : dspba_delay
    GENERIC MAP ( width => 52, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_mtree_mult1_0_align_1_q_19_q, xout => d_u0_m0_wo0_mtree_mult1_0_align_1_q_20_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel2_0(BITSELECT,177)@20
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel2_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_align_1_q_20_q(44 downto 30));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a(CONSTANT,85)@0
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q <= "0000000000000000000";

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select(BITSELECT,220)@20
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q(10 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q(18 downto 11));

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d_20(DELAY,252)@18 + 2
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d_20 : dspba_delay
    GENERIC MAP ( width => 4, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d_20_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_BitJoin_for_d(BITJOIN,170)@20
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_BitJoin_for_d_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_b & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel0_0_merged_bit_select_d_20_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4(ADD,93)@20 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_4_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_BitJoin_for_d_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel2_0_b) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_o(16);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_o(15 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5(ADD,106)@21 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_cin <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_5_c;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_b <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_d_21_q) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_o(16);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_o(15 downto 1);

    -- u0_m0_wo0_accum_p3_of_5(ADD,68)@22 + 1
    u0_m0_wo0_accum_p3_of_5_cin <= u0_m0_wo0_accum_p2_of_5_c;
    u0_m0_wo0_accum_p3_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_q) & '1';
    u0_m0_wo0_accum_p3_of_5_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p3_of_5_q) & u0_m0_wo0_accum_p3_of_5_cin(0);
    u0_m0_wo0_accum_p3_of_5_i <= u0_m0_wo0_accum_p3_of_5_a;
    u0_m0_wo0_accum_p3_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p3_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_22_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_22_q = "1") THEN
                    u0_m0_wo0_accum_p3_of_5_o <= u0_m0_wo0_accum_p3_of_5_i;
                ELSE
                    u0_m0_wo0_accum_p3_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p3_of_5_a) + UNSIGNED(u0_m0_wo0_accum_p3_of_5_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p3_of_5_c(0) <= u0_m0_wo0_accum_p3_of_5_o(16);
    u0_m0_wo0_accum_p3_of_5_q <= u0_m0_wo0_accum_p3_of_5_o(15 downto 1);

    -- d_u0_m0_wo0_aseq_q_23(DELAY,231)@22 + 1
    d_u0_m0_wo0_aseq_q_23 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_aseq_q_22_q, xout => d_u0_m0_wo0_aseq_q_23_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_23(DELAY,227)@22 + 1
    d_u0_m0_wo0_compute_q_23 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_22_q, xout => d_u0_m0_wo0_compute_q_23_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e_22(DELAY,255)@19 + 3
    d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e_22 : dspba_delay
    GENERIC MAP ( width => 15, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e_22_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1(BITSELECT,189)@22
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_q(7 downto 7));

    -- d_u0_m0_wo0_mtree_mult1_0_align_1_q_21(DELAY,243)@20 + 1
    d_u0_m0_wo0_mtree_mult1_0_align_1_q_21 : dspba_delay
    GENERIC MAP ( width => 52, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_mtree_mult1_0_align_1_q_20_q, xout => d_u0_m0_wo0_mtree_mult1_0_align_1_q_21_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_1(BITSELECT,180)@21
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_1_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_align_1_q_21_q(51 downto 51));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_0(BITSELECT,179)@21
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_align_1_q_21_q(51 downto 45));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_BitJoin_for_e(BITJOIN,181)@21
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_BitJoin_for_e_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_tessel3_0_b;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4(ADD,94)@21 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p3_of_4_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_tessel2_1_merged_bit_select_c) & '1');
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 8 => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_BitJoin_for_e_q(7)) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_BitJoin_for_e_q) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_cin(0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_o(8 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_e(BITJOIN,196)@22
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_e_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p4_of_4_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5(ADD,107)@22 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_cin <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p3_of_5_c;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_e_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_b <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel0_0_merged_bit_select_e_22_q) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_o(15 downto 1);

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel3_1(BITSELECT,117)@23
    u0_m0_wo0_accum_BitSelect_for_a_tessel3_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q(12 downto 12));

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel3_0(BITSELECT,116)@23
    u0_m0_wo0_accum_BitSelect_for_a_tessel3_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q(12 downto 0));

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_e(BITJOIN,119)@23
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_e_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel3_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel3_0_b;

    -- u0_m0_wo0_accum_p4_of_5(ADD,69)@23 + 1
    u0_m0_wo0_accum_p4_of_5_cin <= u0_m0_wo0_accum_p3_of_5_c;
    u0_m0_wo0_accum_p4_of_5_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_e_q) & '1';
    u0_m0_wo0_accum_p4_of_5_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_accum_p4_of_5_q) & u0_m0_wo0_accum_p4_of_5_cin(0);
    u0_m0_wo0_accum_p4_of_5_i <= u0_m0_wo0_accum_p4_of_5_a;
    u0_m0_wo0_accum_p4_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p4_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_23_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_23_q = "1") THEN
                    u0_m0_wo0_accum_p4_of_5_o <= u0_m0_wo0_accum_p4_of_5_i;
                ELSE
                    u0_m0_wo0_accum_p4_of_5_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p4_of_5_a) + UNSIGNED(u0_m0_wo0_accum_p4_of_5_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p4_of_5_c(0) <= u0_m0_wo0_accum_p4_of_5_o(16);
    u0_m0_wo0_accum_p4_of_5_q <= u0_m0_wo0_accum_p4_of_5_o(15 downto 1);

    -- d_u0_m0_wo0_aseq_q_24(DELAY,232)@23 + 1
    d_u0_m0_wo0_aseq_q_24 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_aseq_q_23_q, xout => d_u0_m0_wo0_aseq_q_24_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_24(DELAY,228)@23 + 1
    d_u0_m0_wo0_compute_q_24 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_23_q, xout => d_u0_m0_wo0_compute_q_24_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q_24(DELAY,250)@23 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q_24 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q_24_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel4_0(BITSELECT,120)@24
    u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p4_of_5_q_24_q(12 downto 12));

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_f(BITJOIN,124)@24
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_f_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel4_0_b;

    -- u0_m0_wo0_accum_p5_of_5(ADD,70)@24 + 1
    u0_m0_wo0_accum_p5_of_5_cin <= u0_m0_wo0_accum_p4_of_5_c;
    u0_m0_wo0_accum_p5_of_5_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((4 downto 4 => u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_f_q(3)) & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_f_q) & '1');
    u0_m0_wo0_accum_p5_of_5_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((4 downto 4 => u0_m0_wo0_accum_p5_of_5_q(3)) & u0_m0_wo0_accum_p5_of_5_q) & u0_m0_wo0_accum_p5_of_5_cin(0));
    u0_m0_wo0_accum_p5_of_5_i <= u0_m0_wo0_accum_p5_of_5_a;
    u0_m0_wo0_accum_p5_of_5_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p5_of_5_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_24_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_24_q = "1") THEN
                    u0_m0_wo0_accum_p5_of_5_o <= u0_m0_wo0_accum_p5_of_5_i;
                ELSE
                    u0_m0_wo0_accum_p5_of_5_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_accum_p5_of_5_a) + SIGNED(u0_m0_wo0_accum_p5_of_5_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p5_of_5_q <= u0_m0_wo0_accum_p5_of_5_o(4 downto 1);

    -- d_u0_m0_wo0_accum_p4_of_5_q_25(DELAY,247)@24 + 1
    d_u0_m0_wo0_accum_p4_of_5_q_25 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p4_of_5_q, xout => d_u0_m0_wo0_accum_p4_of_5_q_25_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_accum_p3_of_5_q_25(DELAY,246)@23 + 2
    d_u0_m0_wo0_accum_p3_of_5_q_25 : dspba_delay
    GENERIC MAP ( width => 15, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p3_of_5_q, xout => d_u0_m0_wo0_accum_p3_of_5_q_25_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_accum_p2_of_5_q_25(DELAY,245)@22 + 3
    d_u0_m0_wo0_accum_p2_of_5_q_25 : dspba_delay
    GENERIC MAP ( width => 15, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p2_of_5_q, xout => d_u0_m0_wo0_accum_p2_of_5_q_25_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_accum_p1_of_5_q_25(DELAY,244)@21 + 4
    d_u0_m0_wo0_accum_p1_of_5_q_25 : dspba_delay
    GENERIC MAP ( width => 15, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_5_q, xout => d_u0_m0_wo0_accum_p1_of_5_q_25_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitJoin_for_q(BITJOIN,71)@25
    u0_m0_wo0_accum_BitJoin_for_q_q <= u0_m0_wo0_accum_p5_of_5_q & d_u0_m0_wo0_accum_p4_of_5_q_25_q & d_u0_m0_wo0_accum_p3_of_5_q_25_q & d_u0_m0_wo0_accum_p2_of_5_q_25_q & d_u0_m0_wo0_accum_p1_of_5_q_25_q;

    -- GND(CONSTANT,0)@0
    GND_q <= "0";

    -- u0_m0_wo0_oseq(SEQUENCE,35)@23 + 1
    u0_m0_wo0_oseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_oseq_c : SIGNED(8 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_c := "000111111";
            u0_m0_wo0_oseq_q <= "0";
            u0_m0_wo0_oseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_23_q = "1") THEN
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

    -- u0_m0_wo0_oseq_gated(LOGICAL,36)@24
    u0_m0_wo0_oseq_gated_q <= u0_m0_wo0_oseq_q and d_u0_m0_wo0_compute_q_24_q;

    -- u0_m0_wo0_oseq_gated_reg(REG,37)@24 + 1
    u0_m0_wo0_oseq_gated_reg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= "0";
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_gated_q);
        END IF;
    END PROCESS;

    -- xOut(PORTOUT,42)@25 + 1
    xOut_v <= u0_m0_wo0_oseq_gated_reg_q;
    xOut_c <= STD_LOGIC_VECTOR("0000000" & GND_q);
    xOut_0 <= u0_m0_wo0_accum_BitJoin_for_q_q;

END normal;
