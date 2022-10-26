#include "snap.h"
#include "fft.h"
#include "trx_manager.h"
#include "lcd.h"

static arm_sort_instance_f32 SNAP_SortInstance;
SRAM_ON_F407 static float32_t SNAP_buffer[FFT_SIZE] = {0};
static float32_t SNAP_buffer_tmp[FFT_SIZE] = {0};
static bool SNAP_need_buffer = false;
static bool SNAP_process_from_auto = false;
static uint8_t SNAP_buffer_avg_index = 0;

static void SNAP_Process();

void SNAP_FillBuffer(float32_t *buff)
{
	if(!SNAP_need_buffer)
		return;
	
	if(CurrentVFO->Mode != TRX_MODE_CW && CurrentVFO->Mode != TRX_MODE_NFM) {
		SNAP_buffer_avg_index = 0;
		SNAP_need_buffer = false;
		return;
	}
	
	if(SNAP_buffer_avg_index == 0) {
		dma_memset(SNAP_buffer, 0x00, sizeof(SNAP_buffer));
	}
	
	arm_add_f32(buff, SNAP_buffer, SNAP_buffer, FFT_SIZE);
	SNAP_buffer_avg_index++;
	
	if(SNAP_buffer_avg_index >= SNAP_AVERAGING) {
		arm_scale_f32(SNAP_buffer, 1.0f / (float32_t)SNAP_AVERAGING, SNAP_buffer, FFT_SIZE);
		SNAP_need_buffer = false;
		SNAP_Process();
	}
}

void SNAP_DoSnap(bool do_auto)
{
	if(do_auto && TRX_Inactive_Time < SNAP_AUTO_TIMEOUT)
		return;
	
	SNAP_process_from_auto = do_auto;
	SNAP_buffer_avg_index = 0;
	SNAP_need_buffer = true;
}

static void SNAP_Process()
{
	arm_sort_init_f32(&SNAP_SortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
	arm_sort_f32(&SNAP_SortInstance, SNAP_buffer, SNAP_buffer_tmp, FFT_SIZE);
	
	float32_t noise_level = SNAP_buffer_tmp[(uint32_t)(SNAP_NOISE_FLOOR * (float32_t)FFT_SIZE)];
	float32_t hz_in_bin = (float32_t)FFT_current_spectrum_width_hz / (float32_t)FFT_SIZE;
	
	uint32_t bins_in_bandwidth = (float32_t)CurrentVFO->LPF_RX_Filter_Width / hz_in_bin;
	uint32_t bandwidth_bin_start = (FFT_SIZE / 2) - (bins_in_bandwidth / 2);
	uint32_t bandwidth_bin_end = (FFT_SIZE / 2) + (bins_in_bandwidth / 2);
	uint32_t bandwidth_bin_count = bandwidth_bin_end - bandwidth_bin_start;
	
	uint64_t fft_freq_start = (float64_t)CurrentVFO->Freq - (float64_t)FFT_current_spectrum_width_hz / 2.0f;
	float32_t maxAmplValue;
	uint32_t maxAmplIndex;
	
	//search in BW
	arm_max_f32(&SNAP_buffer[bandwidth_bin_start], bandwidth_bin_count, &maxAmplValue, &maxAmplIndex);
	float32_t signal_snr = rate2dbP(maxAmplValue / noise_level);
	uint64_t target_freq = fft_freq_start + ((bandwidth_bin_start + maxAmplIndex) * hz_in_bin);
	
	if(signal_snr < SNAP_BW_SNR_THRESHOLD && !SNAP_process_from_auto) {
		//search in all FFT (nearest)
		for(int32_t allfft_bin_start = bandwidth_bin_start; allfft_bin_start > 0; allfft_bin_start -= bins_in_bandwidth / 2) {
			uint32_t allfft_bin_end = bandwidth_bin_end + (bandwidth_bin_start - allfft_bin_start);
			uint32_t allfft_bin_count = allfft_bin_end - allfft_bin_start;
			
			arm_max_f32(&SNAP_buffer[allfft_bin_start], allfft_bin_count, &maxAmplValue, &maxAmplIndex);
			signal_snr = rate2dbP(maxAmplValue / noise_level);
			target_freq = fft_freq_start + ((allfft_bin_start + maxAmplIndex) * hz_in_bin);
			
			if (signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
				break;
			}
		}
	}
	
	bool result_ok = false;
	if(!SNAP_process_from_auto && signal_snr >= SNAP_BW_SNR_THRESHOLD) {
		result_ok = true;
	}
	if(SNAP_process_from_auto && signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
		result_ok = true;
	}
	
	if(result_ok) {
		TRX_setFrequencySlowly(target_freq);
		LCD_UpdateQuery.FreqInfo = true;
	}
	
	println("NOISE: ", noise_level, " MAX: ", maxAmplValue, " SNR: ", signal_snr); 
	println("INDEX: ", maxAmplIndex, " HZ/BIN: ", hz_in_bin, " TARGET: ", target_freq, " OK: ", (uint8_t)result_ok);
	println("");
}
