// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_AIO_IMAGE_REQUEST_WQ_H
#define CEPH_LIBRBD_AIO_IMAGE_REQUEST_WQ_H

#include "include/Context.h"
#include "common/WorkQueue.h"
#include "common/Mutex.h"
#include "librbd/ImageWatcher.h"

namespace librbd {

class AioCompletion;
class AioImageRequest;
class ImageCtx;

class AioImageRequestWQ : protected ThreadPool::PointerWQ<AioImageRequest> {
public:
  AioImageRequestWQ(ImageCtx *image_ctx, const string &name, time_t ti,
                    ThreadPool *tp);

  ssize_t read(uint64_t off, size_t len, char *buf, int op_flags);
  ssize_t write(uint64_t off, size_t len, const char *buf, int op_flags);
  int discard(uint64_t off, uint64_t len);

  void aio_read(AioCompletion *c, uint64_t off, size_t len, char *buf,
                bufferlist *pbl, int op_flags);
  void aio_write(AioCompletion *c, uint64_t off, size_t len, const char *buf,
                 int op_flags);
  void aio_discard(AioCompletion *c, uint64_t off, uint64_t len);
  void aio_flush(AioCompletion *c);

  using ThreadPool::PointerWQ<AioImageRequest>::drain;
  using ThreadPool::PointerWQ<AioImageRequest>::empty;

  inline bool writes_empty() const {
    Mutex::Locker locker(m_lock);
    return (m_queued_writes == 0);
  }

  inline bool writes_blocked() const {
    Mutex::Locker locker(m_lock);
    return (m_write_blockers > 0);
  }

  void block_writes();
  void unblock_writes();

  void register_lock_listener();

protected:
  virtual void *_void_dequeue();
  virtual void process(AioImageRequest *req);

private:
  struct LockListener : public ImageWatcher::Listener {
    AioImageRequestWQ *aio_work_queue;
    LockListener(AioImageRequestWQ *_aio_work_queue)
      : aio_work_queue(_aio_work_queue) {
    }

    virtual bool handle_requested_lock() {
      return true;
    }
    virtual void handle_lock_updated(ImageWatcher::LockUpdateState state) {
      aio_work_queue->handle_lock_updated(state);
    }
  };

  ImageCtx &m_image_ctx;
  mutable Mutex m_lock;
  Cond m_cond;
  uint32_t m_write_blockers;
  uint32_t m_in_progress_writes;
  uint32_t m_queued_writes;

  LockListener m_lock_listener;
  bool m_blocking_writes;

  bool is_journal_required() const;
  bool is_lock_required() const;
  void queue(AioImageRequest *req);

  void handle_lock_updated(ImageWatcher::LockUpdateState state);
};

} // namespace librbd

#endif // CEPH_LIBRBD_AIO_IMAGE_REQUEST_WQ_H
