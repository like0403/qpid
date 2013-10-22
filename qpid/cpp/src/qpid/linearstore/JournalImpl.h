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

#ifndef QPID_LINEARSTORE_JOURNALIMPL_H
#define QPID_LINEARSTORE_JOURNALIMPL_H

#include <set>
#include "qpid/linearstore/jrnl/enums.h"
#include "qpid/linearstore/jrnl/EmptyFilePoolTypes.h"
#include "qpid/linearstore/jrnl/jcntl.h"
#include "qpid/linearstore/DataTokenImpl.h"
#include "qpid/linearstore/PreparedTransaction.h"
#include "qpid/broker/PersistableQueue.h"
#include "qpid/sys/Timer.h"
#include "qpid/sys/Time.h"
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/intrusive_ptr.hpp>
#include "qpid/management/Manageable.h"
#include "qmf/org/apache/qpid/linearstore/Journal.h"

namespace qpid{

namespace sys {
class Timer;
}
namespace qls_jrnl {
class EmptyFilePool;
}

namespace linearstore{

class JournalImpl;
class JournalLogImpl;

class InactivityFireEvent : public qpid::sys::TimerTask
{
    JournalImpl* _parent;
    qpid::sys::Mutex _ife_lock;

  public:
    InactivityFireEvent(JournalImpl* p, const qpid::sys::Duration timeout);
    virtual ~InactivityFireEvent() {}
    void fire();
    inline void cancel() { qpid::sys::Mutex::ScopedLock sl(_ife_lock); _parent = 0; }
};

class GetEventsFireEvent : public qpid::sys::TimerTask
{
    JournalImpl* _parent;
    qpid::sys::Mutex _gefe_lock;

  public:
    GetEventsFireEvent(JournalImpl* p, const qpid::sys::Duration timeout);
    virtual ~GetEventsFireEvent() {}
    void fire();
    inline void cancel() { qpid::sys::Mutex::ScopedLock sl(_gefe_lock); _parent = 0; }
};

class JournalImpl : public qpid::broker::ExternalQueueStore, public qpid::qls_jrnl::jcntl, public qpid::qls_jrnl::aio_callback
{
  public:
    typedef boost::function<void (JournalImpl&)> DeleteCallback;

  protected:
    qpid::sys::Timer& timer;
    JournalLogImpl& _journalLogRef;
    bool getEventsTimerSetFlag;
    boost::intrusive_ptr<qpid::sys::TimerTask> getEventsFireEventsPtr;
    qpid::sys::Mutex _getf_lock;
    qpid::sys::Mutex _read_lock;

    bool writeActivityFlag;
    bool flushTriggeredFlag;
    boost::intrusive_ptr<qpid::sys::TimerTask> inactivityFireEventPtr;

    qpid::management::ManagementAgent* _agent;
    qmf::org::apache::qpid::linearstore::Journal::shared_ptr _mgmtObject;
    DeleteCallback deleteCallback;

  public:

    JournalImpl(qpid::sys::Timer& timer,
                const std::string& journalId,
                const std::string& journalDirectory,
                JournalLogImpl& journalLogRef,
                const qpid::sys::Duration getEventsTimeout,
                const qpid::sys::Duration flushTimeout,
                qpid::management::ManagementAgent* agent,
                DeleteCallback deleteCallback=DeleteCallback() );

    virtual ~JournalImpl();

    void initManagement(qpid::management::ManagementAgent* agent);

    void initialize(qpid::qls_jrnl::EmptyFilePool* efp,
                    const uint16_t wcache_num_pages,
                    const uint32_t wcache_pgsize_sblks,
                    qpid::qls_jrnl::aio_callback* const cbp);

    inline void initialize(qpid::qls_jrnl::EmptyFilePool* efpp,
                           const uint16_t wcache_num_pages,
                           const uint32_t wcache_pgsize_sblks) {
        initialize(efpp, wcache_num_pages, wcache_pgsize_sblks, this);
    }

    void recover(boost::shared_ptr<qpid::qls_jrnl::EmptyFilePoolManager> efpm,
                 const uint16_t wcache_num_pages,
                 const uint32_t wcache_pgsize_sblks,
                 qpid::qls_jrnl::aio_callback* const cbp,
                 boost::ptr_list<PreparedTransaction>* prep_tx_list_ptr,
                 uint64_t& highest_rid,
                 uint64_t queue_id);

    inline void recover(boost::shared_ptr<qpid::qls_jrnl::EmptyFilePoolManager> efpm,
                        const uint16_t wcache_num_pages,
                        const uint32_t wcache_pgsize_sblks,
                        boost::ptr_list<PreparedTransaction>* prep_tx_list_ptr,
                        uint64_t& highest_rid,
                        uint64_t queue_id) {
        recover(efpm, wcache_num_pages, wcache_pgsize_sblks, this, prep_tx_list_ptr, highest_rid, queue_id);
    }

