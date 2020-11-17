
#include "PlatformData.h"
#include "ImgPlane.h"
#include "ip.h"

#include "fftw.h"

#include <cnpy.h>

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

extern "C" {

#if 1
void load(
        double *d_u, double *d_v,
        float *n_hat_dat_buf, float *n_hat_dat, int n_hat_offset,
		int n_hat_n, int n_hat_s_n,
        float *R_c_dat_buf, float *R_c_dat, int R_c_offset,
		int R_c_n, int R_c_s_n) {
#else
void load(double *d_u, double *d_v) {
#endif

    // TODO: duplicated in main
    string platform_dir = "input_data_dir"; // see CMakeLists.txt

    auto start = high_resolution_clock::now();
    // TODO: remove filling of Halide buffers from this loading procedure
    PlatformData pd = platform_load(platform_dir);
    auto stop = high_resolution_clock::now();
    cout << "Loaded platform data in "
         << duration_cast<milliseconds>(stop - start).count() << " ms" << endl;
    cout << "Number of pulses: " << pd.npulses << endl;
    cout << "Pulse sample size: " << pd.nsamples << endl;

    // TODO: duplicated in main
    int nu;
    int nv;
    bool upsample = true; // TODO
    if (upsample) {
        nu = ip_upsample(pd.nsamples);
        nv = ip_upsample(pd.npulses);
    } else {
        nu = pd.nsamples;
        nv = pd.npulses;
    }

    *d_u = ip_du(pd.delta_r, RES_FACTOR, pd.nsamples, nu);
    *d_v = ip_dv(ASPECT, *d_u);
    cout << "ptr: d_u=" << d_u << "d_v=" << d_v << std::endl;
    cout << "set: d_u=" << *d_u << "d_v=" << *d_v << std::endl;

    const float *n_hat = pd.n_hat.has_value() ? pd.n_hat.value().begin() : &N_HAT[0];
    printf("n_hat_dat: %d, %d, %d \n", n_hat_offset, n_hat_n, n_hat_s_n);
    for (int i = 0; i < 3 /* TODO */; ++i) {
        n_hat_dat[i] = n_hat[i]; /* TODO: offest, strides etc */
    }

    cnpy::NpyArray npy_R_c = cnpy::npy_load(platform_dir + "/R_c.npy");
    if (npy_R_c.shape.size() != 1 || npy_R_c.shape[0] != 3) {
        throw runtime_error("Bad shape: R_c");
    }
    if (npy_R_c.word_size == sizeof(float)) {
        memcpy(R_c_dat, npy_R_c.data<float>(), npy_R_c.num_bytes());
    } else if (npy_R_c.word_size == sizeof(double)) {
        cout << "PlatformData: downcasting R_c from double to float" << endl;
        const double *src = npy_R_c.data<double>();
        float *dest = R_c_dat;
        for (size_t i = 0; i < npy_R_c.num_vals; i++) {
            dest[i] = (float)src[i];
        }
    } else {
        throw runtime_error("Bad word size: R_c");
    }

    cout << "X length: " << nu << endl;
    cout << "Y length: " << nv << endl;

    // Compute FFT width (power of 2)
    int N_fft = static_cast<int>(pow(2, static_cast<int>(log2(pd.nsamples * upsample)) + 1));

}

void init_fft(int N_fft) {
    // FFTW: init shared context
    dft_init_fftw(static_cast<size_t>(N_fft));

}

void destroy_fft() {
    // FFTW: clean up shared context
    dft_destroy_fftw();
}

#if 1
void load_test_ptr(double *d_u, double *d_v) {
    cout << "get: d_u=" << *d_u << "d_v=" << *d_v << std::endl;
}

void load_test(double d_u, double d_v) {
    cout << "by value: d_u=" << d_u << "d_v=" << d_v << std::endl;
}
#endif

} // extern
