#ifndef _qpid_agent_ManagementAgent_
#define _qpid_agent_ManagementAgent_

//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//

#include "qpid/agent/QmfAgentImportExport.h"
#include "qpid/management/ManagementObject.h"
#include "qpid/management/ManagementEvent.h"
#include "qpid/management/Manageable.h"
#include "qpid/sys/Mutex.h"
#include "qpid/client/ConnectionSettings.h"

namespace qpid {
namespace management {

class Notifyable {
public:
    virtual ~Notifyable() {}
    virtual void notify() = 0;
};

class ManagementAgent
{
  public:

    class Singleton {
    public:
        QMF_AGENT_EXTERN Singleton(bool disableManagement = false);
        QMF_AGENT_EXTERN ~Singleton();
        QMF_AGENT_EXTERN static ManagementAgent* getInstance();
    private:
        static sys::Mutex lock;
        static bool disabled;
        static int refCount;
        static ManagementAgent* agent;
    };

    typedef enum {
    SEV_EMERG = 0,
    SEV_ALERT = 1,
    SEV_CRIT  = 2,
    SEV_ERROR = 3,
    SEV_WARN  = 4,
    SEV_NOTE  = 5,
    SEV_INFO  = 6,
    SEV_DEBUG = 7,
    SEV_DEFAULT = 8
    } severity_t;

    ManagementAgent() {}
    virtual ~ManagementAgent() {}

    virtual int getMaxThreads() = 0;

    // Set the name of the agent
    //
    //   vendor   - Vendor name or domain (i.e. "apache.org")
    //   product  - Product name (i.e. "qpid")
    //   instance - A unique identifier for this instance of the agent.
    //              If empty, the agent will create a GUID for the instance.
    //
    virtual void setName(const std::string& vendor,
                         const std::string& product,
                         const std::string& instance="") = 0;

    // Connect to a management broker
    //
    //   brokerHost        - Hostname or IP address (dotted-quad) of broker.
    //
    //   brokerPort        - TCP port of broker.
    //
    //   intervalSeconds   - The interval (in seconds) that this agent shall use
    //                       between broadcast updates to the broker.
    //
    //   useExternalThread - If true, the thread of control used for callbacks
    //                       must be supplied by the user of the object (via the
    //                       pollCallbacks method).
    //
    //                       If false, callbacks shall be invoked on the management
    //                       agent's thread.  In this case, the callback implementations
    //                       MUST be thread safe.
    //
    //   storeFile         - File where this process has read and write access.  This
    //                       file shall be used to store persistent state.
    //
    virtual void init(const std::string& brokerHost = "localhost",
                      uint16_t brokerPort = 5672,
                      uint16_t intervalSeconds = 10,
                      bool useExternalThread = false,
                      const std::string& storeFile = "",
                      const std::string& uid = "guest",
                      const std::string& pwd = "guest",
                      const std::string& mech = "PLAIN",
                      const std::string& proto = "tcp") = 0;

    virtual void init(const client::ConnectionSettings& settings,
                      uint16_t intervalSeconds = 10,
                      bool useExternalThread = false,
                      const std::string& storeFile = "") = 0;

    // Register a schema with the management agent.  This is normally called by the
    // package initializer generated by the management code generator.
    //
    virtual void
    registerClass(const std::string& packageName,
                  const std::string& className,
                  uint8_t*    md5Sum,
                  management::ManagementObject::writeSchemaCall_t schemaCall) = 0;

    virtual void
    registerEvent(const std::string& packageName,
                  const std::string& eventName,
                  uint8_t*    md5Sum,
                  management::ManagementEvent::writeSchemaCall_t schemaCall) = 0;

    // Add a management object to the agent.  Once added, this object shall be visible
    // in the greater management context.
    //
    // Please note that ManagementObject instances are not explicitly deleted from
    // the management agent.  When the core object represented by a management object
    // is deleted, the "resourceDestroy" method on the management object must be called.
    // It will then be reclaimed in due course by the management agent.
    //
    // Once a ManagementObject instance is added to the agent, the agent then owns the
    // instance.  The caller MUST NOT free the resources of the instance at any time.
    // When it is no longer needed, invoke its "resourceDestroy" method and discard the
    // pointer.  This allows the management agent to report the deletion of the object
    // in an orderly way.
    //
    virtual ObjectId addObject(ManagementObject* objectPtr, uint64_t persistId = 0) = 0;
    virtual ObjectId addObject(ManagementObject* objectPtr,
                               const std::string& key,
                               bool persistent = true) = 0;

    //
    //
    virtual void raiseEvent(const ManagementEvent& event,
                            severity_t severity = SEV_DEFAULT) = 0;

    // If "useExternalThread" was set to true in init, this method must
    // be called to provide a thread for any pending method calls that have arrived.
    // The method calls for ManagementObject instances shall be invoked synchronously
    // during the execution of this method.
    //
    // callLimit may optionally be used to limit the number of callbacks invoked.
    // if 0, no limit is imposed.
    //
    // The return value is the number of callbacks that remain queued after this
    // call is complete.  It can be used to determine whether or not further calls
    // to pollCallbacks are necessary to clear the backlog.  If callLimit is zero,
    // the return value will also be zero.
    //
    virtual uint32_t pollCallbacks(uint32_t callLimit = 0) = 0;

    // In the "useExternalThread" scenario, there are three ways that an application can
    // use to be notified that there is work to do.  Of course the application may periodically
    // call pollCallbacks if it wishes, but this will cause long latencies in the responses
    // to method calls.
    //
    // The notification methods are:
    //
    //  1) Register a C-style callback by providing a pointer to a function
    //  2) Register a C++-style callback by providing an object of a class that is derived
    //     from Notifyable
    //  3) Call getSignalFd() to get a file descriptor that can be used in a select
    //     call.  The file descriptor shall become readable when the agent has work to
    //     do.  Note that this mechanism is specific to Posix-based operating environments.
    //     getSignalFd will probably not function correctly on Windows.
    //
    // If a callback is registered, the callback function will be called on the agent's
    // thread.  The callback function must perform no work other than to signal the application
    // thread to call pollCallbacks.
    //
    typedef void (*cb_t)(void*);
    virtual void setSignalCallback(cb_t callback, void* context) = 0;
    virtual void setSignalCallback(Notifyable& notifyable) = 0;
    virtual int getSignalFd() = 0;
};

}}

#endif  /*!_qpid_agent_ManagementAgent_*/
