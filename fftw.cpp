#include <fftw3.h>

#include <cassert>

// FFTW plan initialization can have high overhead, so share and reuse it
fftw_plan fft_plan = nullptr;

void dft_init_fftw(size_t N_fft) {
    assert(N_fft > 0);
    fftw_complex *fft_in = fftw_alloc_complex(N_fft);
    assert(fft_in);
    fftw_complex *fft_out = fftw_alloc_complex(N_fft);
    assert(fft_out);
    fft_plan = fftw_plan_dft_1d(N_fft, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    assert(fft_plan);
    fftw_free(fft_in);
    fftw_free(fft_out);
}

void dft_destroy_fftw(void) {
    fftw_destroy_plan(fft_plan);
    fft_plan = nullptr;
}
