#ifndef SETTINGS_H
#define SETTINGS_H

#include <boost/asio/ip/tcp.hpp>
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#    include <boost/asio/local/stream_protocol.hpp>
#endif 
#include <cstdint>
#include <string>

class Settings {
public:
    Settings();

    bool debug() const;
    const std::string& realm() const;
    const boost::asio::ip::tcp::endpoint& rawsocket_endpoint() const;

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    void set_uds_endpoint(const std::string& path);
    const boost::asio::local::stream_protocol::endpoint& uds_endpoint() const;
#endif

    void set_debug(bool enabled);
    void set_realm(const std::string& realm);
    void set_rawsocket_endpoint(const std::string& ip_address, uint16_t port);
private:
    bool m_debug;
    std::string m_realm;
    boost::asio::ip::tcp::endpoint m_rawsocket_endpoint;
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    boost::asio::local::stream_protocol::endpoint m_uds_endpoint;
#endif
};

#endif
