#include "Settings.h"

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

Settings::Settings() :
        m_debug(false), m_realm(DEFAULT_REALM), m_rawsocket_endpoint(
                ROUTER_IP_ADDRESS, DEFAULT_RAWSOCKET_PORT)
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
                , m_uds_endpoint(DEFAULT_UDS_PATH)
#endif
{
}

bool Settings::debug() const {
    return m_debug;
}

const std::string& Settings::realm() const {
    return m_realm;
}

const boost::asio::ip::tcp::endpoint& Settings::rawsocket_endpoint() const {
    return m_rawsocket_endpoint;
}

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
const boost::asio::local::stream_protocol::endpoint& Settings::uds_endpoint() const {
    return m_uds_endpoint;
}
#endif

void Settings::set_debug(bool value) {
    m_debug = value;
}

void Settings::set_realm(const std::string& realm) {
    m_realm = realm;
}

void Settings::set_rawsocket_endpoint(const std::string& ip_address,
        uint16_t port) {
    m_rawsocket_endpoint = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(ip_address), port);
}

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
void Settings::set_uds_endpoint(const std::string& path) {
    m_uds_endpoint = boost::asio::local::stream_protocol::endpoint(path);
}
#endif 
