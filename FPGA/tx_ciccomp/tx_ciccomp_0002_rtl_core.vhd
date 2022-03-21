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
-- VHDL created on Mon Mar 21 16:21:23 2022


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
        enable_i : in std_logic_vector(0 downto 0);  -- sfix1
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
    signal d_xIn_0_14_q : STD_LOGIC_VECTOR (23 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_11_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_14_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_eq : std_logic;
    signal u0_m0_wo0_run_count : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_run_preEnaQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_out : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_enableQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_ctrl : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_memread_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_compute_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_17_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_i : SIGNED (7 downto 0);
    attribute preserve : boolean;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count0_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count1_q : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count1_i : UNSIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count1_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_a : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_b : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_o : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_memr0_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr0_ia : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_aa : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_ab : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_iq : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_ca0_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca0_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_ca0_inner_i : signal is true;
    signal u0_m0_wo0_ca0_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_ca0_inner_eq : signal is true;
    signal u0_m0_wo0_ca0_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_ca0_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_ca0_i : signal is true;
    signal u0_m0_wo0_cm0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_aseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_aseq_eq : std_logic;
    signal d_u0_m0_wo0_aseq_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_eq : std_logic;
    signal u0_m0_wo0_oseq_gated_reg_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_out0_m0_wo0_assign_id3_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal outchan_q : STD_LOGIC_VECTOR (1 downto 0);
    signal outchan_i : UNSIGNED (0 downto 0);
    attribute preserve of outchan_i : signal is true;
    signal u0_m0_wo0_mtree_mult1_0_im0_a0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_s1 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im0_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_a0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_s1 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im3_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_a0 : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_b0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_s1 : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im6_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_a0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_s1 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im9_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_a0 : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_b0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_s1 : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im12_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_a0 : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_s1 : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im15_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_i : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p1_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_accum_p1_of_2_q_20_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_a : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_b : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_i : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_o : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17_q : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_a : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_b : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_o : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_q : STD_LOGIC_VECTOR (14 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17_q : STD_LOGIC_VECTOR (6 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_a : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_b : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_o : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_adelay_p0_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_adelay_p1_q : STD_LOGIC_VECTOR (19 downto 0);
    signal input_valid_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_gated_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_in : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs5_in : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs5_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs11_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs7_b : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_join_18_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_qint : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_q : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_qint : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_23_q : STD_LOGIC_VECTOR (25 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_23_qint : STD_LOGIC_VECTOR (25 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_join_20_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_accum_BitJoin_for_q_q : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_c_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel1_0_b : STD_LOGIC_VECTOR (13 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q : STD_LOGIC_VECTOR (26 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q : STD_LOGIC_VECTOR (34 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q : STD_LOGIC_VECTOR (34 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q : STD_LOGIC_VECTOR (26 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c : STD_LOGIC_VECTOR (6 downto 0);

begin


    -- GND(CONSTANT,0)@0
    GND_q <= "0";

    -- VCC(CONSTANT,1)@0
    VCC_q <= "1";

    -- input_valid(LOGICAL,3)@10
    input_valid_q <= xIn_v and enable_i;

    -- d_in0_m0_wi0_wo0_assign_id1_q_11(DELAY,183)@10 + 1
    d_in0_m0_wi0_wo0_assign_id1_q_11 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => input_valid_q, xout => d_in0_m0_wi0_wo0_assign_id1_q_11_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_inputframe_seq(SEQUENCE,14)@10 + 1
    u0_m0_wo0_inputframe_seq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_inputframe_seq_c : SIGNED(3 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_inputframe_seq_c := "0001";
            u0_m0_wo0_inputframe_seq_q <= "0";
            u0_m0_wo0_inputframe_seq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (input_valid_q = "1") THEN
                IF (u0_m0_wo0_inputframe_seq_c = "0000") THEN
                    u0_m0_wo0_inputframe_seq_eq <= '1';
                ELSE
                    u0_m0_wo0_inputframe_seq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_inputframe_seq_eq = '1') THEN
                    u0_m0_wo0_inputframe_seq_c := u0_m0_wo0_inputframe_seq_c + 1;
                ELSE
                    u0_m0_wo0_inputframe_seq_c := u0_m0_wo0_inputframe_seq_c - 1;
                END IF;
                u0_m0_wo0_inputframe_seq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_inputframe_seq_c(3 downto 3));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_inputframe(LOGICAL,15)@11
    u0_m0_wo0_inputframe_q <= u0_m0_wo0_inputframe_seq_q and d_in0_m0_wi0_wo0_assign_id1_q_11_q;

    -- u0_m0_wo0_run(ENABLEGENERATOR,16)@11 + 2
    u0_m0_wo0_run_ctrl <= u0_m0_wo0_run_out & u0_m0_wo0_inputframe_q & u0_m0_wo0_run_enableQ;
    u0_m0_wo0_run_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_run_enable_c : SIGNED(7 downto 0);
        variable u0_m0_wo0_run_inc : SIGNED(1 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_run_q <= "0";
            u0_m0_wo0_run_enable_c := TO_SIGNED(126, 8);
            u0_m0_wo0_run_enableQ <= "0";
            u0_m0_wo0_run_count <= "00";
            u0_m0_wo0_run_inc := (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_run_out = "1") THEN
                IF (u0_m0_wo0_run_enable_c(7) = '1') THEN
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c - (-127);
                ELSE
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c + (-1);
                END IF;
                u0_m0_wo0_run_enableQ <= STD_LOGIC_VECTOR(u0_m0_wo0_run_enable_c(7 downto 7));
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

    -- u0_m0_wo0_memread(DELAY,17)@13
    u0_m0_wo0_memread : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_run_q, xout => u0_m0_wo0_memread_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_compute(DELAY,19)@13
    u0_m0_wo0_compute : dspba_delay
    GENERIC MAP ( width => 1, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => u0_m0_wo0_compute_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_17(DELAY,187)@13 + 4
    d_u0_m0_wo0_compute_q_17 : dspba_delay
    GENERIC MAP ( width => 1, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_compute_q, xout => d_u0_m0_wo0_compute_q_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_aseq(SEQUENCE,43)@17 + 1
    u0_m0_wo0_aseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_aseq_c : SIGNED(9 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_aseq_c := "0000000000";
            u0_m0_wo0_aseq_q <= "0";
            u0_m0_wo0_aseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_17_q = "1") THEN
                IF (u0_m0_wo0_aseq_c = "1111111111") THEN
                    u0_m0_wo0_aseq_eq <= '1';
                ELSE
                    u0_m0_wo0_aseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_aseq_eq = '1') THEN
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c + 127;
                ELSE
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c - 1;
                END IF;
                u0_m0_wo0_aseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_aseq_c(9 downto 9));
            END IF;
        END IF;
    END PROCESS;

    -- d_u0_m0_wo0_compute_q_18(DELAY,188)@17 + 1
    d_u0_m0_wo0_compute_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_17_q, xout => d_u0_m0_wo0_compute_q_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_18(DELAY,185)@13 + 5
    d_u0_m0_wo0_memread_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 5, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => d_u0_m0_wo0_memread_q_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay_p0(DELAY,132)@18
    u0_m0_wo0_adelay_p0 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_2_q, xout => u0_m0_wo0_adelay_p0_q, ena => d_u0_m0_wo0_compute_q_18_q(0), clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b(BITSELECT,113)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_23_q(25 downto 25));

    -- u0_m0_wo0_wi0_r0_ra0_count2_inner(COUNTER,26)@13
    -- low=-1, high=0, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count2_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= TO_SIGNED(0, 1);
            u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count2_inner_eq = '0') THEN
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '1';
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '0';
                END IF;
                IF (u0_m0_wo0_wi0_r0_ra0_count2_inner_eq = '1') THEN
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= u0_m0_wo0_wi0_r0_ra0_count2_inner_i + 1;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= u0_m0_wo0_wi0_r0_ra0_count2_inner_i + 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count2_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count2_inner_i, 1)));

    -- u0_m0_wo0_wi0_r0_ra0_count2_run(LOGICAL,27)@13
    u0_m0_wo0_wi0_r0_ra0_count2_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count2_inner_q(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_count2(COUNTER,28)@13
    -- low=0, high=127, step=2, init=0
    u0_m0_wo0_wi0_r0_ra0_count2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_i <= TO_UNSIGNED(0, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count2_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count2_i <= u0_m0_wo0_wi0_r0_ra0_count2_i + 2;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count2_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count2_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_inner(COUNTER,22)@13
    -- low=-1, high=126, step=-1, init=126
    u0_m0_wo0_wi0_r0_ra0_count0_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= TO_SIGNED(126, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count0_inner_i(7 downto 7) = "1") THEN
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 129;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_inner_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_run(LOGICAL,23)@13
    u0_m0_wo0_wi0_r0_ra0_count0_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count0_inner_q(7 downto 7));

    -- u0_m0_wo0_wi0_r0_ra0_count0(COUNTER,24)@13
    -- low=0, high=127, step=2, init=0
    u0_m0_wo0_wi0_r0_ra0_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_i <= TO_UNSIGNED(0, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count0_i <= u0_m0_wo0_wi0_r0_ra0_count0_i + 2;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a(BITSELECT,83)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 8 => u0_m0_wo0_wi0_r0_ra0_count0_q(7)) & u0_m0_wo0_wi0_r0_ra0_count0_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in(6 downto 1));

    -- u0_m0_wo0_wi0_r0_ra0_count1(COUNTER,25)@13
    -- low=0, high=1, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count1_i <= TO_UNSIGNED(0, 1);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count1_i <= u0_m0_wo0_wi0_r0_ra0_count1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count1_i, 2)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b(BITSELECT,84)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 2 => u0_m0_wo0_wi0_r0_ra0_count1_q(1)) & u0_m0_wo0_wi0_r0_ra0_count1_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join(BITJOIN,85)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q <= GND_q & GND_q & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b;

    -- u0_m0_wo0_wi0_r0_ra0_add_1_0(ADD,30)@13 + 1
    u0_m0_wo0_wi0_r0_ra0_add_1_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q);
    u0_m0_wo0_wi0_r0_ra0_add_1_0_b <= STD_LOGIC_VECTOR("00" & u0_m0_wo0_wi0_r0_ra0_count2_q);
    u0_m0_wo0_wi0_r0_ra0_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_1_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_1_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_add_1_0_q <= u0_m0_wo0_wi0_r0_ra0_add_1_0_o(9 downto 0);

    -- u0_m0_wo0_wi0_r0_ra0_resize(BITSELECT,31)@14
    u0_m0_wo0_wi0_r0_ra0_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_1_0_q(6 downto 0));
    u0_m0_wo0_wi0_r0_ra0_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_resize_in(6 downto 0));

    -- d_xIn_0_14(DELAY,181)@10 + 4
    d_xIn_0_14 : dspba_delay
    GENERIC MAP ( width => 24, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_0, xout => d_xIn_0_14_q, clk => clk, aclr => areset );

    -- d_in0_m0_wi0_wo0_assign_id1_q_14(DELAY,184)@11 + 3
    d_in0_m0_wi0_wo0_assign_id1_q_14 : dspba_delay
    GENERIC MAP ( width => 1, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_in0_m0_wi0_wo0_assign_id1_q_11_q, xout => d_in0_m0_wi0_wo0_assign_id1_q_14_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_wi0_r0_wa0(COUNTER,32)@14
    -- low=0, high=127, step=1, init=126
    u0_m0_wo0_wi0_r0_wa0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa0_i <= TO_UNSIGNED(126, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_in0_m0_wi0_wo0_assign_id1_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa0_i <= u0_m0_wo0_wi0_r0_wa0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa0_i, 7)));

    -- u0_m0_wo0_wi0_r0_memr0(DUALMEM,33)@14
    u0_m0_wo0_wi0_r0_memr0_ia <= STD_LOGIC_VECTOR(d_xIn_0_14_q);
    u0_m0_wo0_wi0_r0_memr0_aa <= u0_m0_wo0_wi0_r0_wa0_q;
    u0_m0_wo0_wi0_r0_memr0_ab <= u0_m0_wo0_wi0_r0_ra0_resize_b;
    u0_m0_wo0_wi0_r0_memr0_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 24,
        widthad_a => 7,
        numwords_a => 128,
        width_b => 24,
        widthad_b => 7,
        numwords_b => 128,
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
        wren_a => d_in0_m0_wi0_wo0_assign_id1_q_14_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr0_ab,
        q_b => u0_m0_wo0_wi0_r0_memr0_iq
    );
    u0_m0_wo0_wi0_r0_memr0_q <= u0_m0_wo0_wi0_r0_memr0_iq(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bs5(BITSELECT,60)@14
    u0_m0_wo0_mtree_mult1_0_bs5_in <= u0_m0_wo0_wi0_r0_memr0_q(15 downto 0);
    u0_m0_wo0_mtree_mult1_0_bs5_b <= u0_m0_wo0_mtree_mult1_0_bs5_in(15 downto 8);

    -- u0_m0_wo0_ca0_inner(COUNTER,34)@13
    -- low=-1, high=0, step=1, init=0
    u0_m0_wo0_ca0_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca0_inner_i <= TO_SIGNED(0, 1);
            u0_m0_wo0_ca0_inner_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_compute_q = "1") THEN
                IF (u0_m0_wo0_ca0_inner_eq = '0') THEN
                    u0_m0_wo0_ca0_inner_eq <= '1';
                ELSE
                    u0_m0_wo0_ca0_inner_eq <= '0';
                END IF;
                IF (u0_m0_wo0_ca0_inner_eq = '1') THEN
                    u0_m0_wo0_ca0_inner_i <= u0_m0_wo0_ca0_inner_i + 1;
                ELSE
                    u0_m0_wo0_ca0_inner_i <= u0_m0_wo0_ca0_inner_i + 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca0_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca0_inner_i, 1)));

    -- u0_m0_wo0_ca0_run(LOGICAL,35)@13
    u0_m0_wo0_ca0_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_ca0_inner_q(0 downto 0));

    -- u0_m0_wo0_ca0(COUNTER,36)@13
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_ca0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca0_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_compute_q = "1" and u0_m0_wo0_ca0_run_q = "1") THEN
                u0_m0_wo0_ca0_i <= u0_m0_wo0_ca0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca0_i, 6)));

    -- u0_m0_wo0_cm0(LOOKUP,40)@13 + 1
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

    -- u0_m0_wo0_mtree_mult1_0_bs7(BITSELECT,62)@14
    u0_m0_wo0_mtree_mult1_0_bs7_b <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_q(9 downto 8));

    -- u0_m0_wo0_mtree_mult1_0_im12(MULT,67)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im12_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im12_b0 <= '0' & u0_m0_wo0_mtree_mult1_0_bs5_b;
    u0_m0_wo0_mtree_mult1_0_im12_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im12_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 2,
        lpm_widthb => 9,
        lpm_widthp => 11,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im12_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im12_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im12_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im12_s1
    );
    u0_m0_wo0_mtree_mult1_0_im12_q <= u0_m0_wo0_mtree_mult1_0_im12_s1(9 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_align_23(BITSHIFT,78)@16
    u0_m0_wo0_mtree_mult1_0_align_23_qint <= u0_m0_wo0_mtree_mult1_0_im12_q & "0000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_23_q <= u0_m0_wo0_mtree_mult1_0_align_23_qint(25 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b(BITJOIN,112)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b & u0_m0_wo0_mtree_mult1_0_align_23_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b(BITSELECT,116)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q(26 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a(BITSELECT,110)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_21_q(17 downto 17));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a(BITJOIN,111)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b;

    -- u0_m0_wo0_mtree_mult1_0_bs2(BITSELECT,57)@14
    u0_m0_wo0_mtree_mult1_0_bs2_in <= u0_m0_wo0_wi0_r0_memr0_q(7 downto 0);
    u0_m0_wo0_mtree_mult1_0_bs2_b <= u0_m0_wo0_mtree_mult1_0_bs2_in(7 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im6(MULT,61)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im6_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im6_b0 <= '0' & u0_m0_wo0_mtree_mult1_0_bs2_b;
    u0_m0_wo0_mtree_mult1_0_im6_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im6_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 2,
        lpm_widthb => 9,
        lpm_widthp => 11,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im6_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im6_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im6_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im6_s1
    );
    u0_m0_wo0_mtree_mult1_0_im6_q <= u0_m0_wo0_mtree_mult1_0_im6_s1(9 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_align_21(BITSHIFT,76)@16
    u0_m0_wo0_mtree_mult1_0_align_21_qint <= u0_m0_wo0_mtree_mult1_0_im6_q & "00000000";
    u0_m0_wo0_mtree_mult1_0_align_21_q <= u0_m0_wo0_mtree_mult1_0_align_21_qint(17 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a(BITJOIN,109)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q & u0_m0_wo0_mtree_mult1_0_align_21_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a(BITSELECT,115)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q(26 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2(ADD,117)@16 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o(20);
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o(19 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b(BITSELECT,102)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_join_20_q(33 downto 33));

    -- u0_m0_wo0_mtree_mult1_0_bs11(BITSELECT,66)@14
    u0_m0_wo0_mtree_mult1_0_bs11_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr0_q(23 downto 16));

    -- u0_m0_wo0_mtree_mult1_0_im15(MULT,70)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im15_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im15_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs11_b);
    u0_m0_wo0_mtree_mult1_0_im15_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im15_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 2,
        lpm_widthb => 8,
        lpm_widthp => 10,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im15_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im15_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im15_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im15_s1
    );
    u0_m0_wo0_mtree_mult1_0_im15_q <= u0_m0_wo0_mtree_mult1_0_im15_s1;

    -- u0_m0_wo0_mtree_mult1_0_bs1(BITSELECT,56)@14
    u0_m0_wo0_mtree_mult1_0_bs1_in <= u0_m0_wo0_cm0_q(7 downto 0);
    u0_m0_wo0_mtree_mult1_0_bs1_b <= u0_m0_wo0_mtree_mult1_0_bs1_in(7 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im3(MULT,58)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im3_a0 <= u0_m0_wo0_mtree_mult1_0_bs1_b;
    u0_m0_wo0_mtree_mult1_0_im3_b0 <= u0_m0_wo0_mtree_mult1_0_bs5_b;
    u0_m0_wo0_mtree_mult1_0_im3_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im3_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 8,
        lpm_widthb => 8,
        lpm_widthp => 16,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "UNSIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im3_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im3_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im3_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im3_s1
    );
    u0_m0_wo0_mtree_mult1_0_im3_q <= u0_m0_wo0_mtree_mult1_0_im3_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_19(BITSHIFT,74)@16
    u0_m0_wo0_mtree_mult1_0_align_19_qint <= u0_m0_wo0_mtree_mult1_0_im3_q & "00000000";
    u0_m0_wo0_mtree_mult1_0_align_19_q <= u0_m0_wo0_mtree_mult1_0_align_19_qint(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_join_20(BITJOIN,75)@16
    u0_m0_wo0_mtree_mult1_0_join_20_q <= u0_m0_wo0_mtree_mult1_0_im15_q & u0_m0_wo0_mtree_mult1_0_align_19_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b(BITJOIN,101)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b & u0_m0_wo0_mtree_mult1_0_join_20_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b(BITSELECT,105)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q(34 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a(BITSELECT,99)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_join_18_q(31 downto 31));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a(BITJOIN,100)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b;

    -- u0_m0_wo0_mtree_mult1_0_im9(MULT,64)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im9_a0 <= '0' & u0_m0_wo0_mtree_mult1_0_bs1_b;
    u0_m0_wo0_mtree_mult1_0_im9_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs11_b);
    u0_m0_wo0_mtree_mult1_0_im9_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im9_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 9,
        lpm_widthb => 8,
        lpm_widthp => 17,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im9_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im9_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im9_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im9_s1
    );
    u0_m0_wo0_mtree_mult1_0_im9_q <= u0_m0_wo0_mtree_mult1_0_im9_s1(15 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im0(MULT,55)@14 + 2
    u0_m0_wo0_mtree_mult1_0_im0_a0 <= u0_m0_wo0_mtree_mult1_0_bs1_b;
    u0_m0_wo0_mtree_mult1_0_im0_b0 <= u0_m0_wo0_mtree_mult1_0_bs2_b;
    u0_m0_wo0_mtree_mult1_0_im0_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im0_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 8,
        lpm_widthb => 8,
        lpm_widthp => 16,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "UNSIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=NO, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im0_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im0_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im0_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im0_s1
    );
    u0_m0_wo0_mtree_mult1_0_im0_q <= u0_m0_wo0_mtree_mult1_0_im0_s1;

    -- u0_m0_wo0_mtree_mult1_0_join_18(BITJOIN,73)@16
    u0_m0_wo0_mtree_mult1_0_join_18_q <= u0_m0_wo0_mtree_mult1_0_im9_q & u0_m0_wo0_mtree_mult1_0_im0_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a(BITJOIN,98)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q & u0_m0_wo0_mtree_mult1_0_join_18_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a(BITSELECT,104)@16
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q(34 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2(ADD,106)@16 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_o(20);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_o(19 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2(ADD,128)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_q);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_q);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_o(20);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_o(19 downto 0);

    -- u0_m0_wo0_accum_p1_of_2(ADD,95)@18 + 1
    u0_m0_wo0_accum_p1_of_2_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_q);
    u0_m0_wo0_accum_p1_of_2_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_adelay_p0_q);
    u0_m0_wo0_accum_p1_of_2_i <= u0_m0_wo0_accum_p1_of_2_a;
    u0_m0_wo0_accum_p1_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p1_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_18_q = "1") THEN
                IF (u0_m0_wo0_aseq_q = "1") THEN
                    u0_m0_wo0_accum_p1_of_2_o <= u0_m0_wo0_accum_p1_of_2_i;
                ELSE
                    u0_m0_wo0_accum_p1_of_2_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p1_of_2_a) + UNSIGNED(u0_m0_wo0_accum_p1_of_2_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p1_of_2_c(0) <= u0_m0_wo0_accum_p1_of_2_o(20);
    u0_m0_wo0_accum_p1_of_2_q <= u0_m0_wo0_accum_p1_of_2_o(19 downto 0);

    -- d_u0_m0_wo0_aseq_q_19(DELAY,190)@18 + 1
    d_u0_m0_wo0_aseq_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_aseq_q, xout => d_u0_m0_wo0_aseq_q_19_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_19(DELAY,189)@18 + 1
    d_u0_m0_wo0_compute_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_18_q, xout => d_u0_m0_wo0_compute_q_19_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_19(DELAY,186)@18 + 1
    d_u0_m0_wo0_memread_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_memread_q_18_q, xout => d_u0_m0_wo0_memread_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay_p1(DELAY,133)@19
    u0_m0_wo0_adelay_p1 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_p2_of_2_q, xout => u0_m0_wo0_adelay_p1_q, ena => d_u0_m0_wo0_compute_q_19_q(0), clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1(BITSELECT,167)@18
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q(6 downto 6));

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17(DELAY,196)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17 : dspba_delay
    GENERIC MAP ( width => 7, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17(DELAY,195)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17 : dspba_delay
    GENERIC MAP ( width => 7, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2(ADD,118)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((7 downto 7 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17_q(6)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_17_q) & '1');
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((7 downto 7 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17_q(6)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_17_q) & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin(0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o(7 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c(BITJOIN,176)@18
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel1_1(BITSELECT,162)@18
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel1_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_q(14 downto 14));

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17(DELAY,194)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17(DELAY,193)@16 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17 : dspba_delay
    GENERIC MAP ( width => 15, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2(ADD,107)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_2_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((15 downto 15 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17_q(14)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_17_q) & '1');
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((15 downto 15 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17_q(14)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_17_q) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_cin(0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_o(15 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_c(BITJOIN,163)@18
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_c_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_2_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2(ADD,129)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_cin <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_2_c;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((16 downto 16 => u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_c_q(15)) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_a_BitJoin_for_c_q) & '1');
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((16 downto 16 => u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q(15)) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_cin(0));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_o(16 downto 1);

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel1_1(BITSELECT,138)@19
    u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_q(13 downto 13));

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel1_0(BITSELECT,137)@19
    u0_m0_wo0_accum_BitSelect_for_a_tessel1_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_2_q(13 downto 0));

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c(BITJOIN,144)@19
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_1_b & u0_m0_wo0_accum_BitSelect_for_a_tessel1_0_b;

    -- u0_m0_wo0_accum_p2_of_2(ADD,96)@19 + 1
    u0_m0_wo0_accum_p2_of_2_cin <= u0_m0_wo0_accum_p1_of_2_c;
    u0_m0_wo0_accum_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((20 downto 20 => u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q(19)) & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_c_q) & '1');
    u0_m0_wo0_accum_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((20 downto 20 => u0_m0_wo0_adelay_p1_q(19)) & u0_m0_wo0_adelay_p1_q) & u0_m0_wo0_accum_p2_of_2_cin(0));
    u0_m0_wo0_accum_p2_of_2_i <= u0_m0_wo0_accum_p2_of_2_a;
    u0_m0_wo0_accum_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_19_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_19_q = "1") THEN
                    u0_m0_wo0_accum_p2_of_2_o <= u0_m0_wo0_accum_p2_of_2_i;
                ELSE
                    u0_m0_wo0_accum_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_accum_p2_of_2_a) + SIGNED(u0_m0_wo0_accum_p2_of_2_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p2_of_2_q <= u0_m0_wo0_accum_p2_of_2_o(20 downto 1);

    -- d_u0_m0_wo0_accum_p1_of_2_q_20(DELAY,192)@19 + 1
    d_u0_m0_wo0_accum_p1_of_2_q_20 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_2_q, xout => d_u0_m0_wo0_accum_p1_of_2_q_20_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitJoin_for_q(BITJOIN,97)@20
    u0_m0_wo0_accum_BitJoin_for_q_q <= u0_m0_wo0_accum_p2_of_2_q & d_u0_m0_wo0_accum_p1_of_2_q_20_q;

    -- u0_m0_wo0_oseq(SEQUENCE,45)@17 + 1
    u0_m0_wo0_oseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_oseq_c : SIGNED(9 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_c := "0001111110";
            u0_m0_wo0_oseq_q <= "0";
            u0_m0_wo0_oseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_17_q = "1") THEN
                IF (u0_m0_wo0_oseq_c = "1111111111") THEN
                    u0_m0_wo0_oseq_eq <= '1';
                ELSE
                    u0_m0_wo0_oseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_oseq_eq = '1') THEN
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c + 127;
                ELSE
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c - 1;
                END IF;
                u0_m0_wo0_oseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_c(9 downto 9));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_oseq_gated(LOGICAL,46)@18
    u0_m0_wo0_oseq_gated_q <= u0_m0_wo0_oseq_q and d_u0_m0_wo0_compute_q_18_q;

    -- u0_m0_wo0_oseq_gated_reg(REG,47)@18 + 1
    u0_m0_wo0_oseq_gated_reg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= "0";
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_gated_q);
        END IF;
    END PROCESS;

    -- outchan(COUNTER,52)@19 + 1
    -- low=0, high=1, step=1, init=1
    outchan_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            outchan_i <= TO_UNSIGNED(1, 1);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_oseq_gated_reg_q = "1") THEN
                outchan_i <= outchan_i + 1;
            END IF;
        END IF;
    END PROCESS;
    outchan_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(outchan_i, 2)));

    -- d_out0_m0_wo0_assign_id3_q_20(DELAY,191)@19 + 1
    d_out0_m0_wo0_assign_id3_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_oseq_gated_reg_q, xout => d_out0_m0_wo0_assign_id3_q_20_q, clk => clk, aclr => areset );

    -- xOut(PORTOUT,53)@20 + 1
    xOut_v <= d_out0_m0_wo0_assign_id3_q_20_q;
    xOut_c <= STD_LOGIC_VECTOR("000000" & outchan_q);
    xOut_0 <= u0_m0_wo0_accum_BitJoin_for_q_q;

END normal;
