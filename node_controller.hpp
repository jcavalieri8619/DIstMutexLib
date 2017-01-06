/*
 * File:   node_controller.hpp
 * Author: jcavalie
 *
 * Created on November 14, 2016, 3:51 PM
 */

#ifndef NODE_CONTROLLER_HPP
#define NODE_CONTROLLER_HPP

#include "globals.hpp"
#include "client_utils.hpp"
#include <list>
#include <pthread.h>
#include <sys/un.h>
#include <array>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdint>





#define MSG_BUFF_SIZE 1000

std::ostream& operator<<( std::ostream& os, Action act );

std::ostream& operator<<( std::ostream& os, subAction sub_act );

class Socket
{
    int channel;

    enum
    {
        UNINITIALIZED = -1
    };

public:

    Socket( int channel );

    Socket( int family, int type, int protocol = 0 );

    Socket( );

    Socket( const Socket& );

    const Socket& operator=(const Socket&);

    ~Socket( );

    void close_socket( );

    int get_channel( ) const;

};

namespace nodeController
{

    /**
     * LocalSendersock is Unix Domain socket address used for communicating
     * non-request messages between node controller and local clients;
     * e.g. PERMISSION GRANTED, FINISHED_CS
     *
     * an example address would be NC0_local_sending.2.0.sock -- this address
     * is for node controller 0 (NC0) and 2.0 indicates which combination of
     * ACTION.subAction i.e. 2.0 --> PRODUCE FLAVOR_1 per enum definitions in
     * globals.hpp
     *
     */
    static const std::string
    LocalSendersock_template( "/tmp/NC%d_local_sender.%d.%d.sock" );

    /**
     * LocalListensock is Unix domain socket address used for
     * communicating only REQUEST messages FROM local client TO node controller
     */
    static const std::string
    LocalListensock_template( "/tmp/NC%d_localListen_server.sock" );


    /**
     * monitors TCP socket for REQUEST messages from remote peer
     * node controllers
     * @param argument:  remoteMonitor::thread_arg
     * @return
     */
    void * remote_peer_monitor( void * argument );


    /**
     * monitors Unix domain socket at address LocalListensock_template for
     * REQUEST messages from local clients
     * @param argument:  localMonitor::thread_arg
     * @return
     */
    void * local_process_monitor( void* argument );


    /**
     * waits for notification from queue_notifier that a local client's
     * REQUEST has reached head of request queue and sends PERMISSION GRANTED
     * to that local client over Unix domain socket whose address is contained
     * within loc_msg field of the message then waits for FINISHED_CS message
     * from that local client.
     * once it receives FINISHED_CS, it will allow queue_notifier thread to pop
     * local client's REQUEST message from head of queue and then pop
     * deferred REPLY list from same queue.
     * @param argument thread_arg
     * @return
     */
    void * local_process_sender( void* argument );

    /**
     * all incoming messages from both local and remote monitor threads
     * are fed into this thread for timestamping and routed to appropriate
     * location--either remote_peer_sender and request_queue_manager for
     * incoming local REQUEST messages or just request_queue_manager for
     * incoming remote REQUEST and REPLY messages
     * @param argument thread_arg
     * @return
     */
    void * incoming_msg_handler( void* argument );

    /**
     * all outgoing REPLYs to remote peers are fed into this thread for
     * time-stamping and other record keeping then routed to remote_peer_sender.
     * After localSender thread receives FINISHED_CS message from local client, it
     * will allow queue_notifier to pop all messages whose REPLY was
     * deferred and each of those messages will be fed into this
     * thread where it is converted to REPLY message and its timestamp is updated
     * @param argument thread_arg
     * @return
     */
    void * outgoing_peer_replyMsg_handler( void* argument );


    /**
     * broadcasts outgoing REQUEST messages to all remote peer node controllers
     * and sends REPLY messages to particular remote peer nodes whose REQUEST
     * message is ahead of mine on requestQueue
     * @param argument  thread_arg
     * @return
     */
    void * remote_peer_sender( void* argument );


    /**
     * enqueues in sorted order all REQUEST messages into appropriate
     * row of requestQueue and dequeues any REQUEST messages that have been
     * REPLY'd too
     * @param argument  thread_arg
     * @return
     */
    void * request_queue_manager( void* argument );

    /**
     * monitors requestQueue row (thread_arg.act, thread_arg.sub_act) to determine
     * whether to notify local client that it has reached the head of its row or
     * to send out REPLY to remote peer because no local REQUESTS are ahead of it
     * @param argument  thread_arg determines which row of requestQueue to monitor
     * @return
     */
    void * request_queue_notifier( void* argument );

