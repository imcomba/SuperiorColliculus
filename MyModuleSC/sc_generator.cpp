/*
 *  ac_generator.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sc_generator.h"

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


namespace nest
{
    namespace names
    {const Name i0("i0");
     const Name beta("beta");
     const Name gamma("gamma");
     const Name pop("pop");
     const Name distance("distance");
     const Name sc_onset("sc_onset");
        }
RecordablesMap< sc_generator > sc_generator::recordablesMap_;

template <>
void
RecordablesMap< sc_generator >::create()
{
  insert_( Name( names::I ), &sc_generator::get_I_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::sc_generator::Parameters_::Parameters_()
  : i0_( 0.0 )    // pA 
  , beta_( 0.0 )  
  , gamma_( 0.0 )    
  , pop_( 0.0 ) // mm
  , distance_( 0.0 ) // mm
  , sc_onset_( 0.0 ) // ms
{
}

nest::sc_generator::Parameters_::Parameters_( const Parameters_& p )
  : i0_( p.i0_ )
  , beta_( p.beta_ )
  , gamma_( p.gamma_ )
  , pop_( p.pop_ )
  , distance_( p.distance_ )
  , sc_onset_( p.sc_onset_ )
{
}

nest::sc_generator::Parameters_& nest::sc_generator::Parameters_::operator=(
  const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  i0_ = p.i0_;
  beta_ = p.beta_;
  gamma_ = p.gamma_;
  pop_ = p.pop_;
  distance_ =p.distance_;
  sc_onset_ =p.sc_onset_;

  return *this;
}

nest::sc_generator::State_::State_()
    : I_( 0.0 )   // pA
{
}

nest::sc_generator::Buffers_::Buffers_( sc_generator& n )
  : logger_( n )
{
}

nest::sc_generator::Buffers_::Buffers_( const Buffers_&, sc_generator& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::sc_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::i0 ] = i0_;
  ( *d )[ names::beta ] = beta_;
  ( *d )[ names::gamma ] = gamma_;
  ( *d )[ names::pop ] = pop_;
  ( *d )[ names::distance ] = distance_;
  ( *d )[ names::sc_onset ] = sc_onset_;
}

void
nest::sc_generator::State_::get( DictionaryDatum& d ) const
{
  
}

void
nest::sc_generator::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::i0, i0_ );
  updateValue< double >( d, names::beta, beta_ );
  updateValue< double >( d, names::gamma, gamma_ );
  updateValue< double >( d, names::pop, pop_ );
  updateValue< double >( d, names::distance, distance_ );  
  updateValue< double >( d, names::sc_onset, sc_onset_ ); 
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::sc_generator::sc_generator()
  : Node()
  , device_()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::sc_generator::sc_generator( const sc_generator& n )
  : Node( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::sc_generator::init_state_( const Node& proto )
{
  const sc_generator& pr = downcast< sc_generator >( proto );

  device_.init_state( pr.device_ );
  S_ = pr.S_;
}

void
nest::sc_generator::init_buffers_()
{
  device_.init_buffers();
  B_.logger_.reset();
}

void
nest::sc_generator::calibrate()
{
  B_.logger_.init();

  device_.calibrate();


  
}

void
nest::sc_generator::update( Time const& origin, const long from, const long to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  long start = origin.get_steps();

  CurrentEvent ce;
  for ( long lag = from; lag < to; ++lag )
  {
    // We need to iterate the oscillator throughout all steps, even when the
    // device is not active, since inactivity only windows the oscillator.
    double t=Time(Time::step( start + lag )).get_ms()-P_.sc_onset_;
    if (t<0){
        t=0;
    }
    double first_part=P_.i0_*std::exp(-((P_.distance_*P_.distance_))/(2*(P_.pop_*P_.pop_)));
    double second_part=std::pow(t,P_.gamma_)*std::exp(-P_.beta_*t);
    double Iext=first_part*second_part;
    //std::cout<<"Calculating current: lag:"<<lag<<".time:"<<t<<"Input current"<<Iext<<std::endl; 
    
    S_.I_ = 0.0;
    if ( device_.is_active( Time::step( start + lag ) ) )
    {
        S_.I_ = Iext;
      ce.set_current( S_.I_ );
      kernel().event_delivery_manager.send( *this, ce, lag );
    }
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::sc_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
