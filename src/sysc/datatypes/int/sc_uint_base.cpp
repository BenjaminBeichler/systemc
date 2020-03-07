/*****************************************************************************
  
  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sc_uint_base.cpp -- contains interface definitions between sc_uint and
                 sc_signed, sc_unsigned, and definitions for sc_uint_subref.

  Original Author: Ali Dasdan, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


// $Log: sc_uint_base.cpp,v $
// Revision 1.5  2011/02/18 20:19:15  acg
//  Andy Goodrich: updating Copyright notice.
//
// Revision 1.4  2010/02/04 22:23:29  acg
//  Andy Goodrich: fixed bug in concatenation reads for part selections,
//  the mask being used was 32 bits and should have been 64 bits.
//
// Revision 1.3  2008/06/19 17:47:57  acg
//  Andy Goodrich: fixes for bugs. See 2.2.1 RELEASENOTES.
//
// Revision 1.2  2007/11/04 21:27:00  acg
//  Andy Goodrich: changes to make sure the proper value is returned from
//  concat_get_data().
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:49:32  acg
// Added $Log command so that CVS check in comments are reproduced in the
// source.
//

#include "sysc/kernel/sc_macros.h"
#include "sysc/datatypes/int/sc_signed.h"
#include "sysc/datatypes/int/sc_unsigned.h"
#include "sysc/datatypes/int/sc_int_base.h"
#include "sysc/datatypes/int/sc_uint_base.h"
#include "sysc/datatypes/int/sc_int_ids.h"
#include "sysc/datatypes/bit/sc_bv_base.h"
#include "sysc/datatypes/bit/sc_lv_base.h"
#include "sysc/datatypes/int/sc_vector_utils.h"
#include "sysc/datatypes/int/sc_signed_inlines.h"
#include "sysc/datatypes/int/sc_signed_ops.h"
#include "sysc/datatypes/int/sc_unsigned_inlines.h"
#include "sysc/datatypes/int/sc_int_ids.h"
#include "sysc/datatypes/misc/sc_concatref.h"
#include "sysc/datatypes/fx/sc_ufix.h"
#include "sysc/datatypes/fx/scfx_other_defs.h"

#include <sstream>

// explicit template instantiations
namespace sc_core {
template class SC_API sc_vpool<sc_dt::sc_uint_bitref>;
template class SC_API sc_vpool<sc_dt::sc_uint_subref>;
} // namespace sc_core

namespace sc_dt {

// to avoid code bloat in sc_uint_concat<T1,T2>

void
sc_uint_concref_invalid_length( int length )
{
    std::stringstream msg;
    msg << "sc_uint_concref<T1,T2> initialization: length = " << length
        << "violates 1 <= length <= " << SC_INTWIDTH;
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg.str().c_str() );
    sc_core::sc_abort(); // can't recover from here
}



// ----------------------------------------------------------------------------
//  CLASS : sc_uint_bitref
//
//  Proxy class for sc_uint bit selection (r-value and l-value).
// ----------------------------------------------------------------------------

sc_core::sc_vpool<sc_uint_bitref> sc_uint_bitref::m_pool(9);

// concatenation methods:

// #### OPTIMIZE
void sc_uint_bitref::concat_set(int64 src, int low_i)
{
    sc_uint_base aa( 1 );
    *this = aa = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_uint_bitref::concat_set(const sc_signed& src, int low_i)
{
    sc_uint_base aa( 1 );
    if ( low_i < src.length() )
        *this = aa = 1 & (src >> low_i);
    else
        *this = aa = (src < 0) ? (int_type)-1 : 0;
}

void sc_uint_bitref::concat_set(const sc_unsigned& src, int low_i)
{
    sc_uint_base aa( 1 );
    if ( low_i < src.length() )
        *this = aa = 1 & (src >> low_i);
    else
        *this = aa = 0;
}

void sc_uint_bitref::concat_set(uint64 src, int low_i)
{
    sc_uint_base aa( 1 );
    *this = aa = (low_i < 64) ? src >> low_i : 0;
}


// other methods

void
sc_uint_bitref::scan( ::std::istream& is )
{
    bool b;
    is >> b;
    *this = b;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_uint_subref_r
//
//  Proxy class for sc_uint part selection (l-value).
// ----------------------------------------------------------------------------

bool sc_uint_subref_r::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{
    int       dst_i;	   // Word in dst_p now processing.
    int       end_i;	   // Highest order word in dst_p to process.
    int       left_shift;  // Left shift for val.
    uint_type mask;	   // Mask for bits to extract or keep.

    dst_i = SC_DIGIT_INDEX(low_i);
    left_shift = SC_BIT_INDEX(low_i);
    end_i = SC_DIGIT_INDEX((low_i + (m_left-m_right)));

    mask = ~(~UINT_ZERO << left_shift);
    dst_p[dst_i] = (sc_digit)((dst_p[dst_i] & mask));

    dst_i++;
    for ( ; dst_i <= end_i; dst_i++ ) dst_p[dst_i] = 0;

    return false;
}

bool sc_uint_subref_r::concat_get_data( sc_digit* dst_p, int low_i ) const
{
    int       dst_i;	   // Word in dst_p now processing.
    int       end_i;	   // Highest order word in dst_p to process.
    int       high_i;	   // Index of high order bit in dst_p to set.
    int       left_shift;  // Left shift for val.
    uint_type mask;	   // Mask for bits to extract or keep.
    bool      result;	   // True if inserting non-zero value.
    uint_type val;	   // Selection value extracted from m_obj_p.

    dst_i = SC_DIGIT_INDEX(low_i);
    left_shift = SC_BIT_INDEX(low_i);
    high_i = low_i + (m_left-m_right); 
    end_i = SC_DIGIT_INDEX(high_i);
    mask = ~mask_int[m_left][m_right];
    val = (m_obj_p->m_val & mask) >> m_right;
    result = val != 0;


    // PROCESS THE FIRST WORD:

    mask = ~(~UINT_ZERO << left_shift);
    dst_p[dst_i] = (sc_digit)(((dst_p[dst_i] & mask)) |
	SC_MASK_DIGIT( val << left_shift ));

    switch ( end_i - dst_i )
    {
     // BITS ARE ACROSS TWO WORDS:

     case 1:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i] = (sc_digit)val;
        break;

     // BITS ARE ACROSS THREE WORDS:

     case 2:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i] = (sc_digit)val;
        break;

     // BITS ARE ACROSS THREE WORDS:

     case 3:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i] = (sc_digit)val;
        break;
    }
    return result;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_uint_subref
//
//  Proxy class for sc_uint part selection (r-value and l-value).
// ----------------------------------------------------------------------------

sc_core::sc_vpool<sc_uint_subref> sc_uint_subref::m_pool(9);

// assignment operators

sc_uint_subref&
sc_uint_subref::operator = ( uint_type v )
{
    uint_type val = m_obj_p->m_val;
    uint_type mask = mask_int[m_left][m_right];
    val &= mask;
    val |= (v << m_right) & ~mask;
    m_obj_p->m_val = val;
    m_obj_p->extend_sign();
    return *this;
}

sc_uint_subref&
sc_uint_subref::operator = ( const sc_signed& a )
{
    sc_uint_base aa( length() );
    return ( *this = aa = a );
}

sc_uint_subref&
sc_uint_subref::operator = ( const sc_unsigned& a )
{
    sc_uint_base aa( length() );
    return ( *this = aa = a );
}

sc_uint_subref&
sc_uint_subref::operator = ( const sc_bv_base& a )
{
    sc_uint_base aa( length() );
    return ( *this = aa = a );
}

sc_uint_subref&
sc_uint_subref::operator = ( const sc_lv_base& a )
{
    sc_uint_base aa( length() );
    return ( *this = aa = a );
}

// concatenation methods:

// #### OPTIMIZE
void sc_uint_subref::concat_set(int64 src, int low_i)
{
    sc_uint_base aa( length() );
    *this = aa = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_uint_subref::concat_set(const sc_signed& src, int low_i)
{
    sc_uint_base aa( length() );
    if ( low_i < src.length() )
        *this = aa = src >> low_i;
    else
        *this = aa = (src < 0) ? (int_type)-1 : 0;
}

void sc_uint_subref::concat_set(const sc_unsigned& src, int low_i)
{
    sc_uint_base aa( length() );
    if ( low_i < src.length() )
        *this = aa = src >> low_i;
    else
        *this = aa = 0;
}

void sc_uint_subref::concat_set(uint64 src, int low_i)
{
    sc_uint_base aa( length() );
    *this = aa = (low_i < 64) ? src >> low_i : 0;
}

// other methods

void
sc_uint_subref::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}


// ----------------------------------------------------------------------------
//  CLASS : sc_uint_base
//
//  Base class for sc_uint.
// ----------------------------------------------------------------------------

// support methods

void
sc_uint_base::invalid_length() const
{
    std::stringstream msg;
    msg << "sc_uint[_base] initialization: length = " << m_len
        << " violates 1 <= length <= " << SC_INTWIDTH;
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg.str().c_str() );
    sc_core::sc_abort(); // can't recover from here}
}

void
sc_uint_base::invalid_index( int i ) const
{
    std::stringstream msg;
    msg << "sc_uint[_base] bit selection: index = " << i
        << " violates 0 <= index <= " << (m_len - 1);
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg.str().c_str() );
    sc_core::sc_abort(); // can't recover from here
}

void
sc_uint_base::invalid_range( int l, int r ) const
{
    std::stringstream msg;
    msg << "sc_uint[_base] part selection: "
        << "left = " << l << ", right = " << r << " violates "
        << (m_len-1) << " >= left >= right >= 0";
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg.str().c_str() );
    sc_core::sc_abort(); // can't recover from here
}


void
sc_uint_base::check_value() const
{
    uint_type limit = (~UINT_ZERO >> m_ulen);
    if( m_val > limit ) {
        std::stringstream msg;
        msg << "sc_uint[_base]: value does not fit into a length of " << m_len;
        SC_REPORT_WARNING( sc_core::SC_ID_OUT_OF_BOUNDS_, msg.str().c_str() );
    }
}


// constructors

sc_uint_base::sc_uint_base( const sc_bv_base& v ) 
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v;
}
sc_uint_base::sc_uint_base( const sc_lv_base& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v;
}
sc_uint_base::sc_uint_base( const sc_int_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}
sc_uint_base::sc_uint_base( const sc_signed_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}
sc_uint_base::sc_uint_base( const sc_unsigned_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}

sc_uint_base::sc_uint_base( const sc_signed& a )
    : m_val( 0 ), m_len( a.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = a.to_uint64();
}

sc_uint_base::sc_uint_base( const sc_unsigned& a )
    : m_val( 0 ), m_len( a.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = a.to_uint64();
}

// assignment operators

sc_uint_base&
sc_uint_base::operator = ( const sc_signed& a )
{
    m_val = a.to_int64();
    extend_sign();
    return *this;
}

sc_uint_base& 
sc_uint_base::operator = ( const sc_unsigned& a )
{
    m_val = a.to_uint64();
    extend_sign();
    return *this;
}


sc_uint_base&
sc_uint_base::operator = ( const sc_bv_base& a )
{
    m_val = a.to_uint64();
    extend_sign();
    return *this;
}

sc_uint_base&
sc_uint_base::operator = ( const sc_lv_base& a )
{
#if 0
    int minlen = sc_min( m_len, a.length() );
    int i = 0;
    for( ; i < minlen; ++ i ) {
	set( i, sc_logic( a.get_bit( i ) ).to_bool() );
    }
    for( ; i < m_len; ++ i ) {
	// zero extension
	set( i, 0 );
    }
#endif
    m_val = a.to_uint64();
    extend_sign();
    return *this;
}

sc_uint_base&
sc_uint_base::operator = ( const char* a )
{
    if( a == 0 ) {
	SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_,
			 "character string is zero" );
    }
    else if( *a == 0 ) {
	SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_,
			 "character string is empty" );
    }
    else try {
	int len = m_len;
	sc_ufix aa( a, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
	return this->operator = ( aa );
    } catch( const sc_core::sc_report & ) {
        std::stringstream msg;
        msg << "character string '" << a << "' is not valid";
        SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_, msg.str().c_str() );

    }
    return *this;
}


// explicit conversion to character string

const std::string
sc_uint_base::to_string( sc_numrep numrep ) const
{
    int len = m_len;
    sc_ufix aa( *this, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
    return aa.to_string( numrep );
}

const std::string
sc_uint_base::to_string( sc_numrep numrep, bool w_prefix ) const
{
    int len = m_len;
    sc_ufix aa( *this, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
    return aa.to_string( numrep, w_prefix );
}


// reduce methods

bool
sc_uint_base::and_reduce() const
{
    return ( m_val == (~UINT_ZERO >> m_ulen) );
}

bool
sc_uint_base::or_reduce() const
{
    return ( m_val != uint_type( 0 ) );
}

bool
sc_uint_base::xor_reduce() const
{
    uint_type mask = ~UINT_ZERO;
    uint_type val = m_val;
    int n = SC_INTWIDTH;
    do {
	n >>= 1;
	mask >>= n;
	val = ((val & (mask << n)) >> n) ^ (val & mask);
    } while( n != 1 );
    return ( val != uint_type( 0 ) );
}


bool sc_uint_base::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{    
    int       dst_i;       // Word in dst_p now processing.
    int       end_i;       // Highest order word in dst_p to process.
    int       left_shift;  // Left shift for val.
    uint_type mask;        // Mask for bits to extract or keep.

    dst_i = SC_DIGIT_INDEX(low_i);
    left_shift = SC_BIT_INDEX(low_i);
    end_i = SC_DIGIT_INDEX((low_i + (m_len-1)));

    // PROCESS THE FIRST WORD:

    mask = ~(~UINT_ZERO << left_shift);
    dst_p[dst_i] = (sc_digit)((dst_p[dst_i] & mask));

    dst_i++;
    for ( ; dst_i <= end_i; dst_i++ ) dst_p[dst_i] = 0;
    return false;
}

//------------------------------------------------------------------------------
//"sc_uint_base::concat_get_data"
//
// This method transfers the value of this object instance to the supplied
// array of sc_unsigned digits starting with the bit specified by low_i within
// the array of digits.
//
// Notes:
//   (1) we don't worry about masking the high order data we transfer since
//       concat_get_data() is called from low order bit to high order bit. So
//       the bits above where we place ours will be filled in by someone else.
//
//   dst_p -> array of sc_unsigned digits to be filled in.
//   low_i =  first bit within dst_p to be set.
//------------------------------------------------------------------------------
bool sc_uint_base::concat_get_data( sc_digit* dst_p, int low_i ) const
{    
    int       dst_i;       // Word in dst_p now processing.
    int       end_i;       // Highest order word in dst_p to process.
    int       high_i;      // Index of high order bit in dst_p to set.
    int       left_shift;  // Left shift for val.
    uint_type mask;        // Mask for bits to extract or keep.
    bool      result;	   // True if inserting non-zero value.
    uint_type val;         // Value for this object.

    dst_i = SC_DIGIT_INDEX(low_i);
    left_shift = SC_BIT_INDEX(low_i);
    high_i = low_i + (m_len-1);
    end_i = SC_DIGIT_INDEX(high_i);
    val = m_val;
    result = val != 0;

    // MASK OFF DATA TO BE TRANSFERRED BASE ON WIDTH:

    if ( m_len < 64 )
    {
	mask = ~(~UINT_ZERO << m_len);
        val &=  mask;
    }

    // PROCESS THE FIRST WORD:

    mask = ~(~UINT_ZERO << left_shift);
    dst_p[dst_i] = (sc_digit)(((dst_p[dst_i] & mask)) | 
	SC_MASK_DIGIT(val << left_shift));

    switch ( end_i - dst_i )
    {
     // BITS ARE ACROSS TWO WORDS:

     case 1:
	dst_i++;
	val >>= (BITS_PER_DIGIT-left_shift);
	dst_p[dst_i] = (sc_digit)val;
	break;

     // BITS ARE ACROSS THREE WORDS:

     case 2:
	dst_i++;
	val >>= (BITS_PER_DIGIT-left_shift);
	dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
	val >>= BITS_PER_DIGIT;
	dst_p[dst_i] = (sc_digit)val;
	break;

     // BITS ARE ACROSS FOUR WORDS:

     case 3:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i++] = (sc_digit)SC_MASK_DIGIT(val);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i] = (sc_digit)val;
        break;

    }
    return result;
}

// #### OPTIMIZE
void sc_uint_base::concat_set(int64 src, int low_i)
{
    *this = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_uint_base::concat_set(const sc_signed& src, int low_i)
{
    if ( low_i < src.length() )
        *this = src >> low_i;                             
    else
        *this = (src < 0) ? (int_type)-1 : 0; 
}

void sc_uint_base::concat_set(const sc_unsigned& src, int low_i)
{
    if ( low_i < src.length() )
        *this = src >> low_i;
    else
        *this = 0;
}

void sc_uint_base::concat_set(uint64 src, int low_i)
{
    *this = (low_i < 64) ? src >> low_i : 0;
}


// other methods

void
sc_uint_base::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}

} // namespace sc_dt


// Taf!
