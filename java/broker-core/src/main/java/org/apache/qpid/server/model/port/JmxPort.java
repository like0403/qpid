/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
package org.apache.qpid.server.model.port;

import java.util.Set;

import org.apache.qpid.server.model.AuthenticationProvider;
import org.apache.qpid.server.model.ManagedAttribute;
import org.apache.qpid.server.model.ManagedObject;
import org.apache.qpid.server.model.Port;
import org.apache.qpid.server.model.Protocol;
import org.apache.qpid.server.model.Transport;

@ManagedObject( category = false, type = "JMX")
public interface JmxPort<X extends JmxPort<X>> extends Port<X>
{
    @ManagedAttribute( mandatory = true )
    AuthenticationProvider getAuthenticationProvider();

    @ManagedAttribute( defaultValue = "TCP",
                       validValues = {"[ \"TCP\" ]", "[ \"SSL\" ]"})
    Set<Transport> getTransports();

    @ManagedAttribute( defaultValue = "JMX_RMI", validValues = { "[ \"JMX_RMI\"]"} )
    Set<Protocol> getProtocols();

    void setPortManager(PortManager manager);
}