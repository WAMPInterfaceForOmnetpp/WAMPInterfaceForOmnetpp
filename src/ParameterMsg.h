/*
 * ParameterMsg.h
 *
 *  Created on: 24.11.2017
 *      Author: cjh6384
 */

#ifndef PARAMETERMSG_H_
#define PARAMETERMSG_H_

#include <omnetpp.h>

namespace wampinterfaceforomnetpp {

class ParameterMsg {
public:
    std::string moduleName;
    std::string paramName;
    std::string value;

};

} /* namespace wampinterfaceforomnetpp */

#endif /* PARAMETERMSG_H_ */
