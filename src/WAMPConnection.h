#ifndef SETTINGS_H
#define SETTINGS_H

#include <boost/asio/ip/tcp.hpp>
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#    include <boost/asio/local/stream_protocol.hpp>
#endif 
#include <cstdint>
#include <string>
#include <thread>
#include <autobahn/autobahn.hpp>

class WAMPConnection {
public:
    WAMPConnection();
    ~WAMPConnection();

    void start(std::function<bool(std::shared_ptr<autobahn::wamp_session>)> setup);
    void exec(std::function<bool(std::shared_ptr<autobahn::wamp_session>)> task);
    void stop();
    void join();

    bool isRunning() {
        return running;
    }

private:
    void run();
    void connect();

    /**
     * io service for establishing a TCP connection to the router.
     */
    boost::asio::io_service io;

    /**
     * Thread for running the io service
     */
    std::thread runner;

    /**
     * Thread for connection
     */
    std::thread connecter;

    /**
     * Function for setup after connection
     */
    std::function<bool(std::shared_ptr<autobahn::wamp_session>)> setup;

    /**
     * WAMP Session
     */
    std::shared_ptr<autobahn::wamp_session> session;

    bool debug;
    bool running;
    bool stopPending;
    std::string realm;
    boost::asio::ip::tcp::endpoint rawsocket_endpoint;
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    boost::asio::local::stream_protocol::endpoint uds_endpoint;
#endif
};

#endif