    //MessageBuffer's are used as ring buffers inside the node controller for
    //passing messages between its threads; the number of rows nrows is
    //determined by the needs of the threads acting on it
    template< size_t nrows, size_t buffsize>
    class MessageBuffer;



    typedef int32_t logical_time;

    /**
     * implements Lamport's logical clock
     */
    class LogicalClock
    {
        logical_time time;
        pthread_mutex_t * const time_guard;

    public:
        LogicalClock( );
        ~LogicalClock( );

        enum
        {
            UNITIALIZED = -1
        };

        /**
         * acts on logical_time type, not Message, so pass in the
         * timestamp field of the message and it will be updated via value-result
         * argument type. if msg_time is UNINITIALIZED then return current
         * clock value and update clock, but if msg_time is not UNINITIALIZED
         * then compare msg_time with current clock value and update clock value
         * if necessary
         * @param msg_time  the timestamp field of Message structure
         */
        void operator()( logical_time& msg_time );

        /**
         * resets msg_time to UNINTIALIZED so that the associated Message can be
         * converted from REQUEST to REPLY.  Once timestamp is reset, use
         * the call operator defined above to get correct value for timestamp
         * @param msg_time  timestamp field of Message structure
         */
        void clear_timestamp( logical_time& msg_time );
    };

    struct localMessage
    {
        //cliaddr and clilen obtained from recvfrom call in localMonitor thread
        //and then used by localSender to send messages back to the particular
        //local client
        struct sockaddr_un cliaddr;
        socklen_t clilen;

        //PID of local client
        pid_t localPID;


    };

    enum class MESSAGE_T : uint32_t
    {
        REQUEST, REPLY

    };


    /**
     * message structure passed back and forth between remote-peer
     * node controllers
     */
    struct Message
    {

        //each message sent between node controllers is either REQUEST or REPLY
        MESSAGE_T type;

        //if type if REQUEST then this indicates that node sending message
        //is requesting (Action act, subAction sub_act) e.g (CONSUME, FLAVOR_1);
        //if type is REPLY then this message is replying to a peer node that
        //previously requested (act, sub_act)
        Action act;

        //which "flavor" to produce or consume where flavor can represent any
        //object that client wishes to consume or produce
        subAction sub_act;

        //node_ID is overloaded so that when a message is sent it indicates
        //the receiving node (ME IF SENDING TO ME) but when a message arrives, the node_ID is
        //overwritten to indicate the sending node (PERSON THAT SENT TO ME)
        uint32_t node_ID;

        //implemented as Lamport's logical clock
        logical_time time_stamp;

        //bit array where each bit indicates whether that node sent a REPLY
        uint32_t rcvdReplies;

        //if message is associated with a local client request then this pointer
        //will be non-NULL and will point to a structure needed to communicate
        //with local client
        localMessage* loc_msg;


        //both lt and eq operators induce total ordering on message in
        //requestQueue
        bool operator<(const Message&) const;
        bool operator==(const Message&) const;


        friend std::ostream& operator<<( std::ostream&, const Message&);



    };

    /**
     * updates the bit array Message.rcvdReplies to flip the bit on for the
     * argument node_ID
     * @param node_ID the ID of the node that sent a REPLY message
     * @param reply_array bit array stored in Message structure
     */
    void update_rcvd_replies_array( unsigned node_ID, unsigned& reply_array );



    namespace remoteSender
    {

        struct thread_arg
        {
            static constexpr size_t incoming_NROWS = 1;
            static constexpr size_t incoming_BUFFSIZE = MSG_BUFF_SIZE;
            //ring buffer that contains outgoing REQUEST and REPLY messages
            //that were produced by incoming_msg_handler thread and
            //outgoing_reply_handler thread
            MessageBuffer<incoming_NROWS, incoming_BUFFSIZE> *
            in_buffer;
        };
    }


    namespace localSender
    {

        struct thread_arg
        {

            //same act,sub_act pair passed to the associated queue_notifier
            //so that the queue_notifier for (CONSUME,FLAVOR_1) indicates that
            //a local client reached head of queue, it will enqueue a message
            //on in_buffer of localSender for (CONSUME,Flavor_1). This setup
            //allows local clients to produce and consume the same object
            //concurrently but prevents 2 producers / 2 consumers from
            //producing / consuming same object concurrently
            Action act;
            subAction sub_act;



            static constexpr size_t incoming_NROWS = 2 * NUMFLAVORS;
            static constexpr size_t incoming_BUFFSIZE = MSG_BUFF_SIZE;

            MessageBuffer<incoming_NROWS, incoming_BUFFSIZE> *
                    in_buffer = nullptr;
        };
    }



    namespace outgoingReplyHandler
    {

