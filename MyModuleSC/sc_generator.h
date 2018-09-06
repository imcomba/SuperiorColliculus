/*
 *  ac_generator.h
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

#ifndef SC_GENERATOR_H
#define SC_GENERATOR_H

// provides input current

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "stimulating_device.h"
#include "universal_data_logger.h"

/* BeginDocumentation
   Name: sc_generator - provides the input current to FEF layer neurons.
   Description:

   This device produce a current which are sent by a CurrentEvent. The
   current is given by

           I(t) = i0*exp(-(distance**2)/(2*(pop**2)))*(t**gamma)*exp(-beta*t)

   The parameters are

   i0       double -  Scaling constant
   beta     double -  Measure for burst duration
   gamma    double -  Skewness and peak of the burst
   pop      double -  Recruited population size
   distance double -  Difference between the position of a neuron and the nueron where the saccade is produced
    
   Setting start and stop (see StimulatingDevice) only windows the current
   as defined above. It does not shift the time axis.

   References:
   [1] S. Rotter and M. Diesmann, Exact digital simulation of time-
   invariant linear systems with applications to neuronal modeling,
   Biol. Cybern. 81, 381-402 (1999)

   Sends: CurrentEvent

   Author: Johan Hake, Spring 2003

   SeeAlso: Device, StimulatingDevice, dc_generator, step_current_generator
*/

namespace nest
{ namespace names
    {extern const Name i0;
     extern const Name beta;
     extern const Name gamma;
     extern const Name pop;
     extern const Name distance;
     extern const Name sc_onset;
        }
class sc_generator : public Node
{

public:
  sc_generator();
  sc_generator( const sc_generator& );

  bool
  has_proxies() const
  {
    return false;
  }

  port send_test_event( Node&, rport, synindex, bool );

  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& );

  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  //! Allow multimeter to connect to local instances
  bool
  local_receiver() const
  {
    return true;
  }

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );


  // ------------------------------------------------------------

  struct Parameters_
  {
    double i0_;     //!< Amplitude of sine-current
    double beta_;  //!< Offset of sine-current
    double gamma_;    //!< Standard frequency in Hz
    double pop_; //!< Phase of sine current (0-360 deg)
    double distance_;
    double sc_onset_;
    
    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  struct State_
  {
   
    double I_; //!< Instantaneous current value; used for recording current
               //!< Required to handle current values when device is inactive

    State_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< sc_generator >;
  friend class UniversalDataLogger< sc_generator >;

  // ------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( sc_generator& );
    Buffers_( const Buffers_&, sc_generator& );
    UniversalDataLogger< sc_generator > logger_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
   
    
  };

  double
  get_I_() const
  {
    return S_.I_;
  }

  // ------------------------------------------------------------

  StimulatingDevice< CurrentEvent > device_;
  static RecordablesMap< sc_generator > recordablesMap_;
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
sc_generator::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool )
{
  device_.enforce_single_syn_type( syn_id );

  CurrentEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
sc_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
sc_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  device_.get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
sc_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  // State_ is read-only

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
}
#endif // SC_GENERATOR_H
