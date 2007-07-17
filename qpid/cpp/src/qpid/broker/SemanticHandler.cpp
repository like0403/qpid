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

#include "SemanticHandler.h"
#include "BrokerAdapter.h"
#include "qpid/framing/ChannelAdapter.h"

using namespace qpid::broker;
using namespace qpid::framing;

SemanticHandler::SemanticHandler(ChannelId id, Connection& c) : 
    connection(c),
    channel(c, id, &c.broker.getStore())
{
    init(id, connection.getOutput(), connection.getVersion());
    adapter = std::auto_ptr<BrokerAdapter>(new BrokerAdapter(channel, connection, connection.broker, *this));
}


void SemanticHandler::handle(framing::AMQFrame& frame) 
{
    handleBody(frame.getBody());
}

//ChannelAdapter virtual methods:
void SemanticHandler::handleMethodInContext(boost::shared_ptr<qpid::framing::AMQMethodBody> method, 
                                            const qpid::framing::MethodContext& context)
{
    try{
        if(getId() != 0 && !method->isA<ChannelOpenBody>() && !isOpen()) {
            if (!method->isA<ChannelCloseOkBody>()) {
                std::stringstream out;
                out << "Attempt to use unopened channel: " << getId();
                throw ConnectionException(504, out.str());
            }
        } else {
            method->invoke(*adapter, context);
        }
    }catch(ChannelException& e){
        adapter->getProxy().getChannel().close(
            e.code, e.toString(),
            method->amqpClassId(), method->amqpMethodId());
        connection.closeChannel(getId());
    }catch(ConnectionException& e){
        connection.close(e.code, e.toString(), method->amqpClassId(), method->amqpMethodId());
    }catch(std::exception& e){
        connection.close(541/*internal error*/, e.what(), method->amqpClassId(), method->amqpMethodId());
    }

}

bool SemanticHandler::isOpen() const 
{ 
    return channel.isOpen(); 
}

void SemanticHandler::handleHeader(boost::shared_ptr<qpid::framing::AMQHeaderBody> body) 
{
    channel.handleHeader(body);
}

void SemanticHandler::handleContent(boost::shared_ptr<qpid::framing::AMQContentBody> body) 
{
    channel.handleContent(body);
}

void SemanticHandler::handleHeartbeat(boost::shared_ptr<qpid::framing::AMQHeartbeatBody> body) 
{
    channel.handleHeartbeat(body);
}

