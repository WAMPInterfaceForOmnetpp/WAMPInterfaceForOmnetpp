// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef __INET_LIVERECORDER_H
#define __INET_LIVERECORDER_H

#include <string>
#include "inet/common/INETDefs.h"
#include <thread>
#include <stdio.h>
#include "Settings.h"
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_publish_options.hpp>
#include <boost/program_options.hpp>
#include <sstream>

namespace wampinterfaceforomnetpp {

/**
 * Listener for sending events via WAMP to the router.
 * @param topic     The router topic the event is published to. See the crossbar.io documentation for details about topics.
 */
template<char const *topic>
class INET_API LiveRecorder: public cResultRecorder /*public RPCallable<LiveRecorder>*/
{
protected:
    /**
     * collects the signal and sends the respective event to the WAMP router.
     *
     * @param val   The value that was emitted
     */
    virtual void collect(std::string val);

    /**
     * Each function receives events of a special data type and forwards it as a string to the collect function.
     *
     * @param prev      The result filter
     * @param t         The simulation time the event occurs
     * @param b         The value that was given (l, d, v, s, obj respectively)
     * @param details   An optional object that can be emitted to give details about the event (not used here).
     */
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b,
            cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l,
            cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
            unsigned long l, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d,
            cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
            const SimTime& v, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
            const char *s, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
            cObject *obj, cObject *details) override;
};

template<const char* topic>
void LiveRecorder<topic>::collect(std::string value) {
    try {
        std::unique_ptr<Settings> settings(new Settings);

        boost::asio::io_service io;
        bool debug = settings->debug();

        auto transport = std::make_shared<autobahn::wamp_tcp_transport>(io,
                settings->rawsocket_endpoint(), debug);

        // create a WAMP session that talks WAMP-RawSocket over TCP

        auto session = std::make_shared<autobahn::wamp_session>(io, debug);

        transport->attach(
                std::static_pointer_cast<autobahn::wamp_transport_handler>(
                        session));

        // Make sure the continuation futures we use do not run out of scope prematurely.
        // Since we are only using one thread here this can cause the io service to block
        // as a future generated by a continuation will block waiting for its promise to be
        // fulfilled when it goes out of scope. This would prevent the session from receiving
        // responses from the router.
        boost::future<void> connect_future;
        boost::future<void> start_future;
        boost::future<void> join_future;
        boost::future<void> leave_future;
        boost::future<void> stop_future;

        connect_future =
                transport->connect().then(
                        [&](boost::future<void> connected) {
                            try {
                                connected.get();
                            } catch (const std::exception& e) {
                                std::cerr << e.what() << std::endl;
                                return;
                            }

                            std::cerr << "transport connected" << std::endl;

                            start_future = session->start().then([&](boost::future<void> started) {
                                        try {
                                            started.get();
                                        } catch (const std::exception& e) {
                                            std::cerr << e.what() << std::endl;
                                            io.stop();
                                            return;
                                        }

                                        std::cerr << "session started" << std::endl;

                                        join_future = session->join(settings->realm()).then([&](boost::future<uint64_t> joined) {
                                                    try {
                                                        std::cerr << "joined realm: " << joined.get() << std::endl;
                                                    } catch (const std::exception& e) {
                                                        std::cerr << e.what() << std::endl;
                                                        io.stop();
                                                        return;
                                                    }

                                                    std::tuple<std::string, std::string> arguments = std::make_tuple(simTime().str(), std::string(value));
                                                    session->publish(topic, arguments);

                                                    std::cerr << "event published" << std::endl;

                                                    leave_future = session->leave().then([&](boost::future<std::string> reason) {
                                                                try {
                                                                    std::cerr << "left session (" << reason.get() << ")" << std::endl;
                                                                } catch (const std::exception& e) {
                                                                    std::cerr << "failed to leave session: " << e.what() << std::endl;
                                                                    io.stop();
                                                                    return;
                                                                }

                                                                stop_future = session->stop().then([&](boost::future<void> stopped) {
                                                                            std::cerr << "stopped session" << std::endl;
                                                                            io.stop();
                                                                            return;
                                                                        });
                                                            });
                                                });
                                    });
                        });

        std::cerr << "starting io service" << std::endl;
        io.run();
        std::cerr << "stopped io service" << std::endl;

        transport->detach();
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        bool b, cObject* DETAILS_ARG) {
    collect(b ? "true" : "false");
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        long l, cObject* DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        unsigned long l, cObject* DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        double d, cObject* DETAILS_ARG) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        const SimTime& v, cObject* DETAILS_ARG) {
    collect(v.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        const char *s, cObject* DETAILS_ARG) {
    collect(s);
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        cObject *obj, cObject* DETAILS_ARG) {
    collect(obj->getFullPath());
}

} // namespace wampinterfaceforomnetpp

#endif
