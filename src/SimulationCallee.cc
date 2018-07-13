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

#include "SimulationCallee.h"

namespace wampinterfaceforomnetpp {

Define_Module(SimulationCallee);

// Initialize class variables
std::string SimulationCallee::calleeModulePath = "Tictoc.callee";

boost::lockfree::queue<ParameterMsg*> SimulationCallee::ParametersToSet{100};

void SimulationCallee::getSubmodules(autobahn::wamp_invocation invocation) {
    std::string modulePath = invocation->argument<std::string>(0);
    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();
    std::list<std::tuple<std::string, std::string>>* modules = new std::list<
            std::tuple<std::string, std::string>>();

    cModule* module;
    if (modulePath == "") {
        module = sim->getSystemModule();
        std::tuple<std::string, std::string> myTuple = std::make_tuple(
                module->getFullName(), module->getModuleType()->str());
        modules->push_back(myTuple);
        invocation->result(*modules);
    } else {
        module = sim->getModuleByPath(modulePath.c_str());
        if (module != nullptr) {
            for (cModule::SubmoduleIterator it(module); !it.end(); ++it) {
                modules->push_back(
                        std::make_tuple((*it)->getFullName(),
                                (*it)->getModuleType()->str()));
            }
            invocation->result(*modules);
        } else {
            invocation->result("Module not found");
        }
    }
    delete modules;
}

void SimulationCallee::getModuleParameterNames(
        autobahn::wamp_invocation invocation) {
    std::string modulePath = invocation->argument<std::string>(0);
    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();
    std::list<std::tuple<std::string, std::string, std::string>>* parameters = new std::list<
            std::tuple<std::string, std::string, std::string>>();

    cModule* module;
    if (modulePath == "") {
        module = sim->getSystemModule();
    } else {
        module = sim->getModuleByPath(modulePath.c_str());
    }

    if (module != nullptr) {
        for (int i = 0; i < module->getNumParams(); ++i) {
            std::string name = module->par(i).getFullName();
            std::string type;
            type = module->par(i).getTypeName(module->par(i).getType());
            if (module->par(i).isVolatile()) {
                type = "volatile " + type;
            }
            std::string unit("");
            const char * unit_or_null = module->par(i).getUnit();
            if (unit_or_null) {
                unit = unit_or_null;
            }
            parameters->push_back(std::make_tuple(name, type, unit));

        }
        invocation->result(*parameters);
    } else
        invocation->result(std::make_tuple("Module not found"));
    delete parameters;
}

void SimulationCallee::setParameter(autobahn::wamp_invocation invocation) {
    std::string module = invocation->argument<std::string>(0);
    std::string paramName = invocation->argument<std::string>(1);
    std::string value = invocation->argument<std::string>(2);

    ParameterMsg* msg = new ParameterMsg();
    msg->moduleName = module;
    msg->paramName = paramName;
    msg->value = value;

    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();
    cSimpleModule *mod = (cSimpleModule*) (sim->getModuleByPath(
            module.c_str()));
    if (mod != nullptr) {
        ParametersToSet.push(msg);
        invocation->result(std::make_tuple("\n"));
    } else
        invocation->result(std::make_tuple("Module not found"));
}

void SimulationCallee::getParameter(autobahn::wamp_invocation invocation) {
    std::string module = invocation->argument<std::string>(0);
    std::string paramName = invocation->argument<std::string>(1);

    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();
    cSimpleModule *mod = (cSimpleModule*) (sim->getModuleByPath(module.c_str()));
    if (mod != nullptr) {
        if (mod->hasPar(paramName.c_str())) {
            if (mod->par(paramName.c_str()).isExpression()) {
                std::string res =
                        mod->par(paramName.c_str()).getExpression()->str();
                res = "=" + res;
                invocation->result(std::make_tuple(res));
            } else {
                omnetpp::cPar::Type type =
                        mod->par(paramName.c_str()).getType();
                if (type == 'D') {
                    std::list<double> results_d;
                    traverseGetPath(module, nullptr, paramName, &results_d);
                    invocation->result(results_d);
                } else if (type == 'S') {
                    std::list<const char*> results_s;
                    traverseGetPath(module, nullptr, paramName, &results_s);
                    invocation->result(results_s);
                } else if (type == 'L') {
                    std::list<long> results_l;
                    traverseGetPath(module, nullptr, paramName, &results_l);
                    invocation->result(results_l);
                } else if (type == 'B') {
                    std::list<bool> results_b;
                    traverseGetPath(module, nullptr, paramName, &results_b);
                    invocation->result(results_b);
                }
            }
        } else {
            invocation->result(std::make_tuple("Parameter not found"));
        }
    } else {
        invocation->result(std::make_tuple("Module not found"));
    }
}

template<typename T>
void SimulationCallee::traverseGetPath(std::string path, cModule* mod,
        std::string param, std::list<T>* list) {
    std::string modPath = "";
    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();

    if (mod != nullptr)
        modPath = mod->getFullPath();

    if (path == "") {
        // Found module, so we set the respective parameter
        list->push_back(getSingleParameter<T>(mod, param));
    } else {
        int pos = path.find(".");
        std::string firstPart;

        // check if first part is last one in path string
        if (pos == std::string::npos)
            firstPart = path;
        else
            firstPart = path.substr(0, pos);

        if (firstPart.find("[*]") != std::string::npos) {
            // call this function for all modules that are part of the array
            std::string arrayName = firstPart.substr(0, firstPart.find("[*]"));
            std::string pattern = modPath + "." + arrayName;
            for (cModule::SubmoduleIterator it(mod); !it.end(); ++it) {
                cModule *submodule = *it;
                if (submodule->getFullPath().find(pattern) == 0) {
                    int newPos = path.find(".");
                    std::string subPath;
                    if (newPos == std::string::npos)
                        subPath = "";
                    else
                        subPath = path.substr(newPos + 1, path.size());
                    traverseGetPath(subPath, submodule, param, list);
                }
            }
        } else {
            // Found a new path part that is not an array.

            // Check if it is the first path part
            if (mod != nullptr)
                modPath += "." + firstPart;
            else
                modPath = firstPart;

            // Call this function again with new parameters.
            cModule *ParamModule = sim->getModuleByPath(modPath.c_str());
            std::string subPath;
            if (pos == std::string::npos)
                subPath = "";
            else
                subPath = path.substr(pos + 1, path.size());

            if (ParamModule != nullptr)
                traverseGetPath(subPath, ParamModule, param, list);

        }
    }
}

template<typename T>
T SimulationCallee::getSingleParameter(cModule* mod, std::string paramName) {
    if (mod->hasPar(paramName.c_str())) {
        T result;
        result = mod->par(paramName.c_str());
        return result;
    } else
        return 0;
}

void SimulationCallee::handleParameterChange(const char *parname) {
    setParameterPath = par("setParameterPath").stringValue();
    getParameterPath = par("getParameterPath").stringValue();
    getAllSubmodulesPath = par("getAllSubmodulesPath").stringValue();
    getParameterNamesPath = par("getParameterNamesPath").stringValue();
    interval = par("setParameterInterval").doubleValue();
    SimulationCallee::calleeModulePath = par("modulePath").stringValue();
    if (par("stopSimulation").boolValue() == true) {
        endSimulation();
    }
}

void SimulationCallee::initialize(int stage) {
    cSimpleModule::initialize(stage);

    setParameterPath = par("setParameterPath").stringValue();
    getParameterPath = par("getParameterPath").stringValue();
    getAllSubmodulesPath = par("getAllSubmodulesPath").stringValue();
    getParameterNamesPath = par("getParameterNamesPath").stringValue();

    interval = par("setParameterInterval").doubleValue();

    cMessage* msg = new cMessage("interval");
    scheduleAt(simTime() + interval, msg);

    wampConnection.start([&](std::shared_ptr<autobahn::wamp_session> session){
        std::vector<boost::future<autobahn::wamp_registration>> registrations;
        registrations.push_back(session->provide(SimulationCallee::setParameterPath, &(setParameter)));
        registrations.push_back(session->provide(SimulationCallee::getParameterPath, &(getParameter)));
        registrations.push_back(session->provide(SimulationCallee::getAllSubmodulesPath, &(getSubmodules)));
        registrations.push_back(session->provide(SimulationCallee::getParameterNamesPath, &(getModuleParameterNames)));

        for(auto& registration : registrations) {
            try {
                registration.get();
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
        }

        return true;
    });
}

void SimulationCallee::finish() {
    wampConnection.stop();
    wampConnection.join();
}

void SimulationCallee::handleMessage(cMessage *msg) {

    while(!ParametersToSet.empty()){
        ParameterMsg *myMsg;
        ParametersToSet.pop(myMsg);
        std::string wholePath = myMsg->moduleName;
        traverseSetPath(wholePath, nullptr, myMsg->paramName, myMsg->value);
    }
    scheduleAt(simTime() + interval, msg);
}

void SimulationCallee::traverseSetPath(std::string path, cModule* mod,
        std::string param, std::string value) {
    std::string modPath = "";
    omnetpp::cSimulation *sim = omnetpp::cSimulation::getActiveSimulation();

    if (mod != nullptr) {
        modPath = mod->getFullPath();
    }

    if (path == "") {
        // Found module, so we set the respective parameter
        setSingleParameter(mod, param, value);
    } else {
        int pos = path.find(".");
        std::string firstPart;

        // check if first part is last one in path string
        if (pos == std::string::npos)
            firstPart = path;
        else
            firstPart = path.substr(0, pos);

        if (firstPart == "*") {
            // check if the first module was given as star
            if (mod == nullptr) {
                mod = sim->getSystemModule();
                int newPos = path.find(".");
                std::string subPath;
                if (newPos == std::string::npos)
                    subPath = "";
                else
                    subPath = path.substr(newPos + 1, path.size());
                traverseSetPath(subPath, mod, param, value);
            } else {
                // call this function for all submodules
                for (cModule::SubmoduleIterator it(mod); !it.end(); ++it) {
                    cModule *submodule = *it;
                    int newPos = path.find(".");
                    std::string subPath;
                    if (newPos == std::string::npos)
                        subPath = "";
                    else
                        subPath = path.substr(newPos + 1, path.size());
                    traverseSetPath(subPath, submodule, param, value);
                }
            }
        } else if (firstPart.find("[*]") != std::string::npos) {
            // call this function for all modules that are part of the array
            std::string arrayName = firstPart.substr(0, firstPart.find("[*]"));
            std::string pattern = modPath + "." + arrayName;
            for (cModule::SubmoduleIterator it(mod); !it.end(); ++it) {
                cModule *submodule = *it;
                if (submodule->getFullPath().find(pattern) == 0) {
                    int newPos = path.find(".");
                    std::string subPath;
                    if (newPos == std::string::npos)
                        subPath = "";
                    else
                        subPath = path.substr(newPos + 1, path.size());
                    traverseSetPath(subPath, submodule, param, value);
                }
            }
        } else {
            // Found a new path part that is not a star or an array.

            // Check if it is the first path part
            if (mod != nullptr)
                modPath += "." + firstPart;
            else
                modPath = firstPart;

            // Call this function again with new parameters.
            cModule *ParamModule = sim->getModuleByPath(modPath.c_str());
            std::string subPath;
            if (pos == std::string::npos)
                subPath = "";
            else
                subPath = path.substr(pos + 1, path.size());
            if (ParamModule != nullptr)
                traverseSetPath(subPath, ParamModule, param, value);
        }
    }
}

void SimulationCallee::setSingleParameter(cModule* mod, std::string paramName,
        std::string val) {
    std::string value = val;
    bool valueIsExpression;
    if (value.find("=") == 0) {
        valueIsExpression = true;
        value = value.erase(0,1);
    } else
        valueIsExpression = false;

    if (mod->hasPar(paramName.c_str())) {
        if (mod->par(paramName.c_str()).isExpression()) {
            if (!valueIsExpression) {
                setParameterByDataType(mod, paramName, value);
            } else
                mod->par(paramName.c_str()).getExpression()->parse(
                        value.c_str());
        } else if (mod->par(paramName.c_str()).isVolatile()) {
            if (!valueIsExpression) {
                setParameterByDataType(mod, paramName, value);
            } else {
                cExpression* myExp = new cDynamicExpression();
                mod->par(paramName.c_str()).setExpression(myExp, nullptr);
                mod->par(paramName.c_str()).getExpression()->parse(
                                        value.c_str());
            }

        } else {
            setParameterByDataType(mod, paramName, value);
        }
    }
}

void SimulationCallee::setParameterByDataType(cModule* mod,
        std::string paramName, std::string value) {
    omnetpp::cPar::Type type = mod->par(paramName.c_str()).getType();
    switch (type) {
    case 'D':
        mod->par(paramName.c_str()).setDoubleValue(std::stod(value));
        break;
    case 'S':
        mod->par(paramName.c_str()) = value.c_str();
        break;
    case 'L':
        mod->par(paramName.c_str()).setIntValue(std::stol(value));
        break;
    case 'B':
        if (value == "true") {
            mod->par(paramName.c_str()) = true;
        } else if (value == "false") {
            mod->par(paramName.c_str()) = false;
        }
        break;
    default:
        break;

    }
    std::cout << "new " << paramName << " in " << mod->getFullPath() << " is "
            << value << endl;
}

} /* namespace wampinterfaceforomnetpp */
