#include "globals.hpp"
#include "node_controller.hpp"

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <bitset>
#include <iomanip>
#include <fstream>
#include <netinet/in.h>
#include <algorithm>
#include <vector>
#include <ctime>

using std::endl;
using std::cerr;
using std::bitset;
using std::noboolalpha;
using std::boolalpha;
using std::setw;
using std::left;
using std::list;
using std::vector;
using std::find_if;

extern unsigned MY_NODE_ID;

namespace
{

    union MessageContainer
    {
        MessageContainer( ) : bytestream{ 0 }
        {
        }
        nodeController::Message message;

        char bytestream[sizeof (message )];
    } ;

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
}


std::ostream& operator<<( std::ostream& os, Action act )
{
    switch ( act ) {
        case Action::CONSUME:
            os << "CONSUME";
            break;
        case Action::PRODUCE:
            os << "PRODUCE";
            break;
    }
    return os;
}

std::ostream& operator<<( std::ostream& os, subAction sub_act )
{

    char strbuffer[12] = { 0 };
    snprintf( strbuffer, 12, "FLAVOR_%u", static_cast<unsigned> ( sub_act ) );
    os << strbuffer;
    return os;
}

Socket::Socket( int channel ) : channel( channel )
{
    if ( channel < 0 ) {
        throw std::runtime_error( build_error_str( ) );
    }
};

Socket::Socket( int family, int type, int protocol )
{
    int fd;
    if ( ( fd = socket( family, type, protocol ) ) < 0 ) {
        throw std::runtime_error( build_error_str( ) );
    }

    channel = fd;
}

Socket::Socket( ) : channel( UNINITIALIZED ){ };

Socket::Socket( const Socket& arg )
{
    int fd;
    if ( ( fd = dup( arg.get_channel( ) ) ) < 0 ) {
        throw std::runtime_error( build_error_str( ) );
    }

    channel = fd;

}

const Socket& Socket::operator=(const Socket& RHS )
{

    int fd;
    if ( ( fd = dup( RHS.get_channel( ) ) ) < 0 ) {
        throw std::runtime_error( build_error_str( ) );
    }

    channel = fd;
}

Socket::~Socket( )
{
    close( channel );
};

void Socket::close_socket( )
{
    this->~Socket( );
}

int Socket::get_channel( ) const
{
    if ( channel == UNINITIALIZED ) {
        throw std::runtime_error( "SOCKET attempting to use uninit socket" );

    }
    return channel;
};

size_t compute_buffer_index( Action act, subAction sub_act )
{
    return (act == Action::PRODUCE ) ?
            static_cast<size_t> ( act ) * static_cast<size_t> ( sub_act ) :
            /////////////////////////ELSE CASE///////////////////////////////
            ( static_cast<size_t> ( Action::PRODUCE ) *
              static_cast<size_t> ( sub_act ) +
              static_cast<size_t> ( act ) );
}


namespace nodeController
{

    remoteMonitor::accept_info::accept_info( const Socket& listen_socket
                                             , pthread_mutex_t* accept_lock ) :
        listen_socket( listen_socket ),
        accept_lock( accept_lock ){ }

    localMonitor::accept_info::accept_info( const Socket& listen_socket,
                                            pthread_mutex_t* accept_lock ) :
        listen_socket( listen_socket ), accept_lock( accept_lock ){ }

