#include <Halide.h>

#include "complexfunc.h"
#include "signal.h"
#include "util.h"

using namespace Halide;

class ImgOutputToDBGenerator : public Halide::Generator<ImgOutputToDBGenerator> {
public:
    Input<Buffer<double>> img {"img", 3}; // complex input
    Output<Buffer<double>> out{"out", 2};

    void generate() {
        Var x{"x"}, y{"y"}, c{"c"};

        Func img_func("cimg");
        img_func = img;
        ComplexFunc cimg(c, img_func);

        RDom r(0, img.dim(1).extent(), 0, img.dim(2).extent(), "r");
        Func m("m");
        m() = maximum(abs(cimg(r.x, r.y)));

        out(x, y) = Expr(10) * log10(abs(cimg(x, y)) / m());

        m.compute_root();


#if 1
        int NU = 512;
        int NV = 512;
        img.dim(0).set_stride(NU * NV);
        img.dim(1).set_stride(NV * 1);
        img.dim(2).set_stride(1);

        out.dim(0).set_stride(NV * 1);
        out.dim(1).set_stride(1);
#endif
    }
};

class ImgOutputU8Generator : public Halide::Generator<ImgOutputU8Generator> {
public:
    Input<Buffer<double>> dB {"dB", 2};
    Input<double> dB_min {"dB_min"};
    Input<double> dB_max {"dB_max"};
    Output<Buffer<uint8_t>> out{"out", 2};

    void generate() {
        Var x{"x"}, y{"y"};
        out(x, y) = ConciseCasts::u8(dB_scale(dB(x, y), dB_min, dB_max, UInt(8)));

        int NU = 512;
        int NV = 512;
        dB.dim(0).set_stride(NV * 1);
        dB.dim(1).set_stride(1);
        out.dim(0).set_stride(NV * 1);
        out.dim(1).set_stride(1);
    }
};

HALIDE_REGISTER_GENERATOR(ImgOutputToDBGenerator, img_output_to_dB)
HALIDE_REGISTER_GENERATOR(ImgOutputU8Generator, img_output_u8)
