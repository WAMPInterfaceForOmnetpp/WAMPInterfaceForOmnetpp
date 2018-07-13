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
#include <thread>
#include <stdio.h>
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_publish_options.hpp>
#include <sstream>
#include "WAMPConnection.h"

namespace wampinterfaceforomnetpp {

/**
 * Listener for sending events via WAMP to the router.
 * @param topic     The router topic the event is published to. See the crossbar.io documentation for details about topics.
 */
template<char const *topic>
class LiveRecorder: public omnetpp::cResultRecorder /*public RPCallable<LiveRecorder>*/
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
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t, bool b,
            omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t, long l,
            omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
            unsigned long l, omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t, double d,
            omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
            const omnetpp::SimTime& v, omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
            const char *s, omnetpp::cObject *details) override;
    virtual void receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
            omnetpp::cObject *obj, omnetpp::cObject *details) override;

private:
    WAMPConnection connection;
};

template<const char* topic>
void LiveRecorder<topic>::collect(std::string value) {
    std::tuple<std::string, std::string> arguments = std::make_tuple(omnetpp::simTime().str(), std::string(value));
    connection.exec([arguments](std::shared_ptr<autobahn::wamp_session> session){
        session->publish(topic, arguments);
        return true;
    });
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        bool b, omnetpp::cObject* DETAILS_ARG) {
    collect(b ? "true" : "false");
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        long l, omnetpp::cObject* DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        unsigned long l, omnetpp::cObject* DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        double d, omnetpp::cObject* DETAILS_ARG) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        const omnetpp::SimTime& v, omnetpp::cObject* DETAILS_ARG) {
    collect(v.str());
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        const char *s, omnetpp::cObject* DETAILS_ARG) {
    collect(s);
}

template<const char* topic>
void LiveRecorder<topic>::receiveSignal(omnetpp::cResultFilter *prev, omnetpp::simtime_t_cref t,
        omnetpp::cObject *obj, omnetpp::cObject* DETAILS_ARG) {
    collect(obj->getFullPath());
}

} // namespace wampinterfaceforomnetpp

#endif
