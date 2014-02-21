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
package org.apache.qpid.server.virtualhost;

import java.util.Collection;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ScheduledFuture;

import org.apache.qpid.server.exchange.AMQUnknownExchangeType;
import org.apache.qpid.common.Closeable;
import org.apache.qpid.server.configuration.VirtualHostConfiguration;
import org.apache.qpid.server.connection.IConnectionRegistry;
import org.apache.qpid.server.exchange.Exchange;
import org.apache.qpid.server.message.MessageDestination;
import org.apache.qpid.server.message.MessageSource;
import org.apache.qpid.server.plugin.ExchangeType;
import org.apache.qpid.server.protocol.AMQSessionModel;
import org.apache.qpid.server.protocol.LinkRegistry;
import org.apache.qpid.server.queue.AMQQueue;
import org.apache.qpid.server.security.SecurityManager;
import org.apache.qpid.server.stats.StatisticsGatherer;
import org.apache.qpid.server.store.DurableConfigurationStore;
import org.apache.qpid.server.store.MessageStore;
import org.apache.qpid.server.txn.DtxRegistry;

public interface VirtualHost extends DurableConfigurationStore.Source, Closeable, StatisticsGatherer
{
    IConnectionRegistry getConnectionRegistry();

    VirtualHostConfiguration getConfiguration();

    String getName();

    AMQQueue getQueue(String name);
    MessageSource getMessageSource(String name);

    AMQQueue getQueue(UUID id);

    Collection<AMQQueue> getQueues();

    int removeQueue(AMQQueue queue);

    AMQQueue createQueue(Map<String, Object> arguments) throws QueueExistsException;


    Exchange createExchange(UUID id,
                            String exchange,
                            String type,
                            boolean durable,
                            boolean autoDelete,
                            String alternateExchange)
            throws ExchangeExistsException, ReservedExchangeNameException,
                   UnknownExchangeException, AMQUnknownExchangeType;

    void removeExchange(Exchange exchange, boolean force) throws ExchangeIsAlternateException,
                                                                 RequiredExchangeException;

    MessageDestination getMessageDestination(String name);

    Exchange getExchange(String name);
    Exchange getExchange(UUID id);


    Exchange getDefaultExchange();

    Collection<Exchange> getExchanges();

    Collection<ExchangeType<? extends Exchange>> getExchangeTypes();

    DurableConfigurationStore getDurableConfigurationStore();

    MessageStore getMessageStore();

    SecurityManager getSecurityManager();

    void addVirtualHostListener(VirtualHostListener listener);

    void close();

    UUID getId();

    void scheduleHouseKeepingTask(long period, HouseKeepingTask task);

    long getHouseKeepingTaskCount();

    public long getHouseKeepingCompletedTaskCount();

    int getHouseKeepingPoolSize();

    void setHouseKeepingPoolSize(int newSize);

    int getHouseKeepingActiveCount();

    VirtualHostRegistry getVirtualHostRegistry();

    DtxRegistry getDtxRegistry();

    LinkRegistry getLinkRegistry(String remoteContainerId);

    ScheduledFuture<?> scheduleTask(long delay, Runnable timeoutTask);

    State getState();

    public void block();

    public void unblock();

    long getDefaultAlertThresholdMessageAge();

    long getDefaultAlertThresholdMessageSize();

    long getDefaultAlertThresholdQueueDepthMessages();

    long getDefaultAlertThresholdQueueDepthBytes();

    long getDefaultAlertRepeatGap();

    long getDefaultQueueFlowControlSizeBytes();

    long getDefaultQueueFlowResumeSizeBytes();

    int getDefaultMaximumDeliveryAttempts();
}