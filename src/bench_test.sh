filestore_path=/data1/ceph_data/osd.1/
journal_path=/data2/ceph_data

rm -fr $filestore_path
rm -fr $journal_path
mkdir -p $filestore_path
mkdir -p $journal_path

rm -fr  /data1/testdir/*
rm -fr mytest.log

#obj_size=67108864 #64M
obj_size=$[4*1024*1024]
io_size=$[4*1024* 1024]
disable_detail=1

debug_filestore=0/20
debug_journal=0/20
filestore_op_threads=2
sequential=1

max_ops=8000
num_colls=20
num_objects=10240
inflight_ops=64
num_writers=1

./ceph_smalliobenchfs --filestore-path $filestore_path  --journal-path ${journal_path}/osd.1.journal --num-concurrent-ops=$inflight_ops --num-objects $num_objects --num-colls $num_colls --num-writers $num_writers --write-ratio 1.0  --disable-detailed-ops $disable_detail --sequential $sequential --max-ops $max_ops  --object-size $obj_size --io-size $io_size  --log-file mytest.log  --journal_force_aio=true --journal_dio=true --journal_aio=true --debug-filestore=$debug_filestore --filestore_op_threads=$filestore_op_threads --debug-journal=$debug_journal --op-dump-file=dump --filestore_fd_cache_size=1024