        struct thread_arg
        {
            //out_buffer is the ring buffer that contains all outgoing messages
            //to remote peers
            static constexpr size_t outPeer_NROWS = 1;
            static constexpr size_t outPeer_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<outPeer_NROWS, outPeer_BUFFSIZE> *
            out_buffer;


            //in_buffer is the ring buffer that all queue_notifier threads will
            //enqueue messages into that need to be REPLYied too
            static constexpr size_t incoming_NROWS = 1;
            static constexpr size_t incoming_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<incoming_NROWS, incoming_BUFFSIZE> *
            in_buffer;



        };

    }

    namespace incomingMsgHandler
    {

        struct thread_arg
        {
            static constexpr size_t outPeer_NROWS = 1;
            static constexpr size_t outPeer_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<outPeer_NROWS, outPeer_BUFFSIZE> *
            out_buffer;

            static constexpr size_t incoming_NROWS = 1;
            static constexpr size_t incoming_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<incoming_NROWS, incoming_BUFFSIZE> *
            in_buffer;


            //buffer layout flavor0|reply0|flavor1|reply1 ...
            static constexpr size_t rqstQ_NROWS = 2 * NUMFLAVORS;
            static constexpr size_t rqstQ_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<rqstQ_NROWS, rqstQ_BUFFSIZE> *
            rqstQ_buffer;




        };
    }


    namespace remoteMonitor
    {

        enum class comm_actions
        {
            ACCEPT, CONNECT
        };

        struct connect_info
        {
            const char * IPaddr; // = nullptr;
            unsigned short port;


        };

        struct accept_info
        {
            //listen_socket and accept_lock instantiated by main thread
            const Socket& listen_socket;
            pthread_mutex_t * const accept_lock;

            accept_info( const Socket&, pthread_mutex_t* );
        };

        struct thread_arg
        {
            unsigned peerID;
            comm_actions action;
            connect_info * c_data;
            accept_info * a_data;
            pthread_barrier_t *comm_barrier;
            static constexpr size_t NROWS = 1;
            static constexpr size_t BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<NROWS, BUFFSIZE> * out_buffer;



        };
    }

    namespace localMonitor
    {

        struct accept_info
        {
            const Socket& listen_socket;

            pthread_mutex_t * const accept_lock;

            accept_info( const Socket&, pthread_mutex_t* );
        };

        struct thread_arg
        {
            accept_info * a_data;
            static constexpr size_t NROWS = 1;
            static constexpr size_t BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<NROWS, BUFFSIZE> * out_buffer;

        };
    }

    namespace requestQueue_manager
    {

        //this thread will switch on MESSAGE_T and will either handle
        //REQUEST or REPLY messages so all threads are capable of handling
        //both but whichrow will determine whether they handle rqst or reply

        struct thread_arg
        {
            //1 buffer for both P/C for every flavor and 1 buffer for REPLYs
            //for every flavor alternating so whichrow == 1 then that thread
            //handles REPLIES for flavor_0. so odd whichrow's are REPLIES for
            //flavor whichrow-1 assuming that layout of
            //buffers as flavor|reply|flavor|reply ...


            int whichrow;
            static constexpr size_t rqst_NROWS = 2 * NUMFLAVORS;
            static constexpr size_t rqst_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<rqst_NROWS, rqst_BUFFSIZE> * rqstQ_buffer;
            //same rqstQ buffer that incomg_msg_handler thread operates on

        };

    }

    namespace requestQueue_notifier
    {
        //monitors queue for local clients at head of queue and sends
        //immediate REPLY msgs when local clients not in front of queue

        struct thread_arg
        {
            //act and sub_act used to identify which row of queue to monitor
            Action act;
            subAction sub_act;

            static constexpr size_t outLocal_NROWS = 2 * NUMFLAVORS;
            static constexpr size_t outLocal_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<outLocal_NROWS, outLocal_BUFFSIZE> * outLocal_buffer;


            static constexpr size_t outPeer_NROWS = 1;
            static constexpr size_t outPeer_BUFFSIZE = MSG_BUFF_SIZE;
            MessageBuffer<outPeer_NROWS, outPeer_BUFFSIZE> * outPeer_buffer;


        };

    }

    class Ricart_RequestQueue
    {
        //when messages are removed from these queues their dynamic memory
        //must be freed.
        std::list<Message> queues[2 * NUMFLAVORS];
        const size_t queue_size;

        //this array holds the timestamps of the most recent local client
        //to enter critical section for each queue--so if a request shows up
        //in a queue with timestamp strictly less than this value, it can be immediately
        //removed and it may indicate ERROR. The timestamp may be equal--OKAY
        logical_time most_recent_finished_timestamps[2 * NUMFLAVORS] = { LogicalClock::UNITIALIZED };

