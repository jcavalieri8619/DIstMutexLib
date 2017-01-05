

/*
 * File:   message_buffer.hpp
 * Author: jcavalie
 *
 * Created on November 21, 2016, 6:51 PM
 */

#ifndef MESSAGE_BUFFER_HPP
#define MESSAGE_BUFFER_HPP
#include <array>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <pthread.h>
#include "globals.hpp"
#include "node_controller.hpp"

#include <iostream>

namespace nodeController
{

    template< size_t nrows, size_t buffsize>
    class MessageBuffer
    {
        pthread_mutex_t* buffer_lock[2 * nrows];
        pthread_cond_t* buffer_condvar[2 * nrows];

        std::array<Message, buffsize> msgbuffer[nrows];

        size_t in_ptr[nrows] = { 0 };
        size_t out_ptr[nrows] = { 0 };

        int space_count[nrows] = { buffsize };
        int msg_count[nrows] = { 0 };

        char* build_error_str( )
        {


#if (( _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 ) && ! _GNU_SOURCE)

            static char buff[256] = { 0 };

            strerror_r( errno, buff, 256 );

            return buff;
#else
            char buff[256];
            return strerror_r( errno, buff, 256 );
#endif


        }

        size_t compute_controlvar_index( Action act, int whichrow )
        {
            return (act == Action::PRODUCE ) ?
                    static_cast<size_t> ( act ) * whichrow :
                    ///////////////////ELSE CASE/////////////////////////
                    ( static_cast<size_t> ( Action::PRODUCE ) * whichrow +
                      static_cast<size_t> ( act ) );
        }

    public:

        size_t get_size( int whichrow )
        {
            return msg_count[whichrow];
        }

        MessageBuffer( )
        {


            for ( int itr = 0; itr < 2 * nrows; itr++ ) {
                if (  itr  < nrows ) {
                    space_count[itr] = buffsize;
                    msg_count[itr] = 0;
                    in_ptr[itr]=0;
                    out_ptr[itr]=0;

                }

                buffer_lock[itr] = new pthread_mutex_t;
                buffer_condvar[itr] = new pthread_cond_t;

                if ( pthread_mutex_init( buffer_lock[itr], NULL ) != 0 ||
                     pthread_cond_init( buffer_condvar[itr], NULL ) != 0 ) {
                    throw std::runtime_error( build_error_str( ) );

                }

            }

        }

        ~MessageBuffer( )
        {
            for ( int itr = 0; itr < 2 * nrows; itr++ ) {
                delete buffer_lock[itr];
                delete buffer_condvar[itr];
            }
        }

        Message consume_msg( int whichrow )
        {

            if ( whichrow < 0 || whichrow >= nrows )
                throw std::runtime_error( "message_buffer consume:"
                                          " index out of range" );
            Message msg;

            size_t my_indx;
            my_indx = compute_controlvar_index( Action::CONSUME, whichrow );



            if ( pthread_mutex_lock( buffer_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            while ( msg_count[whichrow] == 0 ) {
                if ( pthread_cond_wait( buffer_condvar[my_indx], buffer_lock[my_indx] ) != 0 ) {
                    throw std::runtime_error( build_error_str( ) );
                }
            }


            msg_count[whichrow]--;


            msg = msgbuffer[whichrow].at( out_ptr[whichrow]++ % buffsize );


            if ( pthread_mutex_unlock( buffer_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }


            if ( pthread_mutex_lock( buffer_lock[my_indx - 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            space_count[whichrow]++;

            if ( pthread_mutex_unlock( buffer_lock[my_indx - 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            if ( pthread_cond_broadcast( buffer_condvar[my_indx - 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            return msg;


        }

        void produce_msg( int whichrow, Message newmsg )
        {
            if ( whichrow < 0 || whichrow >= nrows )
                throw std::runtime_error( "message_buffer produce:"
                                          " index out of range" );

            size_t my_indx;
            my_indx = compute_controlvar_index( Action::PRODUCE, whichrow );


            if ( pthread_mutex_lock( buffer_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            while ( space_count[whichrow] == 0 ) {
                if ( pthread_cond_wait( buffer_condvar[my_indx], buffer_lock[my_indx] ) != 0 ) {
                    throw std::runtime_error( build_error_str( ) );
                }
            }

            space_count[whichrow]--;


            msgbuffer[whichrow].at( in_ptr[whichrow]++ % buffsize ) = newmsg;

            if ( pthread_mutex_unlock( buffer_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }


            if ( pthread_mutex_lock( buffer_lock[my_indx + 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            msg_count[whichrow]++;

            if ( pthread_mutex_unlock( buffer_lock[my_indx + 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            if ( pthread_cond_broadcast( buffer_condvar[my_indx + 1] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }


        }
    };
}
#endif /* MESSAGE_BUFFER_HPP */