    void host2network_msg( const Message& msg, char* buffer )
    {

        MessageContainer converter;
        converter.message = msg;

        converter.message.act = static_cast<Action>
                ( htonl( static_cast<uint32_t> ( converter.message.act ) ) );

        converter.message.node_ID = htonl( static_cast<uint32_t>
                                           ( converter.message.node_ID ) );

        converter.message.rcvdReplies = htonl( static_cast<uint32_t>
                                               ( converter.message.rcvdReplies ) );

        converter.message.sub_act = static_cast<subAction>
                ( htonl( static_cast<uint32_t> ( converter.message.sub_act ) ) );

        converter.message.time_stamp = htonl( ( converter.message.time_stamp ) );

        converter.message.type = static_cast<MESSAGE_T>
                ( htonl( static_cast<uint32_t> ( converter.message.type ) ) );

        //the loc_msg ptr will always be NULL for any msg being sent
        //over network so this isn't required--and its 64 bits,
        //htonl is only good for 32 bits.
        //converter.message.loc_msg=htonl(converter.message.loc_msg);

        for ( int itr = 0; itr < sizeof (converter.bytestream );
              ( buffer[itr] = converter.bytestream[itr] ), itr++ );


        return;
    }

    void network2host_msg( Message& msg, const char* buffer )
    {
        MessageContainer converter;

        for ( int itr = 0; itr < sizeof (converter.bytestream );
              ( converter.bytestream[itr] = buffer[itr] ), itr++ );


        converter.message.act = static_cast<Action>
                ( ntohl( static_cast<uint32_t> ( converter.message.act ) ) );

        converter.message.node_ID = ntohl( static_cast<uint32_t>
                                           ( converter.message.node_ID ) );

        converter.message.rcvdReplies = ntohl( static_cast<uint32_t>
                                               ( converter.message.rcvdReplies ) );

        converter.message.sub_act = static_cast<subAction>
                ( ntohl( static_cast<uint32_t> ( converter.message.sub_act ) ) );

        converter.message.time_stamp = ntohl( ( converter.message.time_stamp ) );

        converter.message.type = static_cast<MESSAGE_T>
                ( ntohl( static_cast<uint32_t> ( converter.message.type ) ) );

        msg = converter.message;

        return;
    }

    void update_rcvd_replies_array( unsigned node_ID, unsigned& reply_array )
    {

        bitset<NODECOUNT> bit_array( reply_array );
        bit_array.set( node_ID );
        reply_array = bit_array.to_ulong( );
    }

    void client2internal_msg( Message& internal_msg,
                              const clientUtils::Message& client_msg,
                              struct sockaddr_un client_addr,
                              socklen_t client_len )
    {
        //check off the reply for local node
        //local node ID is global variable MY_NODE_ID
        internal_msg.node_ID = MY_NODE_ID;

        internal_msg.act = client_msg.act;
        internal_msg.sub_act = client_msg.subact;
        internal_msg.time_stamp = LogicalClock::UNITIALIZED;

        update_rcvd_replies_array( MY_NODE_ID, internal_msg.rcvdReplies );

        if ( client_msg.type == clientUtils::MESSAGE_T::LOCAL_REQUEST ) {

            internal_msg.type = MESSAGE_T::REQUEST;

        } else if ( client_msg.type == clientUtils::MESSAGE_T::FINISHED ) {

            internal_msg.type = MESSAGE_T::REPLY;

        } else {
            if ( DO_DEBUG ) {
                cerr << "client2internal_msg: client sent invalid message "
                        "type\n" << client_msg << endl;
            }
            throw std::runtime_error( "client2internal_msg: "
                                      "client sent invalid message"
                                      " type to node controller" );
        }
        internal_msg.loc_msg = new localMessage{ client_addr,
                                                client_len,
                                                client_msg.localPID };
        return;
    }

    /**
     *
     * @param internal_msg internal message generated by local communication
     *      monitor on receipt of request message from client
     * @param client_msg client message converted from internal message
     * @param type the only message type that requires internal to client
     *      conversion is PERMISSION_GRANTED indicating that it is safe
     *      for local client to enter critical section.  It is left as an
     *      argument incase of later extension.
     *
     */
    void internal2client_msg( const Message& internal_msg,
                              clientUtils::Message& client_msg,
                              clientUtils::MESSAGE_T type )
    {
        if ( type != clientUtils::MESSAGE_T::PERMISSION_GRANTED ) {
            cerr << "WARNGING: internal2client_msg possible invalid "
                    "argument for MESSAGE_T type\n" << type << endl;
        }
        client_msg.localPID = internal_msg.loc_msg->localPID;
        client_msg.act = internal_msg.act;
        client_msg.subact = internal_msg.sub_act;
        client_msg.type = type;

        return;
    }