        bool Qhead_changed[2 * NUMFLAVORS] = { false };

        //no need for Qhead_lock because queue_monitor thread will
        //wait on condvar until head has changed then once its done
        //it will reset Qhead_changed variable while it still owns queue_lock
        //and Qhead_changed can only be set TRUE if either enqueue_request or
        //remove_finished_peer owns queue_lock so Qhead_changed is accessed atomically

        pthread_cond_t* Qhead_condvar[2 * NUMFLAVORS];
        //pthread_mutex_t* Qhead_lock[2 * NUMFLAVORS];



        //        bool client_finished[2 * NUMFLAVORS] = { false };
        //        pthread_mutex_t* client_lock[2 * NUMFLAVORS];
        //        pthread_cond_t* client_condvar[2 * NUMFLAVORS];

        pthread_barrier_t* client_barrier[2 * NUMFLAVORS];


        pthread_mutex_t* queue_lock[2 * NUMFLAVORS];
        pthread_cond_t* queue_condvar[4 * NUMFLAVORS];

        enum class OPTION
        {
            CONSUME, PRODUCE
        };

        //only need this function for indices that range to 4*NUMFLAVORS
        //otherwise just use compute_queue_index

        size_t compute_controlvar_index( Action act, subAction sub_act, OPTION opt );
        size_t compute_queue_index( Action act, subAction sub_act );

        //these functions must be code-locked so they're private
        //because I will ensure these are called atomically; these functions
        //are similar to space count and object count for ring buffers

        bool is_empty( Action act, subAction sub_act );

        bool is_full( Action act, subAction sub_act );

        bool has_Qhead_changed( Action act, subAction sub_act );
    public:

        Ricart_RequestQueue( size_t queue_size );


        void print_queue_to_file( const char* filename );

        size_t get_size( Action act, subAction sub_act, bool NONBLOCK = false );

        virtual ~Ricart_RequestQueue( );

        //is_local_finished is called within the "queue_monitor"
        //thread to know when its okay to pop deferred list

        void wait_local_client_finished( Action act, subAction sub_act );

        //notify_local_finished is called within "localComm_sender"
        //to tell the queue_monitor that it is okay to pop deferred list

        void notify_local_client_finished( Action act, subAction sub_act );


        //behaves like producer

        void enqueue_request( Action act, subAction sub_act, Message rqst_msg );

        //behaves like consumer but not linear

        void remove_finished_peers( Action act, subAction sub_act, Message reply_msg );

        //requires mutex access but doesn't need to execute unless
        //something has changed at front of queue
        //like when queue goes from empty to not empty OR
        //when finished peers have been removed OR
        //deferred list is popped
        //BUT do not need to check when local client popped off head
        //because even if my request is next, I can't execute until
        //current request that was just popped finishes and I'll
        //know when that finishes by popping deferred list

        static bool is_local_client_request( const Message& msg );




        //peek_at_head will get mutex access to head of the given queue
        //and apply function func on that element and return its result

        template<typename Func>
        decltype( auto ) peek_at_head( Action act,
                                       subAction sub_act, Func func )
        {
            size_t Qindx;
            size_t cons_indx;

            Qindx = compute_queue_index( act, sub_act );
            cons_indx = compute_controlvar_index( act, sub_act, OPTION::CONSUME );

            if ( pthread_mutex_lock( queue_lock[Qindx] ) != 0 ) {
                throw std::runtime_error( "peek_at_head" );
            }

            //
            //Qhead_condvar  !has_Qhead_changed( act, sub_act ) ||
            while ( is_empty( act, sub_act ) ) {

                if ( pthread_cond_wait( Qhead_condvar[Qindx],
                                        queue_lock[Qindx] ) != 0 ) {
                    throw std::runtime_error( "peek_at_head" );
                }

            }

            auto val = func( queues[Qindx].front( ) );


            if ( pthread_mutex_unlock( queue_lock[Qindx] ) != 0 ) {
                throw std::runtime_error( "peek_at_head" );
            }


            return val;


        }


        void pop_local_client_from_head( Action act, subAction sub_act );

        void pop_deferred_reply_list( Action act, subAction sub_act,
                                      std::list<Message>& deferred_list );


    };


    //create regular Message in host, store it in MessageContainer
    //then pass container to this function to convert fields to network byte order
    //then send bytes over network from bytestream member
    void host2network_msg( const Message&, char* );

    void network2host_msg( Message&, const char* );

    void client2internal_msg( Message&,
                              const clientUtils::Message&,
                              struct sockaddr_un,
                              socklen_t );


    void internal2client_msg( const Message&,
                              clientUtils::Message&,
                              clientUtils::MESSAGE_T );


}



#endif /* NODE_CONTROLLER_HPP */

