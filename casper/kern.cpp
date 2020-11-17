
#include "PlatformData.h"
#include "ImgPlane.h"
#include "ip.h"

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

extern "C" {

void load(double *M_buf, double *M, int offset,
		int n, int m, int s_n, int s_m,
        double *d_u, double *d_v) {

    // TODO: duplicated in main
    string platform_dir = "input_data_dir"; // see CMakeLists.txt

    auto start = high_resolution_clock::now();
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
}

} // extern
