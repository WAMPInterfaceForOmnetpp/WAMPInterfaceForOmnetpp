# Live Monitoring and Remote Control of OMNeT++ Simulations

## Installation Instructions

1. <a href="https://www.youtube.com/watch?v=WwkLIbBmU6Q">Install OMNeT++</a>

2. Install the Boost libraries boost_system, boost_filesystem, boost_thread and boost_program_options.

    ```
    sudo apt-get install libboost-system-dev libboost-thread-dev
    ```

3. Install and start the <a href="https://github.com/crossbario/crossbar">Crossbar.io WAMP router</a>. Make sure it is running before starting any simulation!

    ```
    sudo apt-get install python3-pip
    pip3 install crossbar
    crossbar start
    ```

4. Clone this project and the submodules

    ```
    git clone https://github.com/WAMPInterfaceForOmnetpp/WAMPInterfaceForOmnetpp.git
    cd WAMPInterfaceForOmnetpp
    git submodule update --init
    ```

## Preparation of the Target Project

For using the WAMPInterfaceForOmnetpp in a given project, the following steps have to be performed:

1. Add the WAMPInterfaceForOmnetpp as Project Reference under `Properties -> Project References`.

2. Import the include path under `Properties -> OMNeT++ -> Makemake -> src -> Options... -> Compile -> Add include paths exported from referenced projects`

3. Link the boost libraries `boost_system` and `boost_thread` under `Properties -> OMNeT++ -> Makemake -> src -> Options... -> Link -> Additional libraries to link with`

4. Check the Makemake options at `Properties -> OMNeT++ -> Makemake -> src -> Options... -> Preview`. The lower text area should include the following, otherwise check the steps above.

    ```
    -f --deep -KWAMPINTERFACEFOROMNETPP_PROJ=../../WAMPInterfaceForOmnetpp -I$(WAMPINTERFACEFOROMNETPP_PROJ)/autobahn-cpp -I$(WAMPINTERFACEFOROMNETPP_PROJ)/msgpack-c/include -I$(WAMPINTERFACEFOROMNETPP_PROJ)/src -L$(WAMPINTERFACEFOROMNETPP_PROJ)/src -lboost_system -lboost_thread -lWAMPInterfaceForOmnetpp$(D)`
    ```

## Settings

To adapt the settings to fit your system, adapt the following parameters in the Settings class of the WAMPInterfaceForOmnetpp project.

* `LOCALHOST_IP_ADDRESS_STRING` has to be set to the IP address where the WAMP router can be found (localhost when running the router on the same machine).
* `DEFAULT_REALM` defines to which realm on the router the sessions of the simulation connect to. In the given default Crossbar router configuration this is the realm opplive.
* `DEFAULT_RAWSOCKET_PORT` to define to which port on the router the simulation sessions shall connect. In the default configuration this is port 9000.
