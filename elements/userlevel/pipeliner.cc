// -*- c-basic-offset: 4; related-file-name: "pipeliner.hh" -*-
/*
 * pipeliner.{cc,hh} --
 */

#include <click/config.h>
#include "pipeliner.hh"
#include <click/standard/scheduleinfo.hh>
#include <click/ring.hh>
#include <click/args.hh>

CLICK_DECLS


#define PS_MIN_THRESHOLD 2048
//#define PS_BATCH_SIZE 1024

Pipeliner::Pipeliner()
    :   _ring_size(-1),_block(false),out_id(0),sleepiness(0),_task(NULL),last_start(0) {
#if HAVE_BATCH
	in_batch_mode = BATCH_MODE_YES;
#endif
}

Pipeliner::~Pipeliner()
{

}


bool
Pipeliner::get_runnable_threads(Bitvector& b) {
    unsigned int thisthread = router()->home_thread_id(this);
    b[thisthread] = 1;
    return false;
}


void Pipeliner::cleanup(CleanupStage) {
    for (unsigned i = 0; i < storage.size(); i++) {
    	Packet* p;
        while ((p = storage.get_value(i).extract()) != 0) {
#if HAVE_BATCH
        	if (receives_batch == 1)
        		static_cast<PacketBatch*>(p)->kill();
        	else
#endif
        		p->kill();
        }
    }
}



int
Pipeliner::configure(Vector<String> & conf, ErrorHandler * errh)
{

    if (Args(conf, this, errh)
    .read_p("SIZE", _ring_size)
	.read_p("BLOCKING", _block)
    .complete() < 0)
        return -1;
    return 0;
}


int
Pipeliner::initialize(ErrorHandler *errh)
{

    Bitvector v = get_threads();
    storage.compress(v);
    stats.compress(v);
    out_id = router()->home_thread_id(this);

    if (_ring_size == -1) {
	#  if HAVE_BATCH
		if (receives_batch) {
			_ring_size = 16;
		} else
	#  endif
		{
			_ring_size = 1024;
		}
    }

    for (unsigned i = 0; i < storage.size(); i++) {
        storage.get_value(i).initialize(_ring_size);
    }

    for (int i = 0; i < v.size(); i++) {
        if (v[i]) {
            click_chatter("Pipeline from %d to %d",i,out_id);
            WritablePacket::pool_transfer(out_id,i);
        }
    }

    _task = new Task(this);
    ScheduleInfo::initialize_task(this, _task, true, errh);
    _task->move_thread(out_id);

    return 0;
}

#if HAVE_BATCH
void Pipeliner::push_batch(int,PacketBatch* head) {
	retry:
    if (storage->insert(head)) {
        if (sleepiness >= 4)
                    _task->reschedule();
    } else {
        //click_chatter("Drop!");
    if (_block)
        goto retry;
        head->kill();
    }
}
#endif

void Pipeliner::push_packet(int,Packet* p) {
	retry:
    if (storage->insert(p)) {

    } else {
        if (_block) {
            if (sleepiness >= _ring_size / 4)
                _task->reschedule();
            goto retry;
        }
        p->kill();
        stats->dropped++;
		if (stats->dropped % 100 == 1)
			click_chatter("%s : Dropped %d packets : have %d packets in ring", name().c_str(), stats->dropped, storage->count());
    }
    if (sleepiness >= _ring_size / 4)
        _task->reschedule();
}

#define HINT_THRESHOLD 32
bool
Pipeliner::run_task(Task* t)
{
    bool r = false;
    last_start++; //Used to RR the balancing of revert storage
    for (unsigned j = 0; j < storage.size(); j++) {
        int i = (last_start + j) % storage.size();
        PacketRing& s = storage.get_value(i);
        PacketBatch* out = NULL;
        while (!s.is_empty()) {
#if HAVE_BATCH
            PacketBatch* b = static_cast<PacketBatch*>(s.extract());
            if (unlikely(!receives_batch)) {
            	if (out == NULL) {
            		b->set_tail(b);
            		b->set_count(1);
            		out = b;
            	} else {
            		out->append_packet(b);
            	}
            } else {
            	if (out == NULL) {
					out = b;
				} else {
					out->append_batch(b);
				}
            }
            //pool_hint(b->count(),storage.get_mapping(i));

            //click_chatter("Read %d[%d]",storage.get_mapping(i),tail % PIPELINE_RING_SIZE);
#else
            (void)out;
            //click_chatter("Read %d[%d]",storage.get_mapping(i),tail % PIPELINE_RING_SIZE);
            Packet* p = s.extract();
            output(0).push(p);

                //WritablePacket::pool_hint(HINT_THRESHOLD,storage.get_mapping(i));

            r = true;
#endif
        }

#if HAVE_BATCH
        if (out) {
            output(0).push_batch(out);
            r = true;
        }
#endif

    }
    if (!r) {
        sleepiness++;
        if (sleepiness < (_ring_size / 4)) {
            t->fast_reschedule();
        }
    } else {
        sleepiness = 0;
        t->fast_reschedule();
    }
    return r;

}

void Pipeliner::add_handlers()
{
    add_read_handler("n_dropped", dropped_handler, 0);
}

CLICK_ENDDECLS

ELEMENT_REQUIRES(userlevel)
EXPORT_ELEMENT(Pipeliner)
ELEMENT_MT_SAFE(Pipeliner)
