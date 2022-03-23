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
-- VHDL created on Wed Mar 23 10:11:35 2022


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
        xIn_0 : in std_logic_vector(23 downto 0);  -- sfix24
        enable_i : in std_logic_vector(0 downto 0);  -- sfix1
        xOut_v : out std_logic_vector(0 downto 0);  -- ufix1
        xOut_c : out std_logic_vector(7 downto 0);  -- ufix8
        xOut_0 : out std_logic_vector(47 downto 0);  -- sfix48
        clk : in std_logic;
        areset : in std_logic
    );
end tx_ciccomp_rtl_core;

architecture normal of tx_ciccomp_rtl_core is

    attribute altera_attribute : string;
    attribute altera_attribute of normal : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF; -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 10037; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 15400; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 12020; -name MESSAGE_DISABLE 12030; -name MESSAGE_DISABLE 12010; -name MESSAGE_DISABLE 12110; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 113007";
    
    signal GND_q : STD_LOGIC_VECTOR (0 downto 0);
    signal VCC_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_xIn_0_15_q : STD_LOGIC_VECTOR (23 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_11_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_15_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_eq : std_logic;
    signal u0_m0_wo0_run_count : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_run_preEnaQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_out : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_enableQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_ctrl : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_memread_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_14_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_compute_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_i : SIGNED (9 downto 0);
    attribute preserve : boolean;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_i : UNSIGNED (8 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count1_q : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count1_i : UNSIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count1_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_i : UNSIGNED (8 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_a : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_b : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_o : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_q : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_i : UNSIGNED (8 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_memr0_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr0_ia : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_aa : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_ab : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_iq : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_ca0_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca0_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_ca0_inner_i : signal is true;
    signal u0_m0_wo0_ca0_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_ca0_inner_eq : signal is true;
    signal u0_m0_wo0_ca0_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_ca0_i : UNSIGNED (7 downto 0);
    attribute preserve of u0_m0_wo0_ca0_i : signal is true;
    signal u0_m0_wo0_aseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_aseq_eq : std_logic;
    signal d_u0_m0_wo0_aseq_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_aseq_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_eq : std_logic;
    signal u0_m0_wo0_oseq_gated_reg_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_out0_m0_wo0_assign_id3_q_22_q : STD_LOGIC_VECTOR (0 downto 0);
    signal outchan_q : STD_LOGIC_VECTOR (1 downto 0);
    signal outchan_i : UNSIGNED (0 downto 0);
    attribute preserve of outchan_i : signal is true;
    signal u0_m0_wo0_cm0_lutmem_reset0 : std_logic;
    signal u0_m0_wo0_cm0_lutmem_ia : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_cm0_lutmem_aa : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_cm0_lutmem_ab : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_cm0_lutmem_ir : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_cm0_lutmem_r : STD_LOGIC_VECTOR (15 downto 0);
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
    signal u0_m0_wo0_mtree_mult1_0_im6_a0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_b0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_s1 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im6_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im6_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_a0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_s1 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im9_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im9_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_a0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_b0 : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_s1 : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im12_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im12_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_a0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_b0 : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_s1 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im15_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im15_q : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_i : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p1_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_accum_p1_of_3_q_22_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_a : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_b : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_i : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_o : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p2_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_accum_p2_of_3_q_22_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_a : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_b : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_i : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_o : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_accum_p3_of_3_q : STD_LOGIC_VECTOR (7 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_18_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_18_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_a : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_b : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_o : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18_q : STD_LOGIC_VECTOR (12 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18_q : STD_LOGIC_VECTOR (12 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o : STD_LOGIC_VECTOR (14 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q : STD_LOGIC_VECTOR (12 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_a : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_b : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_o : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_c : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_a : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_b : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_o : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_cin : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q : STD_LOGIC_VECTOR (19 downto 0);
    signal d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q_21_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_adelay_p0_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_adelay_p1_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_adelay_p2_q : STD_LOGIC_VECTOR (7 downto 0);
    signal input_valid_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_gated_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_b : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs5_in : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs5_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs11_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs7_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_join_18_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_qint : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_q : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_qint : STD_LOGIC_VECTOR (23 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_23_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_23_qint : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_join_20_q : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_accum_BitJoin_for_q_q : STD_LOGIC_VECTOR (47 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q : STD_LOGIC_VECTOR (40 downto 0);
    signal u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c : STD_LOGIC_VECTOR (12 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q : STD_LOGIC_VECTOR (40 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_b : STD_LOGIC_VECTOR (19 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c : STD_LOGIC_VECTOR (12 downto 0);

begin


    -- GND(CONSTANT,0)@0
    GND_q <= "0";

    -- VCC(CONSTANT,1)@0
    VCC_q <= "1";

    -- input_valid(LOGICAL,3)@10
    input_valid_q <= xIn_v and enable_i;

    -- d_in0_m0_wi0_wo0_assign_id1_q_11(DELAY,201)@10 + 1
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
        variable u0_m0_wo0_run_enable_c : SIGNED(9 downto 0);
        variable u0_m0_wo0_run_inc : SIGNED(1 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_run_q <= "0";
            u0_m0_wo0_run_enable_c := TO_SIGNED(510, 10);
            u0_m0_wo0_run_enableQ <= "0";
            u0_m0_wo0_run_count <= "00";
            u0_m0_wo0_run_inc := (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_run_out = "1") THEN
                IF (u0_m0_wo0_run_enable_c(9) = '1') THEN
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c - (-511);
                ELSE
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c + (-1);
                END IF;
                u0_m0_wo0_run_enableQ <= STD_LOGIC_VECTOR(u0_m0_wo0_run_enable_c(9 downto 9));
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

    -- d_u0_m0_wo0_compute_q_18(DELAY,207)@13 + 5
    d_u0_m0_wo0_compute_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 5, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_compute_q, xout => d_u0_m0_wo0_compute_q_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_aseq(SEQUENCE,43)@18 + 1
    u0_m0_wo0_aseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_aseq_c : SIGNED(11 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_aseq_c := "000000000000";
            u0_m0_wo0_aseq_q <= "0";
            u0_m0_wo0_aseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_18_q = "1") THEN
                IF (u0_m0_wo0_aseq_c = "111111111111") THEN
                    u0_m0_wo0_aseq_eq <= '1';
                ELSE
                    u0_m0_wo0_aseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_aseq_eq = '1') THEN
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c + 511;
                ELSE
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c - 1;
                END IF;
                u0_m0_wo0_aseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_aseq_c(11 downto 11));
            END IF;
        END IF;
    END PROCESS;

    -- d_u0_m0_wo0_compute_q_19(DELAY,208)@18 + 1
    d_u0_m0_wo0_compute_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_18_q, xout => d_u0_m0_wo0_compute_q_19_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_14(DELAY,203)@13 + 1
    d_u0_m0_wo0_memread_q_14 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => d_u0_m0_wo0_memread_q_14_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_19(DELAY,204)@14 + 5
    d_u0_m0_wo0_memread_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 5, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_memread_q_14_q, xout => d_u0_m0_wo0_memread_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay_p0(DELAY,136)@19
    u0_m0_wo0_adelay_p0 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_3_q, xout => u0_m0_wo0_adelay_p0_q, ena => d_u0_m0_wo0_compute_q_19_q(0), clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b(BITSELECT,116)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_23_q(31 downto 31));

    -- u0_m0_wo0_wi0_r0_ra0_count2_inner(COUNTER,26)@14
    -- low=-1, high=0, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count2_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= TO_SIGNED(0, 1);
            u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_memread_q_14_q = "1") THEN
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

    -- u0_m0_wo0_wi0_r0_ra0_count2_run(LOGICAL,27)@14
    u0_m0_wo0_wi0_r0_ra0_count2_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count2_inner_q(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_count2(COUNTER,28)@14
    -- low=0, high=511, step=2, init=0
    u0_m0_wo0_wi0_r0_ra0_count2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_i <= TO_UNSIGNED(0, 9);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_memread_q_14_q = "1" and u0_m0_wo0_wi0_r0_ra0_count2_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count2_i <= u0_m0_wo0_wi0_r0_ra0_count2_i + 2;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count2_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count2_i, 10)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_inner(COUNTER,22)@14
    -- low=-1, high=510, step=-1, init=510
    u0_m0_wo0_wi0_r0_ra0_count0_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= TO_SIGNED(510, 10);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_memread_q_14_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count0_inner_i(9 downto 9) = "1") THEN
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 513;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_inner_i, 10)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_run(LOGICAL,23)@14
    u0_m0_wo0_wi0_r0_ra0_count0_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count0_inner_q(9 downto 9));

    -- u0_m0_wo0_wi0_r0_ra0_count0(COUNTER,24)@14
    -- low=0, high=511, step=2, init=0
    u0_m0_wo0_wi0_r0_ra0_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_i <= TO_UNSIGNED(0, 9);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_memread_q_14_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count0_i <= u0_m0_wo0_wi0_r0_ra0_count0_i + 2;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_i, 10)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a(BITSELECT,84)@14
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((10 downto 10 => u0_m0_wo0_wi0_r0_ra0_count0_q(9)) & u0_m0_wo0_wi0_r0_ra0_count0_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in(8 downto 1));

    -- u0_m0_wo0_wi0_r0_ra0_count1(COUNTER,25)@14
    -- low=0, high=1, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count1_i <= TO_UNSIGNED(0, 1);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_memread_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count1_i <= u0_m0_wo0_wi0_r0_ra0_count1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count1_i, 2)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b(BITSELECT,85)@14
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((10 downto 2 => u0_m0_wo0_wi0_r0_ra0_count1_q(1)) & u0_m0_wo0_wi0_r0_ra0_count1_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join(BITJOIN,86)@14
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q <= GND_q & GND_q & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b;

    -- u0_m0_wo0_wi0_r0_ra0_add_1_0(ADD,30)@14 + 1
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
    u0_m0_wo0_wi0_r0_ra0_add_1_0_q <= u0_m0_wo0_wi0_r0_ra0_add_1_0_o(11 downto 0);

    -- u0_m0_wo0_wi0_r0_ra0_resize(BITSELECT,31)@15
    u0_m0_wo0_wi0_r0_ra0_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_1_0_q(8 downto 0));
    u0_m0_wo0_wi0_r0_ra0_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_resize_in(8 downto 0));

    -- d_xIn_0_15(DELAY,199)@10 + 5
    d_xIn_0_15 : dspba_delay
    GENERIC MAP ( width => 24, depth => 5, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_0, xout => d_xIn_0_15_q, clk => clk, aclr => areset );

    -- d_in0_m0_wi0_wo0_assign_id1_q_15(DELAY,202)@11 + 4
    d_in0_m0_wi0_wo0_assign_id1_q_15 : dspba_delay
    GENERIC MAP ( width => 1, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_in0_m0_wi0_wo0_assign_id1_q_11_q, xout => d_in0_m0_wi0_wo0_assign_id1_q_15_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_wi0_r0_wa0(COUNTER,32)@15
    -- low=0, high=511, step=1, init=510
    u0_m0_wo0_wi0_r0_wa0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa0_i <= TO_UNSIGNED(510, 9);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_in0_m0_wi0_wo0_assign_id1_q_15_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa0_i <= u0_m0_wo0_wi0_r0_wa0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa0_i, 9)));

    -- u0_m0_wo0_wi0_r0_memr0(DUALMEM,33)@15
    u0_m0_wo0_wi0_r0_memr0_ia <= STD_LOGIC_VECTOR(d_xIn_0_15_q);
    u0_m0_wo0_wi0_r0_memr0_aa <= u0_m0_wo0_wi0_r0_wa0_q;
    u0_m0_wo0_wi0_r0_memr0_ab <= u0_m0_wo0_wi0_r0_ra0_resize_b;
    u0_m0_wo0_wi0_r0_memr0_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 24,
        widthad_a => 9,
        numwords_a => 512,
        width_b => 24,
        widthad_b => 9,
        numwords_b => 512,
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
        wren_a => d_in0_m0_wi0_wo0_assign_id1_q_15_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr0_ab,
        q_b => u0_m0_wo0_wi0_r0_memr0_iq
    );
    u0_m0_wo0_wi0_r0_memr0_q <= u0_m0_wo0_wi0_r0_memr0_iq(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bs5(BITSELECT,61)@15
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
    -- low=0, high=255, step=1, init=0
    u0_m0_wo0_ca0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca0_i <= TO_UNSIGNED(0, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_compute_q = "1" and u0_m0_wo0_ca0_run_q = "1") THEN
                u0_m0_wo0_ca0_i <= u0_m0_wo0_ca0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca0_i, 8)));

    -- u0_m0_wo0_cm0_lutmem(DUALMEM,55)@13 + 2
    u0_m0_wo0_cm0_lutmem_aa <= u0_m0_wo0_ca0_q;
    u0_m0_wo0_cm0_lutmem_reset0 <= areset;
    u0_m0_wo0_cm0_lutmem_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "ROM",
        width_a => 16,
        widthad_a => 8,
        numwords_a => 256,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        outdata_reg_a => "CLOCK0",
        outdata_aclr_a => "CLEAR0",
        clock_enable_input_a => "NORMAL",
        power_up_uninitialized => "FALSE",
        init_file => "tx_ciccomp_rtl_core_u0_m0_wo0_cm0_lutmem.hex",
        init_file_layout => "PORT_A",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        aclr0 => u0_m0_wo0_cm0_lutmem_reset0,
        clock0 => clk,
        address_a => u0_m0_wo0_cm0_lutmem_aa,
        q_a => u0_m0_wo0_cm0_lutmem_ir
    );
    u0_m0_wo0_cm0_lutmem_r <= u0_m0_wo0_cm0_lutmem_ir(15 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bs7(BITSELECT,63)@15
    u0_m0_wo0_mtree_mult1_0_bs7_b <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_lutmem_r(15 downto 8));

    -- u0_m0_wo0_mtree_mult1_0_im12(MULT,68)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im12_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im12_b0 <= '0' & u0_m0_wo0_mtree_mult1_0_bs5_b;
    u0_m0_wo0_mtree_mult1_0_im12_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im12_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 8,
        lpm_widthb => 9,
        lpm_widthp => 17,
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
    u0_m0_wo0_mtree_mult1_0_im12_q <= u0_m0_wo0_mtree_mult1_0_im12_s1(15 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_align_23(BITSHIFT,79)@17
    u0_m0_wo0_mtree_mult1_0_align_23_qint <= u0_m0_wo0_mtree_mult1_0_im12_q & "0000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_23_q <= u0_m0_wo0_mtree_mult1_0_align_23_qint(31 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b(BITJOIN,115)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_b_b & u0_m0_wo0_mtree_mult1_0_align_23_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b(BITSELECT,119)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_b_q(32 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a(BITSELECT,113)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_align_21_q(23 downto 23));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a(BITJOIN,114)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_SignBit_for_a_b;

    -- u0_m0_wo0_mtree_mult1_0_bs2(BITSELECT,58)@15
    u0_m0_wo0_mtree_mult1_0_bs2_in <= u0_m0_wo0_wi0_r0_memr0_q(7 downto 0);
    u0_m0_wo0_mtree_mult1_0_bs2_b <= u0_m0_wo0_mtree_mult1_0_bs2_in(7 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im6(MULT,62)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im6_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im6_b0 <= '0' & u0_m0_wo0_mtree_mult1_0_bs2_b;
    u0_m0_wo0_mtree_mult1_0_im6_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im6_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 8,
        lpm_widthb => 9,
        lpm_widthp => 17,
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
    u0_m0_wo0_mtree_mult1_0_im6_q <= u0_m0_wo0_mtree_mult1_0_im6_s1(15 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_align_21(BITSHIFT,77)@17
    u0_m0_wo0_mtree_mult1_0_align_21_qint <= u0_m0_wo0_mtree_mult1_0_im6_q & "00000000";
    u0_m0_wo0_mtree_mult1_0_align_21_q <= u0_m0_wo0_mtree_mult1_0_align_21_qint(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a(BITJOIN,112)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_UpperBits_for_a_q & u0_m0_wo0_mtree_mult1_0_align_21_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a(BITSELECT,118)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitExpansion_for_a_q(32 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2(ADD,120)@17 + 1
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

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b(BITSELECT,104)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_join_20_q(39 downto 39));

    -- u0_m0_wo0_mtree_mult1_0_bs11(BITSELECT,67)@15
    u0_m0_wo0_mtree_mult1_0_bs11_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr0_q(23 downto 16));

    -- u0_m0_wo0_mtree_mult1_0_im15(MULT,71)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im15_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs7_b);
    u0_m0_wo0_mtree_mult1_0_im15_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs11_b);
    u0_m0_wo0_mtree_mult1_0_im15_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im15_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 8,
        lpm_widthb => 8,
        lpm_widthp => 16,
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

    -- u0_m0_wo0_mtree_mult1_0_bs1(BITSELECT,57)@15
    u0_m0_wo0_mtree_mult1_0_bs1_in <= u0_m0_wo0_cm0_lutmem_r(7 downto 0);
    u0_m0_wo0_mtree_mult1_0_bs1_b <= u0_m0_wo0_mtree_mult1_0_bs1_in(7 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im3(MULT,59)@15 + 2
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

    -- u0_m0_wo0_mtree_mult1_0_align_19(BITSHIFT,75)@17
    u0_m0_wo0_mtree_mult1_0_align_19_qint <= u0_m0_wo0_mtree_mult1_0_im3_q & "00000000";
    u0_m0_wo0_mtree_mult1_0_align_19_q <= u0_m0_wo0_mtree_mult1_0_align_19_qint(23 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_join_20(BITJOIN,76)@17
    u0_m0_wo0_mtree_mult1_0_join_20_q <= u0_m0_wo0_mtree_mult1_0_im15_q & u0_m0_wo0_mtree_mult1_0_align_19_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b(BITJOIN,103)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_b_b & u0_m0_wo0_mtree_mult1_0_join_20_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b(BITSELECT,107)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_b_q(39 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a(BITSELECT,101)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_join_18_q(31 downto 31));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a(BITJOIN,102)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b & u0_m0_wo0_mtree_mult1_0_result_add_0_0_SignBit_for_a_b;

    -- u0_m0_wo0_mtree_mult1_0_im9(MULT,65)@15 + 2
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

    -- u0_m0_wo0_mtree_mult1_0_im0(MULT,56)@15 + 2
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

    -- u0_m0_wo0_mtree_mult1_0_join_18(BITJOIN,74)@17
    u0_m0_wo0_mtree_mult1_0_join_18_q <= u0_m0_wo0_mtree_mult1_0_im9_q & u0_m0_wo0_mtree_mult1_0_im0_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a(BITJOIN,100)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_UpperBits_for_a_q & u0_m0_wo0_mtree_mult1_0_join_18_q;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a(BITSELECT,106)@17
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q(19 downto 0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitExpansion_for_a_q(39 downto 20));

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3(ADD,108)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_b);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_o(20);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_o(19 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3(ADD,131)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_q);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_q);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_c(0) <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_o(20);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_o(19 downto 0);

    -- u0_m0_wo0_accum_p1_of_3(ADD,96)@19 + 1
    u0_m0_wo0_accum_p1_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_q);
    u0_m0_wo0_accum_p1_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_adelay_p0_q);
    u0_m0_wo0_accum_p1_of_3_i <= u0_m0_wo0_accum_p1_of_3_a;
    u0_m0_wo0_accum_p1_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p1_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_19_q = "1") THEN
                IF (u0_m0_wo0_aseq_q = "1") THEN
                    u0_m0_wo0_accum_p1_of_3_o <= u0_m0_wo0_accum_p1_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p1_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p1_of_3_a) + UNSIGNED(u0_m0_wo0_accum_p1_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p1_of_3_c(0) <= u0_m0_wo0_accum_p1_of_3_o(20);
    u0_m0_wo0_accum_p1_of_3_q <= u0_m0_wo0_accum_p1_of_3_o(19 downto 0);

    -- d_u0_m0_wo0_aseq_q_20(DELAY,211)@19 + 1
    d_u0_m0_wo0_aseq_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_aseq_q, xout => d_u0_m0_wo0_aseq_q_20_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_20(DELAY,209)@19 + 1
    d_u0_m0_wo0_compute_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_19_q, xout => d_u0_m0_wo0_compute_q_20_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_20(DELAY,205)@19 + 1
    d_u0_m0_wo0_memread_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_memread_q_19_q, xout => d_u0_m0_wo0_memread_q_20_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay_p1(DELAY,137)@20
    u0_m0_wo0_adelay_p1 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_p2_of_3_q, xout => u0_m0_wo0_adelay_p1_q, ena => d_u0_m0_wo0_compute_q_20_q(0), clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1(BITSELECT,182)@19
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q(12 downto 12));

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18(DELAY,219)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18 : dspba_delay
    GENERIC MAP ( width => 13, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18(DELAY,218)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18 : dspba_delay
    GENERIC MAP ( width => 13, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2(ADD,121)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p1_of_2_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((13 downto 13 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18_q(12)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_a_c_18_q) & '1');
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((13 downto 13 => d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18_q(12)) & d_u0_m0_wo0_mtree_mult1_0_result_add_0_1_BitSelect_for_b_c_18_q) & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_cin(0));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_o(13 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c(BITJOIN,189)@19
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_tessel1_1_b & u0_m0_wo0_mtree_mult1_0_result_add_0_1_p2_of_2_q;

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_18(DELAY,217)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_18 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_18(DELAY,216)@17 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_18 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_18_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3(ADD,109)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_cin <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p1_of_3_c;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_a <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_a_c_18_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_b <= STD_LOGIC_VECTOR("0" & d_u0_m0_wo0_mtree_mult1_0_result_add_0_0_BitSelect_for_b_c_18_q) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_o(20 downto 1);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3(ADD,132)@19 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_cin <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p1_of_3_c;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_0_0_p2_of_3_q) & '1';
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_BitSelect_for_b_BitJoin_for_c_q) & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_cin(0);
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_a) + UNSIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_o(20 downto 1);

    -- u0_m0_wo0_accum_p2_of_3(ADD,97)@20 + 1
    u0_m0_wo0_accum_p2_of_3_cin <= u0_m0_wo0_accum_p1_of_3_c;
    u0_m0_wo0_accum_p2_of_3_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q) & '1';
    u0_m0_wo0_accum_p2_of_3_b <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_adelay_p1_q) & u0_m0_wo0_accum_p2_of_3_cin(0);
    u0_m0_wo0_accum_p2_of_3_i <= u0_m0_wo0_accum_p2_of_3_a;
    u0_m0_wo0_accum_p2_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p2_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_20_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_20_q = "1") THEN
                    u0_m0_wo0_accum_p2_of_3_o <= u0_m0_wo0_accum_p2_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p2_of_3_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_accum_p2_of_3_a) + UNSIGNED(u0_m0_wo0_accum_p2_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p2_of_3_c(0) <= u0_m0_wo0_accum_p2_of_3_o(21);
    u0_m0_wo0_accum_p2_of_3_q <= u0_m0_wo0_accum_p2_of_3_o(20 downto 1);

    -- d_u0_m0_wo0_aseq_q_21(DELAY,212)@20 + 1
    d_u0_m0_wo0_aseq_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_aseq_q_20_q, xout => d_u0_m0_wo0_aseq_q_21_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_21(DELAY,210)@20 + 1
    d_u0_m0_wo0_compute_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_20_q, xout => d_u0_m0_wo0_compute_q_21_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_21(DELAY,206)@20 + 1
    d_u0_m0_wo0_memread_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_memread_q_20_q, xout => d_u0_m0_wo0_memread_q_21_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay_p2(DELAY,138)@21
    u0_m0_wo0_adelay_p2 : dspba_delay
    GENERIC MAP ( width => 8, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_p3_of_3_q, xout => u0_m0_wo0_adelay_p2_q, ena => d_u0_m0_wo0_compute_q_21_q(0), clk => clk, aclr => areset );

    -- d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q_21(DELAY,220)@20 + 1
    d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q_21 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q, xout => d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q_21_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitSelect_for_a_tessel2_0(BITSELECT,144)@21
    u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b <= STD_LOGIC_VECTOR(d_u0_m0_wo0_mtree_mult1_0_result_add_1_0_p2_of_3_q_21_q(19 downto 19));

    -- u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d(BITJOIN,152)@21
    u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q <= u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b & u0_m0_wo0_accum_BitSelect_for_a_tessel2_0_b;

    -- u0_m0_wo0_accum_p3_of_3(ADD,98)@21 + 1
    u0_m0_wo0_accum_p3_of_3_cin <= u0_m0_wo0_accum_p2_of_3_c;
    u0_m0_wo0_accum_p3_of_3_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 8 => u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q(7)) & u0_m0_wo0_accum_BitSelect_for_a_BitJoin_for_d_q) & '1');
    u0_m0_wo0_accum_p3_of_3_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 8 => u0_m0_wo0_adelay_p2_q(7)) & u0_m0_wo0_adelay_p2_q) & u0_m0_wo0_accum_p3_of_3_cin(0));
    u0_m0_wo0_accum_p3_of_3_i <= u0_m0_wo0_accum_p3_of_3_a;
    u0_m0_wo0_accum_p3_of_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_p3_of_3_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_21_q = "1") THEN
                IF (d_u0_m0_wo0_aseq_q_21_q = "1") THEN
                    u0_m0_wo0_accum_p3_of_3_o <= u0_m0_wo0_accum_p3_of_3_i;
                ELSE
                    u0_m0_wo0_accum_p3_of_3_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_accum_p3_of_3_a) + SIGNED(u0_m0_wo0_accum_p3_of_3_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_p3_of_3_q <= u0_m0_wo0_accum_p3_of_3_o(8 downto 1);

    -- d_u0_m0_wo0_accum_p2_of_3_q_22(DELAY,215)@21 + 1
    d_u0_m0_wo0_accum_p2_of_3_q_22 : dspba_delay
    GENERIC MAP ( width => 20, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p2_of_3_q, xout => d_u0_m0_wo0_accum_p2_of_3_q_22_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_accum_p1_of_3_q_22(DELAY,214)@20 + 2
    d_u0_m0_wo0_accum_p1_of_3_q_22 : dspba_delay
    GENERIC MAP ( width => 20, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_accum_p1_of_3_q, xout => d_u0_m0_wo0_accum_p1_of_3_q_22_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_accum_BitJoin_for_q(BITJOIN,99)@22
    u0_m0_wo0_accum_BitJoin_for_q_q <= u0_m0_wo0_accum_p3_of_3_q & d_u0_m0_wo0_accum_p2_of_3_q_22_q & d_u0_m0_wo0_accum_p1_of_3_q_22_q;

    -- u0_m0_wo0_oseq(SEQUENCE,45)@19 + 1
    u0_m0_wo0_oseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_oseq_c : SIGNED(11 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_c := "000111111110";
            u0_m0_wo0_oseq_q <= "0";
            u0_m0_wo0_oseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_19_q = "1") THEN
                IF (u0_m0_wo0_oseq_c = "111111111111") THEN
                    u0_m0_wo0_oseq_eq <= '1';
                ELSE
                    u0_m0_wo0_oseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_oseq_eq = '1') THEN
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c + 511;
                ELSE
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c - 1;
                END IF;
                u0_m0_wo0_oseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_c(11 downto 11));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_oseq_gated(LOGICAL,46)@20
    u0_m0_wo0_oseq_gated_q <= u0_m0_wo0_oseq_q and d_u0_m0_wo0_compute_q_20_q;

    -- u0_m0_wo0_oseq_gated_reg(REG,47)@20 + 1
    u0_m0_wo0_oseq_gated_reg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= "0";
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_gated_q);
        END IF;
    END PROCESS;

    -- outchan(COUNTER,52)@21 + 1
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

    -- d_out0_m0_wo0_assign_id3_q_22(DELAY,213)@21 + 1
    d_out0_m0_wo0_assign_id3_q_22 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_oseq_gated_reg_q, xout => d_out0_m0_wo0_assign_id3_q_22_q, clk => clk, aclr => areset );

    -- xOut(PORTOUT,53)@22 + 1
    xOut_v <= d_out0_m0_wo0_assign_id3_q_22_q;
    xOut_c <= STD_LOGIC_VECTOR("000000" & outchan_q);
    xOut_0 <= u0_m0_wo0_accum_BitJoin_for_q_q;

END normal;
