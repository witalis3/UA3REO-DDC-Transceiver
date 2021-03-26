#include "audio_filters.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "functions.h"
#include "wm8731.h"
#include "settings.h"
#include "noise_reduction.h"
#include "noise_blanker.h"
#include "auto_notch.h"
#include "trx_manager.h"
#include "vad.h"
#include "BiquadDesigner/biquad.h"

//Private variables
static float32_t Fir_RX1_Hilbert_State_I[FIR_RX1_HILBERT_STATE_SIZE] = {0}; // buffers of filter states
static float32_t Fir_RX1_Hilbert_State_Q[FIR_RX1_HILBERT_STATE_SIZE] = {0};
static float32_t Fir_RX2_Hilbert_State_I[FIR_RX2_HILBERT_STATE_SIZE] = {0};
static float32_t Fir_RX2_Hilbert_State_Q[FIR_RX2_HILBERT_STATE_SIZE] = {0};
static float32_t Fir_Tx_Hilbert_State_I[FIR_TX_HILBERT_STATE_SIZE] = {0};
static float32_t Fir_Tx_Hilbert_State_Q[FIR_TX_HILBERT_STATE_SIZE] = {0};
static float32_t IIR_RX1_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX1_LPF_I_State[IIR_RX1_LPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX1_LPF_Q_State[IIR_RX1_LPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX1_GAUSS_State[IIR_RX1_GAUSS_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX2_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX2_LPF_I_State[IIR_RX2_LPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX2_LPF_Q_State[IIR_RX2_LPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX2_GAUSS_State[IIR_RX2_GAUSS_Taps_STATE_SIZE] = {0};
static float32_t IIR_TX_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_TX_LPF_I_State[IIR_RX2_LPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX1_HPF_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX2_HPF_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX1_HPF_I_State[IIR_RX1_HPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX2_HPF_I_State[IIR_RX2_HPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_TX_HPF_I_State[IIR_RX2_HPF_Taps_STATE_SIZE] = {0};
static float32_t IIR_RX1_HPF_SQL_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX2_HPF_SQL_Coeffs[BIQUAD_COEFF_IN_STAGE * IIR_STAGES] = {0};
static float32_t IIR_RX1_HPF_SQL_State[IIR_RX1_HPF_SQL_STATE_SIZE] = {0};
static float32_t IIR_RX2_HPF_SQL_State[IIR_RX2_HPF_SQL_STATE_SIZE] = {0};
static float32_t NOTCH_RX1_Coeffs[BIQUAD_COEFF_IN_STAGE * NOTCH_STAGES] = {0};
static float32_t NOTCH_RX2_Coeffs[BIQUAD_COEFF_IN_STAGE * NOTCH_STAGES] = {0};
static float32_t NOTCH_RX1_State[2 * NOTCH_STAGES] = {0};
static float32_t NOTCH_RX2_State[2 * NOTCH_STAGES] = {0};
static float32_t NOTCH_FFT_I_State[2 * NOTCH_STAGES] = {0};
static float32_t NOTCH_FFT_Q_State[2 * NOTCH_STAGES] = {0};
static float32_t EQ_RX_LOW_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_RX_MID_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_RX_HIG_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_RX_LOW_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t EQ_RX_MID_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t EQ_RX_HIG_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t EQ_MIC_LOW_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_MIC_MID_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_MIC_HIG_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t EQ_MIC_LOW_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t EQ_MIC_MID_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t EQ_MIC_HIG_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t AGC_RX1_KW_HSHELF_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t AGC_RX2_KW_HSHELF_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t AGC_RX1_KW_HPASS_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t AGC_RX2_KW_HPASS_FILTER_State[2 * EQ_STAGES] = {0};
static float32_t AGC_RX_KW_HSHELF_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};
static float32_t AGC_RX_KW_HPASS_FILTER_Coeffs[BIQUAD_COEFF_IN_STAGE * EQ_STAGES] = {0};

static DC_filter_state_type DC_Filter_State[8] = {0}; // states of the DC corrector

// Collection of filters
//IQ Hilbert with (-0 q / +90 i) degrees phase added,  48000 sampling frequency Kaiser, Kaiser Beta 4.800 (+90), Raised Cosine 0.930 (-0) // Iowa Hills Hilbert Filters
static const float32_t FIR_HILB_I_coeffs[IQ_HILBERT_TAPS] = {0.000000000000000000, 0.000348718040684899, 0.000000000000000000, 0.000430833440231578, 0.000000000000000000, 0.000522908939129710, 0.000000000000000000, 0.000625643170080525, 0.000000000000000000, 0.000739763696285730, 0.000000000000000000, 0.000866029072658519, 0.000000000000000000, 0.001005231378692548, 0.000000000000000000, 0.001158199311056187, 0.000000000000000000, 0.001325801939712967, 0.000000000000000000, 0.001508953250504772, 0.000000000000000000, 0.001708617620559288, 0.000000000000000000, 0.001925816401721571, 0.000000000000000000, 0.002161635822912572, 0.000000000000000000, 0.002417236466741860, 0.000000000000000000, 0.002693864631427011, 0.000000000000000000, 0.002992865959230020, 0.000000000000000000, 0.003315701801645692, 0.000000000000000000, 0.003663968905292113, 0.000000000000000000, 0.004039423148679463, 0.000000000000000000, 0.004444008249195281, 0.000000000000000000, 0.004879890607273629, 0.000000000000000000, 0.005349501779412148, 0.000000000000000000, 0.005855590504225607, 0.000000000000000000, 0.006401286783911277, 0.000000000000000000, 0.006990181306750123, 0.000000000000000000, 0.007626424567085178, 0.000000000000000000, 0.008314851520255883, 0.000000000000000000, 0.009061139683747135, 0.000000000000000000, 0.009872011537848968, 0.000000000000000000, 0.010755496313062237, 0.000000000000000000, 0.011721272439718031, 0.000000000000000000, 0.012781121134483500, 0.000000000000000000, 0.013949535528809384, 0.000000000000000000, 0.015244551273554119, 0.000000000000000000, 0.016688898584430931, 0.000000000000000000, 0.018311630846033072, 0.000000000000000000, 0.020150476814457717, 0.000000000000000000, 0.022255321565352600, 0.000000000000000000, 0.024693503177301100, 0.000000000000000000, 0.027558135563807963, 0.000000000000000000, 0.030981687165876175, 0.000000000000000000, 0.035159143774044777, 0.000000000000000000, 0.040389700574458606, 0.000000000000000000, 0.047156937867897229, 0.000000000000000000, 0.056296459573201306, 0.000000000000000000, 0.069387047614511754, 0.000000000000000000, 0.089812363821891061, 0.000000000000000000, 0.126370732214445475, 0.000000000000000000, 0.211324043214114715, 0.000000000000000000, 0.635033486245172751, 0.000000000000000000, -0.635033486245172751, 0.000000000000000000, -0.211324043214114715, 0.000000000000000000, -0.126370732214445475, 0.000000000000000000, -0.089812363821891061, 0.000000000000000000, -0.069387047614511754, 0.000000000000000000, -0.056296459573201306, 0.000000000000000000, -0.047156937867897229, 0.000000000000000000, -0.040389700574458606, 0.000000000000000000, -0.035159143774044777, 0.000000000000000000, -0.030981687165876175, 0.000000000000000000, -0.027558135563807963, 0.000000000000000000, -0.024693503177301100, 0.000000000000000000, -0.022255321565352600, 0.000000000000000000, -0.020150476814457717, 0.000000000000000000, -0.018311630846033072, 0.000000000000000000, -0.016688898584430931, 0.000000000000000000, -0.015244551273554119, 0.000000000000000000, -0.013949535528809384, 0.000000000000000000, -0.012781121134483500, 0.000000000000000000, -0.011721272439718031, 0.000000000000000000, -0.010755496313062237, 0.000000000000000000, -0.009872011537848968, 0.000000000000000000, -0.009061139683747135, 0.000000000000000000, -0.008314851520255883, 0.000000000000000000, -0.007626424567085178, 0.000000000000000000, -0.006990181306750123, 0.000000000000000000, -0.006401286783911277, 0.000000000000000000, -0.005855590504225607, 0.000000000000000000, -0.005349501779412148, 0.000000000000000000, -0.004879890607273629, 0.000000000000000000, -0.004444008249195281, 0.000000000000000000, -0.004039423148679463, 0.000000000000000000, -0.003663968905292113, 0.000000000000000000, -0.003315701801645692, 0.000000000000000000, -0.002992865959230020, 0.000000000000000000, -0.002693864631427011, 0.000000000000000000, -0.002417236466741860, 0.000000000000000000, -0.002161635822912572, 0.000000000000000000, -0.001925816401721571, 0.000000000000000000, -0.001708617620559288, 0.000000000000000000, -0.001508953250504772, 0.000000000000000000, -0.001325801939712967, 0.000000000000000000, -0.001158199311056187, 0.000000000000000000, -0.001005231378692548, 0.000000000000000000, -0.000866029072658519, 0.000000000000000000, -0.000739763696285730, 0.000000000000000000, -0.000625643170080525, 0.000000000000000000, -0.000522908939129710, 0.000000000000000000, -0.000430833440231578, 0.000000000000000000, -0.000348718040684899, 0.000000000000000000};
static const float32_t FIR_HILB_Q_coeffs[IQ_HILBERT_TAPS] = {-0.000056786545255326, 263.1346287525080E-18, -0.000071793697491707, 391.3417109364890E-18, -0.000088874802368221, 336.4125900077540E-18, -0.000108170884956264, 392.7791496728210E-18, -0.000129819032785321, 590.4838201243310E-18, -0.000153950719729993, 596.8112038672230E-18, -0.000180690067531663, 746.3322026715040E-18, -0.000210152062867355, 1.093346048537350E-15, -0.000242440749545727, 1.065677310743770E-15, -0.000277647416860387, 1.039585124576320E-15, -0.000315848806349421, 1.538815826232670E-15, -0.000357105360160803, 1.666207014701170E-15, -0.000401459534874215, 1.539685561133540E-15, -0.000448934204947304, 2.660369443631100E-15, -0.000499531179987328, 3.040479628793550E-15, -0.000553229859656923, 1.044067344343770E-15, -0.000609986049322792, 2.032026148997660E-15, -0.000669730958505527, 6.994036442534390E-15, -0.000732370402749788, 7.411458451156190E-15, -0.000797784227648926, 3.101364677149160E-15, -0.000865825971964948, 2.594164153512510E-15, -0.000936322783811445, 5.348323407953660E-15, -0.001009075601815882, 5.884744611617880E-15, -0.001083859609627728, 5.492731038694370E-15, -0.001160424969305250, 6.445309139376350E-15, -0.001238497835651963, 6.872918232978370E-15, -0.001317781650032627, 6.663418858584270E-15, -0.001397958708679671, 8.133197857946460E-15, -0.001478691996873688, 9.905768905631310E-15, -0.001559627276797271, 10.20890941488510E-15, -0.001640395413295664, 11.20755554836600E-15, -0.001720614918466468, 12.64985408146260E-15, -0.001799894692718967, 12.97295013621260E-15, -0.001877836936972348, 14.24353199778040E-15, -0.001954040207976172, 16.47390711328630E-15, -0.002028102586337237, 18.06444101550620E-15, -0.002099624924798889, 19.78135011594340E-15, -0.002168214142707746, 21.50931561512560E-15, -0.002233486531418778, 23.52243144574740E-15, -0.002295071034610703, 26.83685625665210E-15, -0.002352612467220998, 30.78295485673010E-15, -0.002405774636936983, 35.50722610113330E-15, -0.002454243332825246, 40.92479516392550E-15, -0.002497729146841703, 46.97877831911210E-15, -0.002535970095673103, 56.20264720417670E-15, -0.002568734012302652, 69.94947997469540E-15, -0.002595820679318180, 90.43954876597660E-15, -0.002617063678801859, 127.2729571918730E-15, -0.002632331936897800, 212.5921889885520E-15, -0.002641530944731846, 638.3507757132560E-15, 0.997207486382091335, 638.3507757132560E-15, -0.002641530944731846, 212.5921889885520E-15, -0.002632331936897800, 127.2729571918730E-15, -0.002617063678801859, 90.43954876597660E-15, -0.002595820679318180, 69.94947997469540E-15, -0.002568734012302652, 56.20264720417670E-15, -0.002535970095673103, 46.97877831911210E-15, -0.002497729146841703, 40.92479516392550E-15, -0.002454243332825246, 35.50722610113330E-15, -0.002405774636936983, 30.78295485673010E-15, -0.002352612467220998, 26.83685625665210E-15, -0.002295071034610703, 23.52243144574740E-15, -0.002233486531418778, 21.50931561512560E-15, -0.002168214142707746, 19.78135011594340E-15, -0.002099624924798889, 18.06444101550620E-15, -0.002028102586337237, 16.47390711328630E-15, -0.001954040207976172, 14.24353199778040E-15, -0.001877836936972348, 12.97295013621260E-15, -0.001799894692718967, 12.64985408146260E-15, -0.001720614918466468, 11.20755554836600E-15, -0.001640395413295664, 10.20890941488510E-15, -0.001559627276797271, 9.905768905631310E-15, -0.001478691996873688, 8.133197857946460E-15, -0.001397958708679671, 6.663418858584270E-15, -0.001317781650032627, 6.872918232978370E-15, -0.001238497835651963, 6.445309139376350E-15, -0.001160424969305250, 5.492731038694370E-15, -0.001083859609627728, 5.884744611617880E-15, -0.001009075601815882, 5.348323407953660E-15, -0.000936322783811445, 2.594164153512510E-15, -0.000865825971964948, 3.101364677149160E-15, -0.000797784227648926, 7.411458451156190E-15, -0.000732370402749788, 6.994036442534390E-15, -0.000669730958505527, 2.032026148997660E-15, -0.000609986049322792, 1.044067344343770E-15, -0.000553229859656923, 3.040479628793550E-15, -0.000499531179987328, 2.660369443631100E-15, -0.000448934204947304, 1.539685561133540E-15, -0.000401459534874215, 1.666207014701170E-15, -0.000357105360160803, 1.538815826232670E-15, -0.000315848806349421, 1.039585124576320E-15, -0.000277647416860387, 1.065677310743770E-15, -0.000242440749545727, 1.093346048537350E-15, -0.000210152062867355, 746.3322026715040E-18, -0.000180690067531663, 596.8112038672230E-18, -0.000153950719729993, 590.4838201243310E-18, -0.000129819032785321, 392.7791496728210E-18, -0.000108170884956264, 336.4125900077540E-18, -0.000088874802368221, 391.3417109364890E-18, -0.000071793697491707, 263.1346287525080E-18, -0.000056786545255326};

static const IIR_BIQUAD_FILTER IIR_Biquad_Filters[IIR_FILTERS_COUNT] = (const IIR_BIQUAD_FILTER[]){
	//IIR RX/TX IIR Lowpass Fs=48000 Apass=1 Astop=120 (Cheb I) // coefficients converted to Direct-Form II Transposed by MATLAB // https://ua3reo.ru/services/matlab_to_arm_biquad.php
	{
		.width = 100, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0002143270359125, 0, 0, 0, 0, 1, 0, -1, 1.999528518437, -0.9995713459282, 1, 0, 0, 0, 0},
	},
	{
		.width = 150, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0003214561070949, 0, 0, 0, 0, 1, 0, -1, 1.999260736687, -0.9993570877858, 1, 0, 0, 0, 0},
	},
	{
		.width = 200, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0004285622244523, 0, 0, 0, 0, 1, 0, -1, 1.998971604131, -0.9991428755511, 1, 0, 0, 0, 0},
	},
	{
		.width = 250, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0005356453959978, 0, 0, 0, 0, 1, 0, -1, 1.998661128432, -0.998928709208, 1, 0, 0, 0, 0},
	},
	{
		.width = 300, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0006427056297418, 0, 0, 0, 0, 1, 0, -1, 1.99832931748, -0.9987145887405, 1, 0, 0, 0, 0},
	},
	{
		.width = 350, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0007497429336899, 0, 0, 0, 0, 1, 0, -1, 1.997976179388, -0.9985005141326, 1, 0, 0, 0, 0},
	},
	{
		.width = 400, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0008567573158444, 0, 0, 0, 0, 1, 0, -1, 1.997601722498, -0.9982864853683, 1, 0, 0, 0, 0},
	},
	{
		.width = 450, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.0009637487842045, 0, 0, 0, 0, 1, 0, -1, 1.997205955375, -0.9980725024316, 1, 0, 0, 0, 0},
	},
	{
		.width = 500, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001070717346765, 0, 0, 0, 0, 1, 0, -1, 1.99678888681, -0.9978585653065, 1, 0, 0, 0, 0},
	},
	{
		.width = 550, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001177663011516, 0, 0, 0, 0, 1, 0, -1, 1.99635052582, -0.997644673977, 1, 0, 0, 0, 0},
	},
	{
		.width = 600, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001284585786447, 0, 0, 0, 0, 1, 0, -1, 1.995890881647, -0.9974308284271, 1, 0, 0, 0, 0},
	},
	{
		.width = 650, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001391485679541, 0, 0, 0, 0, 1, 0, -1, 1.995409963756, -0.9972170286409, 1, 0, 0, 0, 0},
	},
	{
		.width = 700, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001498362698778, 0, 0, 0, 0, 1, 0, -1, 1.99490778184, -0.9970032746024, 1, 0, 0, 0, 0},
	},
	{
		.width = 750, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001605216852135, 0, 0, 0, 0, 1, 0, -1, 1.994384345814, -0.9967895662957, 1, 0, 0, 0, 0},
	},
	{
		.width = 800, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001712048147584, 0, 0, 0, 0, 1, 0, -1, 1.993839665819, -0.9965759037048, 1, 0, 0, 0, 0},
	},
	{
		.width = 850, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001818856593095, 0, 0, 0, 0, 1, 0, -1, 1.993273752219, -0.9963622868138, 1, 0, 0, 0, 0},
	},
	{
		.width = 900, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.001925642196633, 0, 0, 0, 0, 1, 0, -1, 1.992686615605, -0.9961487156067, 1, 0, 0, 0, 0},
	},
	{
		.width = 950, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.00203240496616, 0, 0, 0, 0, 1, 0, -1, 1.992078266787, -0.9959351900677, 1, 0, 0, 0, 0},
	},
	{
		.width = 1000, //gauss Q30
		.type = IIR_BIQUAD_LPF_GAUSS,
		.stages = 3,
		.coeffs = (float32_t[]){0.002139144909634, 0, 0, 0, 0, 1, 0, -1, 1.991448716805, -0.9957217101807, 1, 0, 0, 0, 0},
	},
};

