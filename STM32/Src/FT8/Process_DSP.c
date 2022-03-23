#include "lcd_driver.h"
#include "fft.h"
#include "lcd.h"

#include "Process_DSP.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "decode_ft8.h"
#include "FT8_main.h"

#include "traffic_manager.h"

#define M_PI 3.14159265358979323846264338

extern uint16_t cursor_line;

extern int num_decoded_msg;

int master_offset, offset_step;
int CQ_Flag;

#if (defined(LAY_800x480))
uint8_t *export_fft_power;
q15_t *window_dsp_buffer;
uint8_t *WF_index;
//float *window;
SRAM4 float window[FFT_SIZE_FT8];
q15_t *FFT_Scale;
q15_t *FFT_Magnitude;
int32_t *FFT_Mag_10;
uint8_t *FFT_Buffer;
//float *mag_db;
SRAM4 float mag_db[FFT_SIZE_FT8 / 2 + 1];
q15_t *dsp_buffer;
q15_t *dsp_output;
q15_t *input_gulp;
#else
IRAM2 uint8_t export_fft_power[ft8_msg_samples * ft8_buffer * 4];
SRAM q15_t window_dsp_buffer[FFT_SIZE_FT8];

SRAM uint8_t WF_index[WF_index_size];
SRAM float window[FFT_SIZE_FT8];

SRAM q15_t FFT_Scale[FFT_SIZE_FT8 * 2];
SRAM q15_t FFT_Magnitude[FFT_SIZE_FT8];
SRAM int32_t FFT_Mag_10[FFT_SIZE_FT8 / 2];
SRAM uint8_t FFT_Buffer[FFT_SIZE_FT8 / 2];
SRAM float mag_db[FFT_SIZE_FT8 / 2 + 1];

SRAM q15_t dsp_buffer[3 * input_gulp_size] __attribute__((aligned(4)));
SRAM q15_t dsp_output[FFT_SIZE_FT8 * 2] __attribute__((aligned(4)));
q15_t input_gulp[input_gulp_size] __attribute__((aligned(4)));
#endif

arm_rfft_instance_q15 fft_inst;
arm_cfft_radix4_instance_q15 aux_inst;

void init_DSP(void)
{
	#if (defined(LAY_800x480))
  // malloc from Wolf FFT
	uint32_t offset = 0;
	dma_memset(print_output_buffer, 0x00, sizeof(print_output_buffer));
	
  export_fft_power = (uint8_t *)print_output_buffer + offset;
	offset += (ft8_msg_samples * ft8_buffer * 4);
	
	window_dsp_buffer = (q15_t *)print_output_buffer + offset;
	offset += (FFT_SIZE_FT8 * sizeof(q15_t));
	
	WF_index = (uint8_t *)print_output_buffer + offset;
	offset += WF_index_size;
	
	//window = (float *)print_output_buffer + offset;
	offset += (FFT_SIZE_FT8 * sizeof(float));
	
	FFT_Scale = (q15_t *)print_output_buffer + offset;
	offset += (FFT_SIZE_FT8 * 2 * sizeof(q15_t));
	
	FFT_Magnitude = (q15_t *)print_output_buffer + offset;
	offset += (FFT_SIZE_FT8 * sizeof(q15_t));
	
	FFT_Mag_10 = (int32_t *)print_output_buffer + offset;
	offset += ((FFT_SIZE_FT8 / 2) * sizeof(int32_t));
		
	FFT_Buffer = (uint8_t *)print_output_buffer + offset;
	offset += FFT_SIZE_FT8 / 2;
	
	//mag_db = (float *)print_output_buffer + offset;
	offset += ((FFT_SIZE_FT8 / 2 + 1) * sizeof(float));
	
	dsp_buffer = (q15_t *)print_output_buffer + offset;
	offset += ((3 * input_gulp_size) * sizeof(q15_t));
	
	dsp_output = (q15_t *)print_output_buffer + offset;
	offset += ((FFT_SIZE_FT8 * 2) * sizeof(q15_t));
	
	input_gulp = (q15_t *)print_output_buffer + offset;
	offset += (input_gulp_size * sizeof(q15_t));
	
	println("FT8 buff size: ", sizeof(print_output_buffer), " need: ", offset);
	print_flush();
  #endif

  // arm_rfft_init_q15(&fft_inst, &aux_inst, FFT_SIZE_FT8, 0, 1);
  arm_rfft_init_q15(&fft_inst, FFT_SIZE_FT8, 0, 1);
  arm_cfft_radix4_init_q15(&aux_inst, FFT_SIZE_FT8, 0, 1);

  for (int i = 0; i < FFT_SIZE_FT8; ++i)
    window[i] = ft_blackman_i(i, FFT_SIZE_FT8);
  offset_step = (int)ft8_buffer * 4;
}

int max_bin, max_bin_number;

float ft_blackman_i(int i, int N)
{
  const float alpha = 0.16f; // or 2860/18608
  const float a0 = (1 - alpha) / 2;
  const float a1 = 1.0f / 2;
  const float a2 = alpha / 2;

  float x1 = cosf(2 * (float)M_PI * i / (N - 1));
  // float x2 = cosf(4 * (float)M_PI * i / (N - 1));
  float x2 = 2 * x1 * x1 - 1; // Use double angle formula
  return a0 - a1 * x1 + a2 * x2;
}

