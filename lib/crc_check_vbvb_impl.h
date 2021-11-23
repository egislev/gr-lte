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

#ifndef INCLUDED_LTE_CRC_CHECK_VBVB_IMPL_H
#define INCLUDED_LTE_CRC_CHECK_VBVB_IMPL_H

#include <lte/crc_check_vbvb.h>
#include <boost/crc.hpp>

namespace gr {
  namespace lte {

    class crc_check_vbvb_impl : public crc_check_vbvb
    {
     private:
      int d_exp[16];
      const int d_data_len;
      const int d_final_xor;
      // length checksum, CRC ploynomial, initial state, final XOR, REFLECT Input, REFLECT_REM
	  boost::crc_optimal<16, 0x1021, 0x0000, 0x0000, false, false> lte_crc_16;
	  // final XOR may be different, but due to template restrictions this is processed outside of this function.
		unsigned char pack_byte(const char* unc);
     
     public:
      crc_check_vbvb_impl(std::string& name, int data_len, int final_xor);
      ~crc_check_vbvb_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace lte
} // namespace gr

#endif /* INCLUDED_LTE_CRC_CHECK_VBVB_IMPL_H */

