#ifndef PROFILER_h
#define PROFILER_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define PROFILES_COUNT 7 //количество профайлеров

typedef struct //структура профайлера
{
	uint32_t startTime;
	uint32_t endTime;
	uint32_t diff;
	bool started;
	uint32_t samples;
} PROFILE_INFO;

//Public methods
extern void InitProfiler(void); //инициализация профайлера
extern void StartProfiler(uint8_t pid); //запуск профайлера
extern void EndProfiler(uint8_t pid, bool summarize); //завершение профайлера
extern void PrintProfilerResult(void); //вывод результатов профайлера
extern void StartProfilerUs(void); //запуск профайлера в микросекундах
extern void EndProfilerUs(bool summarize); //завершение профайлера в микросекундах

#endif