//////Decimation by 2
//FIR
arm_fir_decimate_instance_f32 DECIMATE_2_RX1_AUDIO_I =
	{
		.M = 2,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_2_RX1_AUDIO_Q =
	{
		.M = 2,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_2_RX2_AUDIO_I =
	{
		.M = 2,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_2_RX2_AUDIO_Q =
	{
		.M = 2,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
//IIR
arm_biquad_cascade_df2T_instance_f32 DECIMATE_2_IIR_RX1_AUDIO_I =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.8384843639921, 0, 0, 0, 0, 1, 0.5130084793341, 1, 0.1784114407685, -0.6967733943344, 0.8744089756375, 0, 0, 0, 0, 1, 1.046379755684, 1, 0.3420998857106, -0.3982809814397, 1.83222755502, 0, 0, 0, 0, 1, 1.831496024383, 1, 0.5072844084012, -0.1179052535088, 0.01953722920982, 0, 0, 0, 0, 1, 0.3029841730578, 1, 0.09694668293684, -0.9095549467394, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_2_IIR_RX1_AUDIO_Q =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.8384843639921, 0, 0, 0, 0, 1, 0.5130084793341, 1, 0.1784114407685, -0.6967733943344, 0.8744089756375, 0, 0, 0, 0, 1, 1.046379755684, 1, 0.3420998857106, -0.3982809814397, 1.83222755502, 0, 0, 0, 0, 1, 1.831496024383, 1, 0.5072844084012, -0.1179052535088, 0.01953722920982, 0, 0, 0, 0, 1, 0.3029841730578, 1, 0.09694668293684, -0.9095549467394, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_2_IIR_RX2_AUDIO_I =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.8384843639921, 0, 0, 0, 0, 1, 0.5130084793341, 1, 0.1784114407685, -0.6967733943344, 0.8744089756375, 0, 0, 0, 0, 1, 1.046379755684, 1, 0.3420998857106, -0.3982809814397, 1.83222755502, 0, 0, 0, 0, 1, 1.831496024383, 1, 0.5072844084012, -0.1179052535088, 0.01953722920982, 0, 0, 0, 0, 1, 0.3029841730578, 1, 0.09694668293684, -0.9095549467394, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_2_IIR_RX2_AUDIO_Q =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.8384843639921, 0, 0, 0, 0, 1, 0.5130084793341, 1, 0.1784114407685, -0.6967733943344, 0.8744089756375, 0, 0, 0, 0, 1, 1.046379755684, 1, 0.3420998857106, -0.3982809814397, 1.83222755502, 0, 0, 0, 0, 1, 1.831496024383, 1, 0.5072844084012, -0.1179052535088, 0.01953722920982, 0, 0, 0, 0, 1, 0.3029841730578, 1, 0.09694668293684, -0.9095549467394, 1, 0, 0, 0, 0},
};

//////Decimation by 4
//FIR
arm_fir_decimate_instance_f32 DECIMATE_4_RX1_AUDIO_I =
	{
		.M = 4,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_4_RX1_AUDIO_Q =
	{
		.M = 4,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_4_RX2_AUDIO_I =
	{
		.M = 4,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_4_RX2_AUDIO_Q =
	{
		.M = 4,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
//IIR
arm_biquad_cascade_df2T_instance_f32 DECIMATE_4_IIR_RX1_AUDIO_I =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.6737499659657, 0, 0, 0, 0, 1, -1.102065194995, 1, 1.353694541279, -0.7896377861467, 0.53324811147, 0, 0, 0, 0, 1, -0.5853766477218, 1, 1.289175897987, -0.5882714065646, 0.6143152247695, 0, 0, 0, 0, 1, 1.182778527244, 1, 1.236309127239, -0.4063767082903, 0.01708381580242, 0, 0, 0, 0, 1, -1.245590418009, 1, 1.418191929315, -0.9374008035325, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_4_IIR_RX1_AUDIO_Q =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.6737499659657, 0, 0, 0, 0, 1, -1.102065194995, 1, 1.353694541279, -0.7896377861467, 0.53324811147, 0, 0, 0, 0, 1, -0.5853766477218, 1, 1.289175897987, -0.5882714065646, 0.6143152247695, 0, 0, 0, 0, 1, 1.182778527244, 1, 1.236309127239, -0.4063767082903, 0.01708381580242, 0, 0, 0, 0, 1, -1.245590418009, 1, 1.418191929315, -0.9374008035325, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_4_IIR_RX2_AUDIO_I =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.6737499659657, 0, 0, 0, 0, 1, -1.102065194995, 1, 1.353694541279, -0.7896377861467, 0.53324811147, 0, 0, 0, 0, 1, -0.5853766477218, 1, 1.289175897987, -0.5882714065646, 0.6143152247695, 0, 0, 0, 0, 1, 1.182778527244, 1, 1.236309127239, -0.4063767082903, 0.01708381580242, 0, 0, 0, 0, 1, -1.245590418009, 1, 1.418191929315, -0.9374008035325, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_4_IIR_RX2_AUDIO_Q =
	{
		.numStages = 9,
		.pState = (float32_t[9 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[9 * 5]){0.6737499659657, 0, 0, 0, 0, 1, -1.102065194995, 1, 1.353694541279, -0.7896377861467, 0.53324811147, 0, 0, 0, 0, 1, -0.5853766477218, 1, 1.289175897987, -0.5882714065646, 0.6143152247695, 0, 0, 0, 0, 1, 1.182778527244, 1, 1.236309127239, -0.4063767082903, 0.01708381580242, 0, 0, 0, 0, 1, -1.245590418009, 1, 1.418191929315, -0.9374008035325, 1, 0, 0, 0, 0},
};

//////Decimation by 8
//FIR
arm_fir_decimate_instance_f32 DECIMATE_8_FIR_RX1_AUDIO_I =
	{
		.M = 8,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_8_FIR_RX1_AUDIO_Q =
	{
		.M = 8,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_8_FIR_RX2_AUDIO_I =
	{
		.M = 8,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
arm_fir_decimate_instance_f32 DECIMATE_8_FIR_RX2_AUDIO_Q =
	{
		.M = 8,
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[4]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};
//IIR
arm_biquad_cascade_df2T_instance_f32 DECIMATE_8_IIR_RX1_AUDIO_I =
	{
		.numStages = 5,
		.pState = (float32_t[5 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[5 * 5]){0.1466070876161, 0, 0, 0, 0, 1, 1.045041909455, 1, 1.748614902139, -0.7769236480316, 0.003648425824363, 0, 0, 0, 0, 1, -0.5702258292353, 1, 1.818349952659, -0.9054865464024, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_8_IIR_RX1_AUDIO_Q =
	{
		.numStages = 5,
		.pState = (float32_t[5 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[5 * 5]){0.1466070876161, 0, 0, 0, 0, 1, 1.045041909455, 1, 1.748614902139, -0.7769236480316, 0.003648425824363, 0, 0, 0, 0, 1, -0.5702258292353, 1, 1.818349952659, -0.9054865464024, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_8_IIR_RX2_AUDIO_I =
	{
		.numStages = 5,
		.pState = (float32_t[5 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[5 * 5]){0.1466070876161, 0, 0, 0, 0, 1, 1.045041909455, 1, 1.748614902139, -0.7769236480316, 0.003648425824363, 0, 0, 0, 0, 1, -0.5702258292353, 1, 1.818349952659, -0.9054865464024, 1, 0, 0, 0, 0},
};
arm_biquad_cascade_df2T_instance_f32 DECIMATE_8_IIR_RX2_AUDIO_Q =
	{
		.numStages = 5,
		.pState = (float32_t[5 * 2]){0},
		.pCoeffs = (float32_t *)(const float32_t[5 * 5]){0.1466070876161, 0, 0, 0, 0, 1, 1.045041909455, 1, 1.748614902139, -0.7769236480316, 0.003648425824363, 0, 0, 0, 0, 1, -0.5702258292353, 1, 1.818349952659, -0.9054865464024, 1, 0, 0, 0, 0},
};

//Public variables
const uint32_t AUTIO_FILTERS_HPF_LIST[CW_HPF_COUNT] = {0, 60, 100, 200, 300, 400, 500, 600};
const uint32_t AUTIO_FILTERS_LPF_CW_LIST[CW_LPF_COUNT] = {100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000};
const uint32_t AUTIO_FILTERS_LPF_SSB_LIST[SSB_LPF_COUNT] = {1400, 1600, 1800, 2100, 2300, 2500, 2700, 2900, 3000, 3200, 3400};
const uint32_t AUTIO_FILTERS_LPF_AM_LIST[AM_LPF_COUNT] = {2100, 2300, 2500, 2700, 2900, 3000, 3200, 3400, 3600, 3800, 4000, 4500, 5000, 6000, 7000, 8000, 9000, 10000};
const uint32_t AUTIO_FILTERS_LPF_NFM_LIST[NFM_LPF_COUNT] = {5000, 6000, 7000, 8000, 9000, 10000, 15000, 20000};

arm_fir_instance_f32 FIR_RX1_Hilbert_I; // filter instances
arm_fir_instance_f32 FIR_RX1_Hilbert_Q;
arm_fir_instance_f32 FIR_RX2_Hilbert_I;
arm_fir_instance_f32 FIR_RX2_Hilbert_Q;
arm_fir_instance_f32 FIR_TX_Hilbert_I;
arm_fir_instance_f32 FIR_TX_Hilbert_Q;
arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_Q;
arm_biquad_cascade_df2T_instance_f32 IIR_RX1_GAUSS;
arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_Q;
arm_biquad_cascade_df2T_instance_f32 IIR_RX2_GAUSS;
arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_I;
arm_biquad_cascade_df2T_instance_f32 IIR_RX1_Squelch_HPF;
arm_biquad_cascade_df2T_instance_f32 IIR_RX2_Squelch_HPF;
arm_biquad_cascade_df2T_instance_f32 NOTCH_RX1_FILTER = {NOTCH_STAGES, NOTCH_RX1_State, NOTCH_RX1_Coeffs}; // manual notch filter
arm_biquad_cascade_df2T_instance_f32 NOTCH_RX2_FILTER = {NOTCH_STAGES, NOTCH_RX2_State, NOTCH_RX2_Coeffs};
arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_I_FILTER = {NOTCH_STAGES, NOTCH_FFT_I_State, NOTCH_RX1_Coeffs};
arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_Q_FILTER = {NOTCH_STAGES, NOTCH_FFT_Q_State, NOTCH_RX1_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_RX_LOW_FILTER = {EQ_STAGES, EQ_RX_LOW_FILTER_State, EQ_RX_LOW_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_RX_MID_FILTER = {EQ_STAGES, EQ_RX_MID_FILTER_State, EQ_RX_MID_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_RX_HIG_FILTER = {EQ_STAGES, EQ_RX_HIG_FILTER_State, EQ_RX_HIG_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_MIC_LOW_FILTER = {EQ_STAGES, EQ_MIC_LOW_FILTER_State, EQ_MIC_LOW_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_MIC_MID_FILTER = {EQ_STAGES, EQ_MIC_MID_FILTER_State, EQ_MIC_MID_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 EQ_MIC_HIG_FILTER = {EQ_STAGES, EQ_MIC_HIG_FILTER_State, EQ_MIC_HIG_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HSHELF_FILTER = {EQ_STAGES, AGC_RX1_KW_HSHELF_FILTER_State, AGC_RX_KW_HSHELF_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HPASS_FILTER = {EQ_STAGES, AGC_RX1_KW_HPASS_FILTER_State, AGC_RX_KW_HPASS_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HSHELF_FILTER = {EQ_STAGES, AGC_RX2_KW_HSHELF_FILTER_State, AGC_RX_KW_HSHELF_FILTER_Coeffs};
arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HPASS_FILTER = {EQ_STAGES, AGC_RX2_KW_HPASS_FILTER_State, AGC_RX_KW_HPASS_FILTER_Coeffs};
volatile bool NeedReinitNotch = false;			   // need to re-initialize the manual Notch filter
volatile bool NeedReinitAudioFilters = false;	   // need to re-initialize Audio filters
volatile bool NeedReinitAudioFiltersClean = false; //also clean state

// Prototypes
static void calcBiquad(BIQUAD_TYPE type, uint32_t Fc, uint32_t Fs, float32_t Q, float32_t peakGain, float32_t *outCoeffs);									  // automatic calculation of the Biquad filter for Notch
static IIR_BIQUAD_FILTER *getIIRFilter(IIR_BIQUAD_FILTER_TYPE type, uint_fast16_t width);																	  // get filter from collection
static void arm_biquad_cascade_df2T_initNoClean_f32(arm_biquad_cascade_df2T_instance_f32 *S, uint8_t numStages, const float32_t *pCoeffs, float32_t *pState); //init without state-clean version
static void fill_biquad_coeffs(iir_filter_t *filter, float32_t *coeffs, uint8_t sect_num);

// initialize audio filters
void InitAudioFilters(void)
{
	//TX Hilbert
	arm_fir_init_f32(&FIR_TX_Hilbert_I, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_I_coeffs, (float32_t *)&Fir_Tx_Hilbert_State_I[0], AUDIO_BUFFER_HALF_SIZE);
	arm_fir_init_f32(&FIR_TX_Hilbert_Q, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_Q_coeffs, (float32_t *)&Fir_Tx_Hilbert_State_Q[0], AUDIO_BUFFER_HALF_SIZE);

	//AGC K-Weight LKFS BS.1770
	calcBiquad(BIQUAD_highShelf, 1500, TRX_SAMPLERATE, 1.0f / sqrtf(2), 4.0f, AGC_RX_KW_HSHELF_FILTER_Coeffs);
	calcBiquad(BIQUAD_highpass, 38, TRX_SAMPLERATE, 0.5f, 0.0f, AGC_RX_KW_HPASS_FILTER_Coeffs);

	//Other
	InitNoiseReduction();
	InitNotchFilter();
	InitAutoNotchReduction();
	InitVAD();
	ReinitAudioFilters();
}

// reinitialize audio filters
void ReinitAudioFilters(void)
{
	//RX1 Hilbert
	if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
	{
		arm_fir_init_f32(&FIR_RX1_Hilbert_I, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_I_coeffs, (float32_t *)&Fir_RX1_Hilbert_State_I[0], AUDIO_BUFFER_HALF_SIZE);
		arm_fir_init_f32(&FIR_RX1_Hilbert_Q, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_Q_coeffs, (float32_t *)&Fir_RX1_Hilbert_State_Q[0], AUDIO_BUFFER_HALF_SIZE);
	}
	else
	{
		arm_fir_init_f32(&FIR_RX1_Hilbert_I, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_I_coeffs, (float32_t *)&Fir_RX1_Hilbert_State_I[0], AUDIO_BUFFER_HALF_SIZE);
		arm_fir_init_f32(&FIR_RX1_Hilbert_Q, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_Q_coeffs, (float32_t *)&Fir_RX1_Hilbert_State_Q[0], AUDIO_BUFFER_HALF_SIZE);
	}

	//RX2 Hilbert
	if (SecondaryVFO()->Mode == TRX_MODE_CW_L || SecondaryVFO()->Mode == TRX_MODE_CW_U)
	{
		arm_fir_init_f32(&FIR_RX2_Hilbert_I, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_I_coeffs, (float32_t *)&Fir_RX2_Hilbert_State_I[0], AUDIO_BUFFER_HALF_SIZE);
		arm_fir_init_f32(&FIR_RX2_Hilbert_Q, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_Q_coeffs, (float32_t *)&Fir_RX2_Hilbert_State_Q[0], AUDIO_BUFFER_HALF_SIZE);
	}
	else
	{
		arm_fir_init_f32(&FIR_RX2_Hilbert_I, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_I_coeffs, (float32_t *)&Fir_RX2_Hilbert_State_I[0], AUDIO_BUFFER_HALF_SIZE);
		arm_fir_init_f32(&FIR_RX2_Hilbert_Q, IQ_HILBERT_TAPS, (float32_t *)&FIR_HILB_Q_coeffs, (float32_t *)&Fir_RX2_Hilbert_State_Q[0], AUDIO_BUFFER_HALF_SIZE);
	}

	//init
	iir_filter_t *filter = biquad_create(IIR_STAGES);
	//
	uint32_t hpf_rx1_width = 300; //default settings
	if (CurrentVFO()->HPF_Filter_Width > 0)
		hpf_rx1_width = CurrentVFO()->HPF_Filter_Width;
	//
	uint32_t hpf_rx2_width = 300; //default settings
	if (SecondaryVFO()->HPF_Filter_Width > 0)
		hpf_rx2_width = SecondaryVFO()->HPF_Filter_Width;
	//
	uint32_t lpf_rx1_width = 2700; //default settings
	if (CurrentVFO()->LPF_RX_Filter_Width > 0)
		lpf_rx1_width = CurrentVFO()->LPF_RX_Filter_Width;
	if(lpf_rx1_width < hpf_rx1_width)
		lpf_rx1_width = hpf_rx1_width + 100;
	//
	uint32_t lpf_rx2_width = 2700; //default settings
	if (SecondaryVFO()->LPF_RX_Filter_Width > 0)
		lpf_rx2_width = SecondaryVFO()->LPF_RX_Filter_Width;
	if(lpf_rx2_width < hpf_rx2_width)
		lpf_rx2_width = hpf_rx2_width + 100;
	//
	uint32_t lpf_tx_width = 2700; //default settings
	if (CurrentVFO()->LPF_TX_Filter_Width > 0)
		lpf_tx_width = CurrentVFO()->LPF_TX_Filter_Width;
	if(lpf_tx_width < hpf_rx1_width)
		lpf_tx_width = hpf_rx1_width + 100;
	//
	
	//LPF RX1
	biquad_init_lowpass(filter, TRX_SAMPLERATE, lpf_rx1_width);
	fill_biquad_coeffs(filter, IIR_RX1_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX1_LPF_I, IIR_STAGES, IIR_RX1_Coeffs, (float32_t *)&IIR_RX1_LPF_I_State[0]);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX1_LPF_Q, IIR_STAGES, IIR_RX1_Coeffs, (float32_t *)&IIR_RX1_LPF_Q_State[0]);
	
	//LPF TX
	biquad_init_lowpass(filter, TRX_SAMPLERATE, lpf_tx_width);
	fill_biquad_coeffs(filter, IIR_TX_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_TX_LPF_I, IIR_STAGES, IIR_TX_Coeffs, (float32_t *)&IIR_TX_LPF_I_State[0]);
	
	//RX1 GAUSS
	IIR_BIQUAD_FILTER *gauss_filter = getIIRFilter(IIR_BIQUAD_LPF_GAUSS, 1000);
	if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
		gauss_filter = getIIRFilter(IIR_BIQUAD_LPF_GAUSS, lpf_rx1_width);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX1_GAUSS, gauss_filter->stages, (float32_t *)gauss_filter->coeffs, (float32_t *)&IIR_RX1_GAUSS_State[0]);

	//LPF RX2
	biquad_init_lowpass(filter, TRX_SAMPLERATE, lpf_rx2_width);
	fill_biquad_coeffs(filter, IIR_RX2_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX2_LPF_I, IIR_STAGES, IIR_RX2_Coeffs, (float32_t *)&IIR_RX2_LPF_I_State[0]);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX2_LPF_Q, IIR_STAGES, IIR_RX2_Coeffs, (float32_t *)&IIR_RX2_LPF_Q_State[0]);

	//RX2 GAUSS
	if (SecondaryVFO()->Mode == TRX_MODE_CW_L || SecondaryVFO()->Mode == TRX_MODE_CW_U)
		gauss_filter = getIIRFilter(IIR_BIQUAD_LPF_GAUSS, lpf_rx2_width);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX2_GAUSS, gauss_filter->stages, (float32_t *)gauss_filter->coeffs, (float32_t *)&IIR_RX2_GAUSS_State[0]);

	//HPF
	biquad_init_highpass(filter, TRX_SAMPLERATE, hpf_rx1_width);
	fill_biquad_coeffs(filter, IIR_RX1_HPF_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX1_HPF_I, IIR_STAGES, IIR_RX1_HPF_Coeffs, (float32_t *)&IIR_RX1_HPF_I_State[0]);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_TX_HPF_I, IIR_STAGES, IIR_RX1_HPF_Coeffs, (float32_t *)&IIR_TX_HPF_I_State[0]);

	biquad_init_highpass(filter, TRX_SAMPLERATE, hpf_rx2_width);
	fill_biquad_coeffs(filter, IIR_RX2_HPF_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX2_HPF_I, IIR_STAGES, IIR_RX2_HPF_Coeffs, (float32_t *)&IIR_RX2_HPF_I_State[0]);

	//FM Squelch
	if (CurrentVFO()->LPF_RX_Filter_Width > 15000 || CurrentVFO()->LPF_RX_Filter_Width == 0)
		biquad_init_highpass(filter, TRX_SAMPLERATE, 20000);
	else
		biquad_init_highpass(filter, TRX_SAMPLERATE, 15000);
	fill_biquad_coeffs(filter, IIR_RX1_HPF_SQL_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX1_Squelch_HPF, IIR_STAGES, IIR_RX2_HPF_SQL_Coeffs, (float32_t *)&IIR_RX1_HPF_SQL_State[0]);
	
	if (SecondaryVFO()->LPF_RX_Filter_Width > 15000 || SecondaryVFO()->LPF_RX_Filter_Width == 0)
		biquad_init_highpass(filter, TRX_SAMPLERATE, 20000);
	else
		biquad_init_highpass(filter, TRX_SAMPLERATE, 15000);
	fill_biquad_coeffs(filter, IIR_RX2_HPF_SQL_Coeffs, IIR_STAGES);
	arm_biquad_cascade_df2T_initNoClean_f32(&IIR_RX2_Squelch_HPF, IIR_STAGES, IIR_RX2_HPF_SQL_Coeffs, (float32_t *)&IIR_RX2_HPF_SQL_State[0]);

	//RX Equalizer
	calcBiquad(BIQUAD_peak, 400, TRX_SAMPLERATE, 0.5f, TRX.RX_EQ_LOW, EQ_RX_LOW_FILTER_Coeffs);
	calcBiquad(BIQUAD_peak, 1000, TRX_SAMPLERATE, 1.0f, TRX.RX_EQ_MID, EQ_RX_MID_FILTER_Coeffs);
	calcBiquad(BIQUAD_peak, 2000, TRX_SAMPLERATE, 1.5f, TRX.RX_EQ_HIG, EQ_RX_HIG_FILTER_Coeffs);

	//MIC Equalizer
	calcBiquad(BIQUAD_peak, 400, TRX_SAMPLERATE, 0.5f, TRX.MIC_EQ_LOW, EQ_MIC_LOW_FILTER_Coeffs);
	calcBiquad(BIQUAD_peak, 1000, TRX_SAMPLERATE, 1.0f, TRX.MIC_EQ_MID, EQ_MIC_MID_FILTER_Coeffs);
	calcBiquad(BIQUAD_peak, 2000, TRX_SAMPLERATE, 1.5f, TRX.MIC_EQ_HIG, EQ_MIC_HIG_FILTER_Coeffs);

	//All done
	NeedReinitAudioFilters = false;
	NeedReinitAudioFiltersClean = false;
}

// initialize the manual Notch filter
void InitNotchFilter(void)
{
	calcBiquad(BIQUAD_notch, CurrentVFO()->NotchFC / 2, TRX_SAMPLERATE, 0.5f, 0, NOTCH_RX1_Coeffs);
	calcBiquad(BIQUAD_notch, SecondaryVFO()->NotchFC / 2, TRX_SAMPLERATE, 0.5f, 0, NOTCH_RX2_Coeffs);
	NeedReinitNotch = false;
}

// start DC corrector
void dc_filter(float32_t *Buffer, int16_t blockSize, DC_FILTER_STATE stateNum) // removes the DC component of the signal
{
	static const float32_t A1 = (1.0f - 0.00048828125f); // (1-2^(-11))

	for (uint16_t i = 0; i < blockSize; i++)
	{
		float32_t sampleIn = Buffer[i];
		float32_t sampleOut = 0;
		float32_t delta_x = sampleIn - DC_Filter_State[stateNum].x_prev;
		float32_t a1_y_prev = A1 * DC_Filter_State[stateNum].y_prev;
		sampleOut = delta_x + a1_y_prev;
		DC_Filter_State[stateNum].x_prev = sampleIn;
		DC_Filter_State[stateNum].y_prev = sampleOut;
		Buffer[i] = sampleOut;
	}
}

// get filter from collection
static IIR_BIQUAD_FILTER *getIIRFilter(IIR_BIQUAD_FILTER_TYPE type, uint_fast16_t width)
{
	for (uint16_t i = 0; i < IIR_FILTERS_COUNT; i++)
		if (IIR_Biquad_Filters[i].type == type && IIR_Biquad_Filters[i].width == width)
			return (IIR_BIQUAD_FILTER *)&IIR_Biquad_Filters[i];
	println("Wrong filter length ", type, " ", (uint16_t)width);
	return (IIR_BIQUAD_FILTER *)&IIR_Biquad_Filters[0];
}

// automatic calculation of the Biquad filter
static void calcBiquad(BIQUAD_TYPE type, uint32_t Fc, uint32_t Fs, float32_t Q, float32_t peakGain, float32_t *outCoeffs)
{
	float32_t a0, a1, a2, b1, b2, norm;

	float32_t V = powf(10.0f, fabsf(peakGain) / 20.0f);
	float32_t K = tanf(PI * Fc / Fs);
	switch (type)
	{
	case BIQUAD_onepolelp:
		b1 = expf(-2.0f * PI * (Fc / Fs));
		a0 = 1.0f - b1;
		b1 = -b1;
		a1 = a2 = b2 = 0.0f;
		break;

	case BIQUAD_onepolehp:
		b1 = -expf(-2.0f * PI * (0.5f - Fc / Fs));
		a0 = 1.0f + b1;
		b1 = -b1;
		a1 = a2 = b2 = 0.0f;
		break;

	case BIQUAD_lowpass:
		norm = 1.0f / (1.0f + K / Q + K * K);
		a0 = K * K * norm;
		a1 = 2.0f * a0;
		a2 = a0;
		b1 = 2.0f * (K * K - 1.0f) * norm;
		b2 = (1.0f - K / Q + K * K) * norm;
		break;

	case BIQUAD_highpass:
		norm = 1.0f / (1.0f + K / Q + K * K);
		a0 = 1.0f * norm;
		a1 = -2.0f * a0;
		a2 = a0;
		b1 = 2.0f * (K * K - 1.0f) * norm;
		b2 = (1.0f - K / Q + K * K) * norm;
		break;

	case BIQUAD_bandpass:
		norm = 1.0f / (1.0f + K / Q + K * K);
		a0 = K / Q * norm;
		a1 = 0.0f;
		a2 = -a0;
		b1 = 2.0f * (K * K - 1.0f) * norm;
		b2 = (1.0f - K / Q + K * K) * norm;
		break;

	case BIQUAD_notch:
		norm = 1.0f / (1.0f + K / Q + K * K);
		a0 = (1.0f + K * K) * norm;
		a1 = 2.0f * (K * K - 1.0f) * norm;
		a2 = a0;
		b1 = a1;
		b2 = (1.0f - K / Q + K * K) * norm;
		break;

	case BIQUAD_peak:
		if (peakGain >= 0.0f)
		{
			norm = 1.0f / (1.0f + 1.0f / Q * K + K * K);
			a0 = (1.0f + V / Q * K + K * K) * norm;
			a1 = 2.0f * (K * K - 1.0f) * norm;
			a2 = (1.0f - V / Q * K + K * K) * norm;
			b1 = a1;
			b2 = (1.0f - 1.0f / Q * K + K * K) * norm;
		}
		else
		{
			norm = 1.0f / (1.0f + V / Q * K + K * K);
			a0 = (1.0f + 1.0f / Q * K + K * K) * norm;
			a1 = 2.0f * (K * K - 1.0f) * norm;
			a2 = (1.0f - 1.0f / Q * K + K * K) * norm;
			b1 = a1;
			b2 = (1.0f - V / Q * K + K * K) * norm;
		}
		break;
	case BIQUAD_lowShelf:
		if (peakGain >= 0.0f)
		{
			norm = 1.0f / (1.0f + SQRT2 * K + K * K);
			a0 = (1.0f + sqrtf(2.0f * V) * K + V * K * K) * norm;
			a1 = 2.0f * (V * K * K - 1.0f) * norm;
			a2 = (1.0f - sqrtf(2.0f * V) * K + V * K * K) * norm;
			b1 = 2.0f * (K * K - 1.0f) * norm;
			b2 = (1.0f - SQRT2 * K + K * K) * norm;
		}
		else
		{
			norm = 1.0f / (1.0f + sqrtf(2.0f * V) * K + V * K * K);
			a0 = (1.0f + SQRT2 * K + K * K) * norm;
			a1 = 2.0f * (K * K - 1.0f) * norm;
			a2 = (1.0f - SQRT2 * K + K * K) * norm;
			b1 = 2.0f * (V * K * K - 1.0f) * norm;
			b2 = (1.0f - sqrtf(2.0f * V) * K + V * K * K) * norm;
		}
		break;
	case BIQUAD_highShelf:
		if (peakGain >= 0.0f)
		{
			norm = 1.0f / (1.0f + SQRT2 * K + K * K);
			a0 = (V + sqrtf(2.0f * V) * K + K * K) * norm;
			a1 = 2.0f * (K * K - V) * norm;
			a2 = (V - sqrtf(2.0f * V) * K + K * K) * norm;
			b1 = 2.0f * (K * K - 1.0f) * norm;
			b2 = (1.0f - SQRT2 * K + K * K) * norm;
		}
		else
		{
			norm = 1.0f / (V + sqrtf(2.0f * V) * K + K * K);
			a0 = (1.0f + SQRT2 * K + K * K) * norm;
			a1 = 2.0f * (K * K - 1.0f) * norm;
			a2 = (1.0f - SQRT2 * K + K * K) * norm;
			b1 = 2.0f * (K * K - V) * norm;
			b2 = (V - sqrtf(2.0f * V) * K + K * K) * norm;
		}
		break;
	}

	//save coefficients
	outCoeffs[0] = a0;
	outCoeffs[1] = a1;
	outCoeffs[2] = a2;
	outCoeffs[3] = -b1;
	outCoeffs[4] = -b2;
}

static void arm_biquad_cascade_df2T_initNoClean_f32(arm_biquad_cascade_df2T_instance_f32 *S, uint8_t numStages, const float32_t *pCoeffs, float32_t *pState)
{
	if (NeedReinitAudioFiltersClean) //original func
	{
		arm_biquad_cascade_df2T_init_f32(S, numStages, pCoeffs, pState);
		return;
	}

	/* Assign filter stages */
	S->numStages = numStages;

	/* Assign coefficient pointer */
	S->pCoeffs = pCoeffs;

	/* Assign state pointer */
	S->pState = pState;
}

static void fill_biquad_coeffs(iir_filter_t *filter, float32_t *coeffs, uint8_t sect_num)
{
	//transpose and save coefficients
	uint16_t ind = 0;
	for(uint8_t sect = 0; sect < sect_num; sect++)
	{
		coeffs[ind + 0] = filter->b[sect * 3 + 0];
		coeffs[ind + 1] = filter->b[sect * 3 + 1];
		coeffs[ind + 2] = filter->b[sect * 3 + 2];
		coeffs[ind + 3] = -filter->a[sect * 3 + 1];
		coeffs[ind + 4] = -filter->a[sect * 3 + 2];
		ind += 5;
	}
}
