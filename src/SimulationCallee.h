//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef SIMULATIONCALLEE_H_
#define SIMULATIONCALLEE_H_

#include <omnetpp.h>
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_publish_options.hpp>
#include <thread>
#include "ParameterMsg.h"
#include <boost/lockfree/queue.hpp>
#include "Settings.h"

using namespace omnetpp;

namespace wampinterfaceforomnetpp {

/**
 * Module that can change any parameter in the simulation remotely.
 */
class SimulationCallee: public cSimpleModule {
private:
    /**
     * Variable that defines under which name the setParameter function can be found on the WAMP router.
     */
    std::string setParameterPath;

    /**
     * Variable that defines under which name the getParameter function can be found on the WAMP router.
     */
    std::string getParameterPath;

    /**
     * Variable that defines under which name the getAllSubpath function can be found on the WAMP router.
     */
    std::string getAllSubmodulesPath;

    /**
     * Variable that defines under which name the getParameterNamesPath function can be found on the WAMP router.
     */
    std::string getParameterNamesPath;

    /**
     * Thread that is used to register the function to the crossbar.io router
     */
    std::thread registeredFunctions;

    /**
     * io_service, that established a tcp connection to the router.
     */
    boost::asio::io_service io;

    /**
     * The time between to setParameters Events, that are used to change parameters.
     */
    double interval;

    /**
     * Boolean variable that indicates whether the functions where registered once.
     */
    bool wasConnected = false;

    /**
     * Defines what happens if the modules parameters change.
     *
     * @param parname   The name of the parameter that changed.
     */
    virtual void handleParameterChange(const char *parname);

    /**
     * Traverses the module path to find all modules where the parameter shall be changed.
     *
     * @param path      Remaining part of the path that is not already part of the module path.
     * @param mod       Module, that submodules are looked at.
     *                  Nullptr if the function is called with the whole path.
     * @param param     The parameter that shall be changed
     * @param value     The value to that the parameter shall be changed.
     */
    void traverseSetPath(std::string path, cModule* mod, std::string param,
            std::string value);

    /**
     * Traverses the get path to return all needed parameter values.
     * A parameter array is returned, if an array of modules was addressed.
     *
     * @param path      Remaining part of the path that is not already part of the module path.
     * @param mod       Module, that submodules are looked at.
     *                  Nullptr if the function is called with the whole path.
     * @param param     The parameter that shall be read
     * @param list      The list where the results shall be stored.
     */
    template<typename T>
    static void traverseGetPath(std::string path, cModule* mod,
            std::string param, std::list<T>* list);

    /**
     * Checks whether the parameter and/or the value is volatile
     * or an expression calls setParameterByDataType accordingly.
     *
     * @param mod       The module where the parameter shall be changed
     * @param paramName The parameter that shall be changed
     * @param value     The value the parameter shall get
     */
    void setSingleParameter(cModule* mod, std::string paramName,
            std::string value);
    /**
     * Sets the given parameter in the given module to the given value.
     *
     * @param mod       The module where the parameter shall be changed
     * @param paramName The parameter that shall be changed
     * @param value     The value the parameter shall get
     */
    void setParameterByDataType(cModule* mod, std::string paramName,
            std::string value);

    /**
     * Returns the value of the given parameter in the given module.
     *
     * @param mod       The module for which the parameter shall be returned
     * @param paramName The parameter thats value shall be read.
     */
    template<typename T>
    static T getSingleParameter(cModule* mod, std::string paramName);

public:

    /**
     * Boolean variable to decide whether the simulation shall stop.
     */
    static bool waitForStop;

    /**
     * String to determine where the callee module can be found.
     */
    static std::string calleeModulePath;

    /**
     * Thread- safe boost queue to hold all parameters that shall be changed in the next Simulation Callee event.
     */
    static boost::lockfree::queue<ParameterMsg*> ParametersToSet;

    /**
     * Defines the static function that is registered at the crossbar.io server to be called
     * to change any parameter of the simulation.
     *
     * @param invocation   The parameters given from the caller.
     *                      Normally the name of the module, the name of the parameter
     *                      and the new value of the parameter.
     */
    static void setParameter(autobahn::wamp_invocation invocation);

    /**
     * Function that is registeres at the crossbar.io router to be called to read any parameter of the simulation.
     *
     * @param invocation    The parameters given from the caller. Normally the name of the module and the name of the parameter.
     */
    static void getParameter(autobahn::wamp_invocation invocation);

    /**
     * Function that is registered at the crossbar.io router to be called to get all submodule names and types of a certain module.
     *
     * @param invocation    The arguments given to the function.
     *                      A string defining the path to the module thats submodule names shall be returned.
     */
    static void getSubmodules(autobahn::wamp_invocation invocation);

    /**
     * Function that is registered at the crossbar.io router to be called to get all parameter names of a module.
     *
     * @param invocation    The arguments given to the function. A string defining the path to the module.
     */
    static void getModuleParameterNames(autobahn::wamp_invocation invocation);

    /**
     * Initializing the thread.
     */
    void initialize(int stage);

    /**
     * Finalizing the thread.
     */
    void finish();

    /**
     * Registers the methods at the crossbar.io router
     */
    void registerMethods();

    /**
     * Loop to make sure it is tried to register the functions until it is done.
     * Without it, only one trial is made and the program fails if the server is not yet established.
     */
    void tryToRegister();

    /**
     * Function to handle all incoming messages.
     *
     * @param msg   The incoming message
     */
    void handleMessage(cMessage *msg);
};

} /* namespace wampinterfaceforomnetpp */

#endif /* SIMULATIONCALLEE_H_ */
