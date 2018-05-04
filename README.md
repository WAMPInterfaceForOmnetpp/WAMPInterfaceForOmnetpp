# Live Monitoring and Remote Control of OMNeT++ Simulations

## Installation Instructions

\begin{itemize}
\item \omnet
\item The Boost\footnote{\url{http://www.boost.org/}} libraries \code{boost\_system}, \code{boost\_filesystem}, \code{boost\_thread} and \code{boost\_program\_options}\footnote{\code{apt-get install }\code{libboost-system-dev }\code{libboost-filesystem-dev }\code{libboost-thread-dev }\code{libboost-program-options-dev}}.

\item The Crossbar.io\footnote{\url{https://github.com/crossbario/crossbar}} \ac{WAMP} router\footnote{\code{apt-get install python3-pip \&\& pip3 install crossbar}}. It has to be started before running any simulation!\footnote{\code{crossbar start}}

\item The AutobahnCPP implementation\footnote{\url{https://github.com/crossbario/autobahn-cpp}}

\item The INET framework \footnote{\url{https://inet.omnetpp.org}}. The interface is tested with INET 5.1.

\item The \code{WAMPInterfaceForOmnetpp} project\footnote{\url{https://github.com/WAMPInterfaceForOmnetpp/WAMPInterfaceForOmnetpp}} that should be added to the \omnet workspace.
\end{itemize}

\subsection{Preparation of the Target Project}
\label{sec:Include}
For including the interface into the target project, the \code{WAMPInterfaceForOmnetpp} project has to be referenced in your projects settings and the previously named Boost libraries need to be added to the linker options of the project as well as the include paths to \code{autobahn-cpp} and \code{msgpack/include} of the \code{WAMPInterfaceForOmnetpp} project.

To adapt the settings to fit your system, adapt the following parameters in the \code{Settings} class of the \code{WAMPInterfaceForOmnetpp} project.

\begin{description}
  \item[\code{LOCALHOST\_IP\_ADDRESS\_STRING}] has to be set to the IP address where the \ac{WAMP} router can be found (\code{localhost} when running the router on the same machine).
  \item[\code{DEFAULT\_REALM}] defines to which realm on the router the sessions of the simulation connect to. In the given default Crossbar router configuration this is the realm \code{opplive}.
\item[\code{DEFAULT\_RAWSOCKET\_PORT}] to define to which port on the router the simulation sessions shall connect. In the default configuration this is port 9000.
\end{description}
