/*******************************************************
 * Copyright (c) 2015, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#pragma once
#include <af/defines.h>
#include <Array.hpp>

namespace cpu
{
namespace kernel
{

template<af_op_t op, typename Ti, typename To, int D>
struct scan_dim
{
    void operator()(Array<To> out, dim_t outOffset,
                    const Array<Ti> in, dim_t inOffset,
                    const int dim) const
    {
        const dim4 odims    = out.dims();
        const dim4 ostrides = out.strides();
        const dim4 istrides = in.strides();

        const int D1 = D - 1;
        for (dim_t i = 0; i < odims[D1]; i++) {
            scan_dim<op, Ti, To, D1> func;
            getQueue().enqueue(func,
                    out, outOffset + i * ostrides[D1],
                    in, inOffset + i * istrides[D1], dim);
            if (D1 == dim) break;
        }
    }
};

template<af_op_t op, typename Ti, typename To>
struct scan_dim<op, Ti, To, 0>
{
    void operator()(Array<To> output, dim_t outOffset,
                    const Array<Ti> input,  dim_t inOffset,
                    const int dim) const
    {
        const Ti* in = input.get() + inOffset;
              To* out= output.get()+ outOffset;

        const dim4 ostrides = output.strides();
        const dim4 istrides = input.strides();
        const dim4 idims    = input.dims();

        dim_t istride = istrides[dim];
        dim_t ostride = ostrides[dim];

        Transform<Ti, To, op> transform;
        // FIXME: Change the name to something better
        Binary<To, op> scan;

        To out_val = scan.init();
        for (dim_t i = 0; i < idims[dim]; i++) {
            To in_val = transform(in[i * istride]);
            out_val = scan(in_val, out_val);
            out[i * ostride] = out_val;
        }
    }
};

}
}
