/* -*- c++ -*- */
/* 
 * Copyright 2013 Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT)
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
#include "pcfich_unpack_vfm_impl.h"

#include <fftw3.h>
#include <volk/volk.h>
#include <boost/format.hpp>

#include <cstdio>

namespace gr {
  namespace lte {

    pcfich_unpack_vfm::sptr
    pcfich_unpack_vfm::make(std::string key, std::string msg_buf_name, std::string name)
    {
      return gnuradio::get_initial_sptr
        (new pcfich_unpack_vfm_impl(key, msg_buf_name, name));
    }

    /*
     * The private constructor
     */
    pcfich_unpack_vfm_impl::pcfich_unpack_vfm_impl(std::string key, std::string msg_buf_name, std::string& name)
      : gr::sync_block(name,
              gr::io_signature::make( 1, 1, sizeof(float) * 32),
              gr::io_signature::make(0, 0, 0)),
              d_subframe(0)
    {
        d_key = pmt::string_to_symbol(key);
        d_port_cfi = pmt::string_to_symbol(msg_buf_name);

        message_port_register_out(d_port_cfi);

        d_in_seq = (float*)fftwf_malloc(sizeof(float) * 32);
        initialize_ref_seqs();
    }

    /*
     * Our virtual destructor.
     */
    pcfich_unpack_vfm_impl::~pcfich_unpack_vfm_impl()
    {
    }

    int
    pcfich_unpack_vfm_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];

        std::vector <gr::tag_t> v_b;
        get_tags_in_range(v_b, 0, nitems_read(0), nitems_read(0)+noutput_items, d_key);
        if(v_b.size() > 0){
            long offset = v_b[0].offset;
            int value = int(pmt::to_long(v_b[0].value) );
            d_subframe = value;

            //printf("%ld\tval = %i\n", offset, value );
        }
        memcpy(d_in_seq, in, sizeof(float)*32);
        cfi_result cfi = calculate_cfi(d_in_seq);
        //printf("cfi = %i\tsubframe = %i\n", cfi, d_subframe );
        publish_cfi(d_subframe, cfi);

        // Tell runtime system how many output items we produced.
        return 1;
    }
    
    pcfich_unpack_vfm_impl::cfi_result
    pcfich_unpack_vfm_impl::calculate_cfi(float* in_seq)
    {
        int cfi = 0;
        float max_val = 0.0f;
        float next_val = 0.0f;
        //~ printf("find cfi ");
        for(int i = 0; i < 3; i++){
            next_val = correlate(in_seq, d_ref_seqs[i], 32);
            //~ printf("%1.2f\t", next_val);
            if(next_val > max_val){
                cfi = i+1;
                max_val = next_val;
            }
        }
        //~ printf("\n");
        cfi_result res;
        res.cfi = cfi;
        res.val = max_val;
        return res;
    }
    
    
    float
    pcfich_unpack_vfm_impl::correlate(float* in0, float* in1, int len)
    {
        float* mult = (float*)fftwf_malloc(sizeof(float) * 32);
        volk_32f_x2_multiply_32f_u(mult, in0, in1, len);
        float res = 0.0f;
        volk_32f_accumulator_s32f_a(&res, mult, len);
        return res;
    }

    void
    pcfich_unpack_vfm_impl::initialize_ref_seqs()
    {
        int cfi1[] = {0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1};
        int cfi2[] = {1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0};
        int cfi3[] = {1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1};
        std::vector<int*> cfi;
        cfi.push_back(cfi1);
        cfi.push_back(cfi2);
        cfi.push_back(cfi3);

        for(int i = 0; i < 3; i++){
            d_ref_seqs.push_back( (float*)fftwf_malloc(sizeof(float) * 32) );
            for(int c = 0; c < 32; c++){
                float val  = float(1.0-2.0*float(cfi[i][c]));
                d_ref_seqs[i][c] = val;
            }
        }
    }

    void
    pcfich_unpack_vfm_impl::publish_cfi(int subframe, cfi_result cfi)
    {
        pmt::pmt_t msg_cfi = pmt::from_long(long(cfi.cfi) );
        pmt::pmt_t msg_subframe = pmt::from_long(long(subframe) );
        pmt::pmt_t msg = pmt::cons(msg_subframe, msg_cfi );

        GR_LOG_INFO(d_logger, boost::format("%s\tsubframe = %i\tCFI = %i\t(correlation value = %f)") % name().c_str() % subframe % cfi.cfi % cfi.val);
        if(d_dbg){
        	d_cfi_results.push_back(cfi.cfi);
        }
        message_port_pub( d_port_cfi, msg );
    }

  } /* namespace lte */
} /* namespace gr */

