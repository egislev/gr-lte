/* -*- c++ -*- */
/*
 * Copyright 2014 Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT)
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <lte/pss.h>
#include <volk/volk.h>


namespace gr
{
namespace lte
{

pss::pss()
{
}

pss::~pss()
{
}

const gr_complex
pss::d_C_I = gr_complex(0,1);

// Define PI for use in this block
const float
pss::d_PI = float(M_PI);

//generate pss in time domain with fftw
void
pss::gen_pss_t(gr_complex *zc_t, int cell_id, int len)
{
    gr_complex zc_f[62];
    zc(zc_f, cell_id);

    gr_complex* d_in  = (gr_complex*) fftwf_malloc(sizeof(gr_complex)*len);
    gr_complex* d_out = (gr_complex*) fftwf_malloc(sizeof(gr_complex)*len);

    memset(d_in, 0, sizeof(gr_complex)*len);
    memcpy(d_in+len-31, zc_f, sizeof(gr_complex)*31);
    memcpy(d_in+1, zc_f+31, sizeof(gr_complex)*31);

    fftwf_plan p = fftwf_plan_dft_1d(len, reinterpret_cast<fftwf_complex*>(d_in), reinterpret_cast<fftwf_complex*>(d_out), FFTW_BACKWARD, FFTW_ESTIMATE);

    fftwf_execute(p);

    memcpy(zc_t, d_out, sizeof(gr_complex)*len);

    fftwf_destroy_plan(p);
    fftwf_free(d_in);
    fftwf_free(d_out);
}

//generate cunjungate pss in time domain
void
pss::gen_conj_pss_t(gr_complex *zc_t, int cell_id, int len)
{
    gen_pss_t(zc_t, cell_id, len);
    volk_32fc_conjugate_32fc(zc_t, zc_t, len);
}


void
pss::zc(gr_complex *zc, int cell_id)
{
    // calculate value of variable u according to cell id number
    float u=0;
    switch (cell_id)
    {
    case 0:
        u=25.0;
        break;
    case 1:
        u=29.0;
        break;
    case 2:
        u=34.0;
        break;
    }

    //generate zadoff-chu sequences
    for(int n = 0; n < 31; n++)
    {
        zc[n]=exp(d_C_I* gr_complex(d_PI*u* float(-1*n*(n+1))/63.0 ) );
    }
    for(int n = 31; n < 62; n++)
    {
        zc[n]=exp(d_C_I* gr_complex(d_PI*u* float(-1*(n+1)*(n+2))/63.0 ) );
    }
}





} /* namespace lte */
} /* namespace gr */

