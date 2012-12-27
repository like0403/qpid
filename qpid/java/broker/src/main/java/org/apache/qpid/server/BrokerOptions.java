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
package org.apache.qpid.server;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class BrokerOptions
{
    public static final String DEFAULT_CONFIG_FILE = "etc/config.json";
    public static final String DEFAULT_LOG_CONFIG_FILE = "etc/log4j.xml";
    public static final String QPID_HOME = "QPID_HOME";
    public static final String QPID_WORK = "QPID_WORK";

    private final Set<Integer> _ports = new HashSet<Integer>();
    private final Set<Integer> _sslPorts = new HashSet<Integer>();
    private final Map<ProtocolExclusion,Set<Integer>> _exclusionMap = new HashMap<ProtocolExclusion, Set<Integer>>();
    private final Map<ProtocolInclusion,Set<Integer>> _inclusionMap = new HashMap<ProtocolInclusion, Set<Integer>>();

    private String _configFile;
    private String _logConfigFile;
    private String _bind;
    private Integer _jmxPortRegistryServer;
    private Integer _jmxPortConnectorServer;

    private Integer _logWatchFrequency = 0;
    private String _qpidWorkFolder;
    private String _qpidHomeFolder;

    private String _configurationStore;
    private String _configurationStoreType;
    private boolean _noDefault;

    public void addPort(final int port)
    {
        _ports.add(port);
    }

    public void addSSLPort(final int sslPort)
    {
        _sslPorts.add(sslPort);
    }

    public Set<Integer> getPorts()
    {
        return Collections.unmodifiableSet(_ports);
    }

    public Set<Integer> getSSLPorts()
    {
        return Collections.unmodifiableSet(_sslPorts);
    }

    @Deprecated
    public String getConfigFile()
    {
        return _configFile;
    }

    @Deprecated
    public void setConfigFile(final String configFile)
    {
        _configFile = configFile;
    }

    public String getLogConfigFile()
    {
        return _logConfigFile;
    }

    public void setLogConfigFile(final String logConfigFile)
    {
        _logConfigFile = logConfigFile;
    }

    public Integer getJmxPortRegistryServer()
    {
        return _jmxPortRegistryServer;
    }

    public void setJmxPortRegistryServer(final int jmxPortRegistryServer)
    {
        _jmxPortRegistryServer = jmxPortRegistryServer;
    }

    public Integer getJmxPortConnectorServer()
    {
        return _jmxPortConnectorServer;
    }

    public void setJmxPortConnectorServer(final int jmxPortConnectorServer)
    {
        _jmxPortConnectorServer = jmxPortConnectorServer;
    }
    public String getQpidHome()
    {
        return _qpidHomeFolder == null? System.getProperty(QPID_HOME): _qpidHomeFolder;
    }

    public Set<Integer> getExcludedPorts(final ProtocolExclusion excludeProtocol)
    {
        final Set<Integer> excludedPorts = _exclusionMap.get(excludeProtocol);
        return excludedPorts == null ? Collections.<Integer>emptySet() : excludedPorts;
    }

    public void addExcludedPort(final ProtocolExclusion excludeProtocol, final int port)
    {
        if (!_exclusionMap.containsKey(excludeProtocol))
        {
            _exclusionMap.put(excludeProtocol, new HashSet<Integer>());
        }

        Set<Integer> ports = _exclusionMap.get(excludeProtocol);
        ports.add(port);
    }

    public String getBind()
    {
        return _bind;
    }

    public void setBind(final String bind)
    {
        _bind = bind;
    }

    public int getLogWatchFrequency()
    {
        return _logWatchFrequency;
    }

    /**
     * Set the frequency with which the log config file will be checked for updates.
     * @param logWatchFrequency frequency in seconds
     */
    public void setLogWatchFrequency(final int logWatchFrequency)
    {
        _logWatchFrequency = logWatchFrequency;
    }

    public Set<Integer> getIncludedPorts(final ProtocolInclusion includeProtocol)
    {
        final Set<Integer> includedPorts = _inclusionMap.get(includeProtocol);
        return includedPorts == null ? Collections.<Integer>emptySet() : includedPorts;
    }

    public void addIncludedPort(final ProtocolInclusion includeProtocol, final int port)
    {
        if (!_inclusionMap.containsKey(includeProtocol))
        {
            _inclusionMap.put(includeProtocol, new HashSet<Integer>());
        }

        Set<Integer> ports = _inclusionMap.get(includeProtocol);
        ports.add(port);
    }

    public String getQpidWork()
    {
        return _qpidWorkFolder;
    }

    public void setQpidWork(String qpidWorkFolder)
    {
        _qpidWorkFolder = qpidWorkFolder;
    }

    public void setQpidHome(String qpidHomeFolder)
    {
        _qpidHomeFolder = qpidHomeFolder;
    }

    public String getConfigurationStore()
    {
        return _configurationStore;
    }

    public void setConfigurationStore(String cofigurationStore)
    {
        _configurationStore = cofigurationStore;
    }

    public String getConfigurationStoreType()
    {
        return _configurationStoreType;
    }

    public void setConfigurationStoreType(String cofigurationStoreType)
    {
        _configurationStoreType = cofigurationStoreType;
    }

    public boolean isNoDefault()
    {
        return _noDefault;
    }

    public void setNoDefault(boolean noDefault)
    {
        _noDefault = noDefault;
    }

}