    void recover_complete();

    // Temporary fn to read and save last msg read from journal so it can be assigned
    // in chunks. To be replaced when coding to do this direct from the journal is ready.
    // Returns true if the record is extern, false if local.
//    bool loadMsgContent(uint64_t rid, std::string& data, size_t length, size_t offset = 0);

    // Overrides for write inactivity timer
    void enqueue_data_record(const void* const data_buff, const size_t tot_data_len,
                             const size_t this_data_len, qpid::qls_jrnl::data_tok* dtokp,
                             const bool transient = false);

    void enqueue_extern_data_record(const size_t tot_data_len, qpid::qls_jrnl::data_tok* dtokp,
                                    const bool transient = false);

    void enqueue_txn_data_record(const void* const data_buff, const size_t tot_data_len,
                                 const size_t this_data_len, qpid::qls_jrnl::data_tok* dtokp, const std::string& xid,
                                 const bool transient = false);

    void enqueue_extern_txn_data_record(const size_t tot_data_len, qpid::qls_jrnl::data_tok* dtokp,
                                        const std::string& xid, const bool transient = false);

    void dequeue_data_record(qpid::qls_jrnl::data_tok* const dtokp, const bool txn_coml_commit = false);

    void dequeue_txn_data_record(qpid::qls_jrnl::data_tok* const dtokp, const std::string& xid, const bool txn_coml_commit = false);

    void txn_abort(qpid::qls_jrnl::data_tok* const dtokp, const std::string& xid);

    void txn_commit(qpid::qls_jrnl::data_tok* const dtokp, const std::string& xid);

    void stop(bool block_till_aio_cmpl = false);

    // Overrides for get_events timer
    qpid::qls_jrnl::iores flush(const bool block_till_aio_cmpl = false);

    // TimerTask callback
    void getEventsFire();
    void flushFire();

    // AIO callbacks
    virtual void wr_aio_cb(std::vector<qpid::qls_jrnl::data_tok*>& dtokl);
    virtual void rd_aio_cb(std::vector<uint16_t>& pil);

    qpid::management::ManagementObject::shared_ptr GetManagementObject (void) const
    { return _mgmtObject; }

    qpid::management::Manageable::status_t ManagementMethod (uint32_t,
                                                             qpid::management::Args&,
                                                             std::string&);

    void resetDeleteCallback() { deleteCallback = DeleteCallback(); }

  protected:
//    void free_read_buffers();
    void createStore();

    inline void setGetEventTimer()
    {
        getEventsFireEventsPtr->setupNextFire();
        timer.add(getEventsFireEventsPtr);
        getEventsTimerSetFlag = true;
    }
    void handleIoResult(const qpid::qls_jrnl::iores r);

    // Management instrumentation callbacks overridden from jcntl
    inline void instr_incr_outstanding_aio_cnt() {
      if (_mgmtObject.get() != 0) _mgmtObject->inc_outstandingAIOs();
    }
    inline void instr_decr_outstanding_aio_cnt() {
      if (_mgmtObject.get() != 0) _mgmtObject->dec_outstandingAIOs();
    }

}; // class JournalImpl

class TplJournalImpl : public JournalImpl
{
  public:
    TplJournalImpl(qpid::sys::Timer& timer,
                   const std::string& journalId,
                   const std::string& journalDirectory,
                   JournalLogImpl& journalLogRef,
                   const qpid::sys::Duration getEventsTimeout,
                   const qpid::sys::Duration flushTimeout,
                   qpid::management::ManagementAgent* agent) :
        JournalImpl(timer, journalId, journalDirectory, journalLogRef, getEventsTimeout, flushTimeout, agent)
    {}

    virtual ~TplJournalImpl() {}

/*
    // Special version of read_data_record that ignores transactions - needed when reading the TPL
    inline qpid::qls_jrnl::iores read_data_record(void** const datapp, std::size_t& dsize,
                                                void** const xidpp, std::size_t& xidsize, bool& transient, bool& external,
                                                qpid::qls_jrnl::data_tok* const dtokp) {
        return JournalImpl::read_data_record(datapp, dsize, xidpp, xidsize, transient, external, dtokp, true);
    }
    inline void read_reset() { _rmgr.invalidate(); }
*/
}; // class TplJournalImpl

} // namespace msgstore
} // namespace mrg

#endif // ifndef QPID_LINEARSTORE_JOURNALIMPL_H
