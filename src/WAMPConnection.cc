#include "WAMPConnection.h"

#include <cstdlib>
#include <boost/asio/ip/address.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace {
const std::string ROUTER_IP_ADDRESS_STRING("127.0.0.1");
const boost::asio::ip::address ROUTER_IP_ADDRESS(
        boost::asio::ip::address::from_string(ROUTER_IP_ADDRESS_STRING));
const std::string DEFAULT_REALM("opplive");
const uint16_t DEFAULT_RAWSOCKET_PORT(9000);
const std::string DEFAULT_UDS_PATH("/tmp/crossbar.sock");
}

WAMPConnection::WAMPConnection() :
             debug(false), running(false), realm(DEFAULT_REALM), rawsocket_endpoint(ROUTER_IP_ADDRESS, DEFAULT_RAWSOCKET_PORT)
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
, uds_endpoint(DEFAULT_UDS_PATH)
#endif
{
}

WAMPConnection::~WAMPConnection() {
    if(running) {
        stop();
        join();
    }
}

void WAMPConnection::start(std::function<bool(std::shared_ptr<autobahn::wamp_session>)> setup) {
    this->setup = setup;
    std::cout << "starting" << std::endl;
    stopPending = false;
    running = true;
    connecter = std::thread(&WAMPConnection::connect, this);
    runner = std::thread(&WAMPConnection::run, this);
}

void WAMPConnection::stop() {
    assert(running);

    stopPending = true;

    boost::future<void> leave_future, stop_future;

    leave_future = session->leave().then([&](boost::future<std::string> reason) {
        try {
            std::cerr << "left session (" << reason.get() << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

        io.stop();
    });
}

void WAMPConnection::join() {
    runner.join();
    connecter.join();
}

void WAMPConnection::exec(std::function<bool(std::shared_ptr<autobahn::wamp_session>)> task) {
    if(!isRunning()) {
        start(task);
    }
    else {
        bool success = task(session);
        if(!success) {
            stop();
        }
    }
}

void WAMPConnection::connect() {
    try {
        boost::future<void> connected, started;

        auto transport = std::make_shared<autobahn::wamp_tcp_transport>(io, rawsocket_endpoint, debug);

        session = std::make_shared<autobahn::wamp_session>(io, debug);

        transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(session));

        while(!stopPending) {
            connected = transport->connect();
            try {
                connected.get();
                std::cout << "transport connected" << std::endl;
                break;
            } catch (const std::system_error & e) {
                if(e.code().value() == boost::asio::error::connection_refused) {
                    std::cerr << "No connection to WAMP router" << std::endl;
                    boost::this_thread::sleep_for(boost::chrono::seconds(2));
                }
                else {
                    std::cerr << e.what() << std::endl;
                    return;
                }
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return;
            }
        }

        if(stopPending) {
            return;
        }

        started = session->start();
        started.get();
        std::cout << "session started" << std::endl;

        auto joined = session->join(realm);
        joined.get();
        std::cout << "joined realm" << std::endl;

        bool success = setup(session);
        if(!success) {
            stop();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void WAMPConnection::run() {
    std::cout << "Running" << std::endl;
    try {
        boost::asio::io_service::work work(io); // avoid leaving run if nothing is left to do
        std::cout << "starting io service .." << std::endl;
        io.run();
        std::cout << "stopped io service" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    running = false;
}

