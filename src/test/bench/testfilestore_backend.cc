// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-

#include "testfilestore_backend.h"
#include "global/global_init.h"
#include "os/ObjectStore.h"

struct C_DeleteTransWrapper : public Context {
  Context *c;
  ObjectStore::Transaction *t;
  C_DeleteTransWrapper(
    ObjectStore::Transaction *t,
    Context *c) : c(c), t(t) {}
  void finish(int r) {
    c->complete(r);
    delete t;
  }
};

TestFileStoreBackend::TestFileStoreBackend(
  FileStore *os, bool write_infos)
  : os(os), finisher(g_ceph_context), write_infos(write_infos)
{
  finisher.start();
}

void TestFileStoreBackend::write(
  const string &oid,
  uint64_t offset,
  const bufferlist &bl,
  Context *on_applied,
  Context *on_commit)
{

  size_t sep = oid.find("/");
  assert(sep != string::npos);
  assert(sep + 1 < oid.size());
  coll_t c;
  bool valid_coll = c.parse(oid.substr(0, sep));
  assert(valid_coll);
  string coll_str = c.to_str();

  hobject_t h(sobject_t(oid.substr(sep+1), 0));
  h.pool = 0;

#if 0
  os->_write(c,ghobject_t(h), offset, bl.length(), bl);
#else
  
  ObjectStore::Transaction *t = new ObjectStore::Transaction;
  if (!osrs.count(coll_str))
    osrs.insert(make_pair(coll_str, ObjectStore::Sequencer(coll_str)));
  ObjectStore::Sequencer *osr = &(osrs.find(coll_str)->second);

  t->write(c, ghobject_t(h), offset, bl.length(), bl);

  if (write_infos) {
    bufferlist bl2;
    for (uint64_t j = 0; j < 128; ++j) bl2.append(0);
    coll_t meta;
    hobject_t info(sobject_t(string("info_")+coll_str, 0));
    t->write(meta, ghobject_t(info), 0, bl2.length(), bl2);
  }
  
  os->queue_transaction(
    osr,
    t,
    new C_DeleteTransWrapper(t, on_applied),
    on_commit);
 #endif
    
}

void TestFileStoreBackend::read(
  const string &oid,
  uint64_t offset,
  uint64_t length,
  bufferlist *bl,
  Context *on_complete)
{
  size_t sep = oid.find("/");
  assert(sep != string::npos);
  assert(sep + 1 < oid.size());
  coll_t c;
  bool valid_coll = c.parse(oid.substr(0, sep));
  assert(valid_coll);
  hobject_t h(sobject_t(oid.substr(sep+1), 0));
  h.pool = 0;
  os->read(c, ghobject_t(h), offset, length, *bl);
  finisher.queue(on_complete);
}
