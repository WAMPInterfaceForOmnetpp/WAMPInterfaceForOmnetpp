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

package wampinterfaceforomnetpp;

module SimulationCallee
{
    parameters:
        // Parameter that can be set to true remotely to end the simulation.
        bool stopSimulation = default(false);
        
     	// Parameter that defines under which name the setParameter function can be found on the WAMP router.
        string setParameterPath = default("com.examples.functions.setParameter");
        
     	// Parameter that defines under which name the getParameter function can be found on the WAMP router.
		string getParameterPath = default("com.examples.functions.getParameter");
		
     	// Parameter that defines under which name the getParameterNamesPath function can be found on the WAMP router.
		string getAllSubmodulesPath = default("com.examples.functions.getAllSubmodules");
		
     	// Parameter that defines under which name the getParameterNamesPath function can be found on the WAMP router.
		string getParameterNamesPath = default("com.examples.functions.getParameterNames");
		
     	// Parameter to determine where the callee module can be found.
		string modulePath = default("Tictoc.callee");
		
		// The interval in which the setParameter function is called.
		double setParameterInterval = default(0.1);
		
        @class(SimulationCallee);
}