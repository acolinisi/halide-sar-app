#pragma once

#include <fftw3.h>
#include <unistd.h>

extern fftw_plan fft_plan;

void dft_init_fftw(size_t N_fft);
void dft_destroy_fftw(void);
