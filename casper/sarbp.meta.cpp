#include "ip.h"

#include "casper.h"

#include <cnpy.h>

#include <vector>

using namespace cac;

namespace {

} // namespace anon

int main(int argc, char **argv) {
	Options opts; // metaprogram can add custom options
	opts.parseOrExit(argc, argv);

        std::string platform_dir = "input_data_dir"; // see CMakeLists.txt
        int upsample = 2;
        int nu, nv;

	TaskGraph tg("sarbp"); // must match target name in CMake script
        
        cnpy::NpyArray npy_nsamples = cnpy::npy_load(platform_dir + "/nsamples.npy");
        int nsamples;
        if (npy_nsamples.word_size == sizeof(int)) {
            nsamples = *npy_nsamples.data<int>();
        } else if (npy_nsamples.word_size == sizeof(int64_t)) {
            // no need to warn of downcasting
            nsamples = (int)*npy_nsamples.data<int64_t>();
        } else {
            throw std::runtime_error("Bad word size: nsamples");
        }

        cnpy::NpyArray npy_npulses = cnpy::npy_load(platform_dir + "/npulses.npy");
        int npulses;
        if (npy_npulses.word_size == sizeof(int)) {
            npulses = *npy_npulses.data<int>();
        } else if (npy_npulses.word_size == sizeof(int64_t)) {
            // no need to warn of downcasting
            npulses = (int)*npy_npulses.data<int64_t>();
        } else {
            throw std::runtime_error("Bad word size: npulses");
        }

        cnpy::NpyArray npy_delta_r = cnpy::npy_load(platform_dir + "/delta_r.npy");
        if (npy_delta_r.word_size != sizeof(double)) {
            throw std::runtime_error("Bad word size: delta_r");
        }
        double delta_r_p = *npy_delta_r.data<double>();

        if (upsample) {
            nu = ip_upsample(nsamples);
            nv = ip_upsample(npulses);
        } else {
            nu = nsamples;
            nv = npulses;
        }

        // TODO: is there a way to get sizes without loading the data?

	Dat *phs = &tg.createFloatDat(3, {2, nsamples, npulses}); // float, d=3
	Dat *k_r = &tg.createDat(1, nsamples); // float

	IntScalar *taylor = &tg.createIntScalar(64, 17);

        // Compute FFT width (power of 2)
        int N_fft_val = static_cast<int>(pow(2,
                    static_cast<int>(log2(nsamples * upsample)) + 1));
	IntScalar *N_fft = &tg.createIntScalar(64, N_fft_val);

	DoubleScalar *delta_r = &tg.createDoubleScalar(delta_r_p);

	Dat *u = &tg.createDoubleDat(1, {nu});
	Dat *v = &tg.createDoubleDat(1, {nv});
	Dat *pos = &tg.createFloatDat(2, {3, npulses}); // float, dim 2
	Dat *r = &tg.createDoubleDat(2, {nu*nv, 3}); // double, dim 2

	//DoubleScalar *res_factor = &tg.createDoubleScalar(RES_FACTOR);
        // TODO: constructor without a value
	DoubleScalar *d_u = &tg.createDoubleScalar(0);
	DoubleScalar *d_v = &tg.createDoubleScalar(0);

        Dat *dummyDat = &tg.createDoubleDat(1, {2});
        Task& task_load = tg.createTask(CKernel("load"), {dummyDat,
                d_u, d_v});

        //Task& task_ip = tg.createTask(HalideKernel("ip_"),
        //        {});

	Task& task_bp = tg.createTask(HalideKernel("backprojection"),
                        {phs, k_r, taylor, N_fft, delta_r, u, v, pos, r},
			{});

	return tryCompile(tg, opts);
}