// Compute FFT magnitudes (log power) for each timeslot in the signal
void extract_power(int offset)
{

  // Loop over two possible time offsets (0 and block_size/2)
  for (int time_sub = 0; time_sub <= input_gulp_size / 2; time_sub += input_gulp_size / 2)
  {

    for (int i = 0; i < FFT_SIZE_FT8; i++)
      window_dsp_buffer[i] = (q15_t)((float)dsp_buffer[i + time_sub] * window[i]);

    arm_rfft_q15(&fft_inst, window_dsp_buffer, dsp_output);
    arm_shift_q15(&dsp_output[0], 5, &FFT_Scale[0], FFT_SIZE_FT8 * 2);
    arm_cmplx_mag_squared_q15(&FFT_Scale[0], &FFT_Magnitude[0], FFT_SIZE_FT8);

    for (int j = 0; j < FFT_SIZE_FT8 / 2; j++)
    {
      FFT_Mag_10[j] = 10 * (int32_t)FFT_Magnitude[j];
      mag_db[j] = 5.0 * log((float)FFT_Mag_10[j] + 0.1);
    }

    // Loop over two possible frequency bin offsets (for averaging)
    for (int freq_sub = 0; freq_sub < 2; ++freq_sub)
    {
      for (int j = 0; j < ft8_buffer; ++j)
      {
        float db1 = mag_db[j * 2 + freq_sub];
        float db2 = mag_db[j * 2 + freq_sub + 1];
        float db = (db1 + db2) / 2;

        int scaled = (int)(db);
        export_fft_power[offset] = (scaled < 0) ? 0 : ((scaled > 255) ? 255 : scaled);
        ++offset;
      }
    }
  }
}

void process_FT8_FFT(void)
{

  if (ft8_flag == 1)
  {

    master_offset = offset_step * FT_8_counter;
    extract_power(master_offset);

    update_offset_waterfall(master_offset);

    FT_8_counter++;
    if (FT_8_counter == ft8_msg_samples)
    {
      ft8_flag = 0;
      decode_flag = 1;
    }
  }
}

void update_offset_waterfall(int offset)
{

  for (int j = ft8_min_bin; j < ft8_buffer; j++)
    FFT_Buffer[j] = export_fft_power[j + offset];

  int bar;
  for (int x = ft8_min_bin; x < ft8_buffer; x++)
  {
    bar = FFT_Buffer[x];
    if (bar > 63)
      bar = 63;
    WF_index[x] = bar;
  }

  int tmp_K;
  for (int k = ft8_min_bin; k < ft8_buffer; k++)
  {
    LCDDriver_drawPixel(k - ft8_min_bin, WF_counter, WFPalette[WF_index[k]]);

    if ((k - ft8_min_bin == cursor_line - 1) || (k - ft8_min_bin == cursor_line) || (k - ft8_min_bin == cursor_line + 1))
      LCDDriver_drawPixel(k - ft8_min_bin, WF_counter, COLOR_RED);

    tmp_K = k;
    //		if (k - ft8_min_bin == cursor_line)
    //			LCDDriver_drawPixel(k - ft8_min_bin, WF_counter,COLOR_RED);
    // tft.drawPixel(k - ft8_min_bin, WF_counter,HX8357_RED);

    //    tft.drawPixel(k-ft8_min_bin, WF_counter, WFPalette[WF_index[k]]);
    //    if (k - ft8_min_bin == cursor_line) tft.drawPixel(k - ft8_min_bin, WF_counter,HX8357_RED);
  }
  //	LCDDriver_drawPixel(tmp_K + 1, WF_counter,COLOR_RED);			//Indicate the end of the  FFT field

  /*
        if (num_decoded_msg > 0 &&  WF_counter == 0) {
        display_messages(num_decoded_msg);

  //       if (CQ_Flag == 1)
          service_CQ();
        else
          Check_Calling_Stations(num_decoded_msg);
  //
        num_decoded_msg = 0;
        }
  */

  WF_counter++;
}

void Service_FT8(void)
{
  if (CQ_Flag == 1)
    service_CQ();
  else if (num_decoded_msg > 0)
    Check_Calling_Stations(num_decoded_msg);

  if (num_decoded_msg > 0)
  {
    display_messages(num_decoded_msg);

    //		num_decoded_msg = 0;
  }
}

void Set_Data_Colection(bool enable)
{
  if (enable)
  {
    ft8_flag = 1;
    FT_8_counter = 0;
    ft8_marker = 1;
    WF_counter = 0;

    FT8_DatBlockNum = 0;      // reset the data buffer
    FT8_ColectDataFlg = true; // Enable the Data colection
  }
  else
  {
    ft8_flag = 0;
    FT_8_counter = 0;
    ft8_marker = 0;
    WF_counter = 0;

    FT8_DatBlockNum = 0;       // Reset the data buffer
    FT8_ColectDataFlg = false; // Disable the Data colection
  }
}