    LogicalClock::LogicalClock( ) : time_guard( new pthread_mutex_t ), time( 0 )
    {
        if ( pthread_mutex_init( time_guard, NULL ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }
    }

    LogicalClock::~LogicalClock( )
    {
        delete time_guard;
    }

    void LogicalClock::operator()( logical_time& msg_time )
    {
        if ( msg_time == UNITIALIZED ) {
            //here we are constructing new message and
            //time stamping it then incrementing our
            //local clock to one greater than any time
            //stamp we've ever used.

            if ( pthread_mutex_lock( time_guard ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            msg_time = time++;

            if ( pthread_mutex_unlock( time_guard ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }


        } else {

            //here we are comparing local clock against
            //incoming messages to ensure local clock is
            //one greater than any time stamp we've seen
            if ( pthread_mutex_lock( time_guard ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

            if ( msg_time >= time ) {
                time = msg_time + 1;
            }

            if ( pthread_mutex_unlock( time_guard ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }


        }


        return;
    }

    void LogicalClock::clear_timestamp( logical_time& msg_time )
    {
        msg_time = UNITIALIZED;
        return;
    }

    std::ostream& operator<<( std::ostream& os, MESSAGE_T mtype )
    {
        switch ( mtype ) {
            case MESSAGE_T::REPLY:
                os << "REPLY";
                break;
            case MESSAGE_T::REQUEST:
                os << "REQUEST";
                break;
        }
        return os;
    }

    std::ostream& operator<<( std::ostream& os, const Message& msg )
    {

        os << "Message Type: " << ( msg.type ) << endl
                << "Action: " << ( msg.act ) << endl
                << "subAction: " << ( msg.sub_act ) << endl
                << "node_ID: " << msg.node_ID << endl
                << "time_stamp: " << msg.time_stamp << endl
                << "received_replies: ";

        //19 spaces to right
        bitset<NODECOUNT> bit_array( msg.rcvdReplies );
        size_t width = os.width( );
        for ( int i = 0; i < NODECOUNT; i++ ) {

            if ( !( ( i + 1 ) % 10 ) )
                os << endl << "\t\t";

            os << left << noboolalpha << "N_" << i << ": ";
            os << boolalpha << setw( 4 ) << left << bit_array.test( i ) << ",  ";
            os << setw( width );

        }


        os << noboolalpha << endl
                << "local_message: ";
        if ( msg.loc_msg ) {
            os << endl << "\t"
                    << "local PID: " << msg.loc_msg->localPID << endl << "\t"
                    << "sun_path: " << msg.loc_msg->cliaddr.sun_path << endl << "\t"
                    << "sun_family: " << msg.loc_msg->cliaddr.sun_family << endl << "\t"
                    << "size: " << msg.loc_msg->clilen << endl;
        } else os << "NULLPTR" << endl;

        return os;
    }

    bool Message::operator==(const Message& rhs ) const
    {

        return (( ( rhs.act == act ) &&
                  ( rhs.loc_msg == loc_msg ) &&
                  ( rhs.node_ID == node_ID ) &&
                  ( rhs.sub_act == sub_act ) &&
                  ( rhs.time_stamp == time_stamp ) &&
                  ( rhs.type == type ) &&
                  ( bitset<NODECOUNT>( rhs.rcvdReplies ) ==
                    bitset<NODECOUNT>( rcvdReplies ) ) ) );

    }

    bool Message::operator<(const Message& rhs ) const
    {
        if ( time_stamp == LogicalClock::UNITIALIZED ||
             rhs.time_stamp == LogicalClock::UNITIALIZED ) {
            throw std::runtime_error( "Message lessThan_OP:"
                                      " uninitialized time stamp in request queue" );
        }

        return (time_stamp == rhs.time_stamp ) ?
                ( node_ID < rhs.node_ID ) : ( time_stamp < rhs.time_stamp );

    }

    size_t Ricart_RequestQueue::compute_controlvar_index( Action act,
                                                          subAction sub_act,
                                                          OPTION opt )
    {
        size_t indx;

        switch ( opt ) {
            case OPTION::CONSUME:
                indx = 2 * compute_queue_index( act, sub_act ) + 1;
                break;
            case OPTION::PRODUCE:
                indx = 2 * compute_queue_index( act, sub_act );
                break;
        }

        if ( indx >= 4 * NUMFLAVORS ) {
            throw std::runtime_error( "Ricart_Q: computed control index outside "
                                      "valid range--index >= 4*NUMFLAVORS" );
        }

        return indx;

    }

    //Action enum: consume=1, produce=2

    size_t Ricart_RequestQueue::compute_queue_index( Action act, subAction sub_act )
    {
        return (act == Action::PRODUCE ) ?
                static_cast<size_t> ( act ) * static_cast<size_t> ( sub_act ) :
                /////////////////////////ELSE CASE///////////////////////////////
                ( static_cast<size_t> ( Action::PRODUCE ) *
                  static_cast<size_t> ( sub_act ) +
                  static_cast<size_t> ( act ) );
    }




    //these test functions must be code-locked so they're private
    //because I will ensure these are called atomically; these functions
    //are equivalent to space_count and message_count and these must
    //also be accessed atomically

    bool Ricart_RequestQueue::is_empty( Action act, subAction sub_act )
    {
        return !queues[compute_queue_index( act, sub_act )].size( );
    }

    bool Ricart_RequestQueue::is_full( Action act, subAction sub_act )
    {
        return queues[compute_queue_index( act, sub_act )].size( ) == queue_size;
    }

    bool Ricart_RequestQueue::has_Qhead_changed( Action act, subAction sub_act )
    {
        return Qhead_changed[compute_queue_index( act, sub_act )];
    }

    Ricart_RequestQueue::Ricart_RequestQueue( size_t queue_size ) :
        queue_size( queue_size )

    {
        for ( int itr = 0; itr < 4 * NUMFLAVORS; itr++ ) {

            if ( itr < 2 * NUMFLAVORS ) {

                queue_lock[itr] = new pthread_mutex_t;

                if ( pthread_mutex_init( queue_lock[itr], NULL ) != 0 ) {
                    throw std::runtime_error( build_error_str( ) );
                }


                Qhead_condvar[itr] = new pthread_cond_t;

                if ( pthread_cond_init( Qhead_condvar[itr], NULL ) != 0 ) {
                    throw std::runtime_error( build_error_str( ) );

                }



                client_barrier[itr] = new pthread_barrier_t;



                if ( pthread_barrier_init( client_barrier[itr], NULL,  2 ) ) {
                    throw std::runtime_error( build_error_str( ) );
                }


            }

            queue_condvar[itr] = new pthread_cond_t;

            if ( pthread_cond_init( queue_condvar[itr], NULL ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );

            }

        }
    }

    Ricart_RequestQueue::~Ricart_RequestQueue( )
    {
        for ( int itr = 0; itr < 4 * NUMFLAVORS; itr++ ) {

            if ( itr < 2 * NUMFLAVORS ) {
                delete queue_lock[itr], Qhead_condvar[itr], client_barrier[itr];

            }

            delete queue_condvar[itr];
        }

    }

    size_t Ricart_RequestQueue::get_size( Action act,
                                          subAction sub_act, bool NONBLOCK )
    {
        size_t my_indx;
        my_indx = compute_queue_index( act, sub_act );

        if ( !NONBLOCK ) {
            if ( pthread_mutex_lock( queue_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }
        }

        size_t size = queues[my_indx].size( );

        if ( !NONBLOCK ) {
            if ( pthread_mutex_unlock( queue_lock[my_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }
        }

        return size;
    }

    void Ricart_RequestQueue::wait_local_client_finished( Action act,
                                                          subAction sub_act )
    {
        size_t my_indx;
        my_indx = compute_queue_index( act, sub_act );


        pthread_barrier_wait( client_barrier[my_indx] );


        return;

    }

    void Ricart_RequestQueue::notify_local_client_finished( Action act,
                                                            subAction sub_act )
    {
        size_t my_indx;
        my_indx = compute_queue_index( act, sub_act );


        pthread_barrier_wait( client_barrier[my_indx] );

        return;

    }

    void Ricart_RequestQueue::enqueue_request( Action act,
                                               subAction sub_act,
                                               Message rqst_msg )
    {
        size_t Qindx;
        size_t prod_indx, cons_indx;

        Qindx = compute_queue_index( act, sub_act );

        prod_indx = compute_controlvar_index( act, sub_act, OPTION::PRODUCE );

        cons_indx = compute_controlvar_index( act, sub_act, OPTION::CONSUME );




        if ( pthread_mutex_lock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }


        while ( is_full( act, sub_act ) ) {


            if ( pthread_cond_wait( queue_condvar[prod_indx],
                                    queue_lock[Qindx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }
        }


        bool empty_b4_enqueue = is_empty( act, sub_act );


        Message old_head;

        if ( !empty_b4_enqueue ) {
            old_head == queues[Qindx].front( );
        }

        bool enqueued_rqst;

        //remember--most_recent_finished holds timestamps of most recent
        //ONLY local clients that already finished C.S.
        if ( rqst_msg.time_stamp >= most_recent_finished_timestamps[Qindx] ) {


            enqueued_rqst = true;

            queues[Qindx].push_back( rqst_msg );
            queues[Qindx].sort( );


        } else {

            enqueued_rqst = false;

            if ( DO_DEBUG ) {
                cerr << "enqueue_request: WARNING\n"
                        "received request message with "
                        "timestamp: " << rqst_msg.time_stamp <<
                        " which is strictly less than timestamp " <<
                        most_recent_finished_timestamps[Qindx] << " from most\n"
                        "recent local client to execute critical section--"
                        "implying the node who sent this request message\n"
                        "has already replied to my local client's request with\n"
                        " timestamp smaller timestamp than " << rqst_msg.time_stamp <<
                        " and therefore this request message's timestamp"
                        " has already been satisfied;\n"
                        "offending request message: \n" << rqst_msg << endl;
            }
        }



        bool head_changed_ = false;

        if ( enqueued_rqst ) {
            if ( !empty_b4_enqueue ) {

                if ( head_changed_ = ( !( old_head == queues[Qindx].front( ) ) ) ) {
                    Qhead_changed[Qindx] = true;

                } else {
                    Qhead_changed[Qindx] = false;
                }
            } else {
                //queue was empty prior to enqueue so head has changed
                head_changed_ = true;
                Qhead_changed[Qindx] = true;
            }
        } else Qhead_changed[Qindx] = false;



        if ( pthread_mutex_unlock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }


        if ( head_changed_  ) {
            if ( pthread_cond_broadcast( Qhead_condvar[Qindx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

        }

        if ( enqueued_rqst ) {
            if ( pthread_cond_broadcast( queue_condvar[cons_indx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

        }

    }

    void Ricart_RequestQueue::remove_finished_peers( Action act,
                                                     subAction sub_act,
                                                     Message reply_msg )
    {

        class predicate
        {
            logical_time time_stamp;
            unsigned node_ID;
        public:

            //FIND: time_stamp argument is taken from REPLY message
            //and node_ID is ME to find where I can begin
            //popping off replied to messages
            //REMOVE: time_stamp is the time_stamp of my message
            //returned from find_if and node_ID is the node_ID
            //taken from the REPLY message indicate who sent the REPLY
            //message

            predicate( logical_time time_stamp, int node_ID ) :
                time_stamp( time_stamp ), node_ID( node_ID )
            {
                if ( time_stamp == LogicalClock::UNITIALIZED )
                    throw std::runtime_error( "uninitialized logical_time"
                                              " inside request queue" );

            }

            bool operator()(const Message& curr_msg )
            {

                return ( curr_msg.node_ID == node_ID ) &&
                        ( curr_msg.time_stamp < time_stamp );

            }

        } ;

        size_t Qindx;
        size_t prod_indx, cons_indx;

        Qindx = compute_queue_index( act, sub_act );

        prod_indx = compute_controlvar_index( act, sub_act, OPTION::PRODUCE );

        cons_indx = compute_controlvar_index( act, sub_act, OPTION::CONSUME );



        if ( pthread_mutex_lock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }



        list<Message>::reverse_iterator start, end, itr2me;
        vector<list<Message>::reverse_iterator> my_msgs;

        start = queues[Qindx].rbegin( );
        end = queues[Qindx].rend( );

        int itr_count = 0;

        int loop_count;
        while ( itr_count++ < queue_size ) {


            loop_count = 0;
            while ( ( ( itr2me = find_if( start,
                                          end,
                                          predicate( reply_msg.time_stamp,
                                                     MY_NODE_ID ) ) ) == end ) ) {


                if ( my_msgs.size( ) ) {

                    break;
                }


                if ( loop_count >= 100 ) {
                    cerr << "RequestQueue remove_finish_peers:\n"
                            "looped 100 times waiting on queue_condvar"
                            " for local client to show up in queue,"
                            " but never happened. ERROR" << endl;
                    exit( EXIT_FAILURE );

                }

                timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                now.tv_sec += 1;
                int retval;
                if ( ( ( retval = pthread_cond_timedwait( queue_condvar[cons_indx],
                                                          queue_lock[Qindx], &now ) ) != 0 ) ||
                     retval != ETIMEDOUT ) {
                    //throw std::runtime_error( build_error_str( ) );
                    ;
                }
                start = queues[Qindx].rbegin( );
                end = queues[Qindx].rend( );

                loop_count++;
            }


            if ( itr2me != end ) {
                my_msgs.push_back( itr2me );
                start = itr2me;
            } else break;

        }



        //queue can't be empty here given that we've left the while loop
        //implying there is at least 1 msg in queue--local msg being replied to
        Message old_head = queues[Qindx].front( );


        for ( auto & my_msgs_itr : my_msgs ) {
            update_rcvd_replies_array( reply_msg.node_ID,
                                       my_msgs_itr->rcvdReplies );
        }


        queues[Qindx].remove_if( predicate( my_msgs.front( )->time_stamp,
                                            reply_msg.node_ID ) );



        bool head_changed_ = false;

        if ( !is_empty( act, sub_act ) ) {
            //need to make sure queue is not empty before attempting to
            //obtain reference to queue head via front()
            if ( head_changed_ = ( !( old_head == queues[Qindx].front( ) ) ) ) {
                Qhead_changed[Qindx] = true;

            } else Qhead_changed[Qindx] = false;

        } else Qhead_changed[Qindx] = false;



        if ( pthread_mutex_unlock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

        if ( head_changed_ ) {

            if ( pthread_cond_broadcast( Qhead_condvar[Qindx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }

        }


        if ( pthread_cond_broadcast( queue_condvar[prod_indx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

    }

    bool Ricart_RequestQueue::is_local_client_request( const Message& msg )
    {
        bool rv;
        if ( rv = ( msg.node_ID == MY_NODE_ID ) ) {

            if ( !msg.loc_msg ) {
                if ( DO_DEBUG ) {
                    cerr << "Request Queue local message null pointer:" <<
                            endl << msg << endl;
                }
                throw std::runtime_error( "Request Queue local client request has"
                                          " null loc_msg pointer" );
            }

        }

        return rv;
    }

    void Ricart_RequestQueue::pop_local_client_from_head( Action act,
                                                          subAction sub_act )
    {
        size_t Qindx;
        size_t prod_indx;

        Qindx = compute_queue_index( act, sub_act );

        prod_indx = compute_controlvar_index( act, sub_act, OPTION::PRODUCE );


        if ( pthread_mutex_lock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }


        //I may arrive at queue head because I just popped a deferred list
        //and I had a msg immediately after deferred list--so I'd be at front
        //without having all REPLY msgs yet given that the people I just replied
        //to are about to do their C.S. and will not REPLY to me until they are done
        while ( !bitset<NODECOUNT>( queues[Qindx].front( ).rcvdReplies ).all( ) ) {

            if ( pthread_cond_wait( Qhead_condvar[Qindx],
                                    queue_lock[Qindx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }
        }

        Message head_msg = queues[Qindx].front( );

        if ( ! head_msg.loc_msg ) {
            std::runtime_error( "RQST_Q popping local client but"
                                " head element has NULL loc_msg pointer" );
        }


        most_recent_finished_timestamps[Qindx] = head_msg.time_stamp;

        queues[Qindx].pop_front( );

        Qhead_changed[Qindx] = true;

        if ( pthread_mutex_unlock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

        if ( pthread_cond_broadcast( Qhead_condvar[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

        if ( pthread_cond_broadcast( queue_condvar[prod_indx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

        return;

    }

    void Ricart_RequestQueue::pop_deferred_reply_list( Action act,
                                                       subAction sub_act,
                                                       std::list<Message>&
                                                       deferred_list )
    {

        size_t Qindx;
        size_t prod_indx;

        Qindx = compute_queue_index( act, sub_act );


        prod_indx = compute_controlvar_index( act, sub_act, OPTION::PRODUCE );

        if ( pthread_mutex_lock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }


        auto end_range = std::find_if( queues[Qindx].begin( ),
                                       queues[Qindx].end( ),
                                       [](const Message & msg ){
                                           return msg.node_ID == MY_NODE_ID;
                                       } );

        deferred_list.splice( deferred_list.begin( ), queues[Qindx],
                              queues[Qindx].begin( ), end_range );

        bool head_changed_ = false;
        if ( !( is_empty( act, sub_act ) ) ) {
            Qhead_changed[Qindx] = head_changed_ = true;

        } else Qhead_changed[Qindx] = false;

        if ( pthread_mutex_unlock( queue_lock[Qindx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }

        if ( head_changed_ ) {
            if ( pthread_cond_broadcast( Qhead_condvar[Qindx] ) != 0 ) {
                throw std::runtime_error( build_error_str( ) );
            }
        }


        if ( pthread_cond_broadcast( queue_condvar[prod_indx] ) != 0 ) {
            throw std::runtime_error( build_error_str( ) );
        }


        return;

    }

    void Ricart_RequestQueue::print_queue_to_file( const char* filename )
    {
        using namespace std;

        ofstream outputFile ( filename, ofstream::out );


        int whichrow = 0;
        for ( int subact_itr = 0; subact_itr < NUMFLAVORS; subact_itr++ ) {

            for ( int act_itr = 2; act_itr > 0 ; act_itr-- ) {

                list<Message> Qrow = queues[whichrow++];

                outputFile << left << setw( 12 ) << setfill( ' ' ) << static_cast<Action> ( act_itr ) <<
                        "\n" << setw( 12 ) << static_cast<subAction> ( subact_itr ) << endl;

                for ( const auto & msg : Qrow ) {
                    outputFile << setw( 12 ) << left << setfill( ' ' ) << msg << endl;
                }

                //queue row separator marker because rows are stacked on vertically
                outputFile << setw( 50 ) << setfill( '-' );
                outputFile << setfill( ' ' ) << endl << endl;


            }

        }
        outputFile.flush( );
    }




}