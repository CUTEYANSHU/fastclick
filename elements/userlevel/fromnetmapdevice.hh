// -*- c-basic-offset: 4 -*-
#ifndef CLICK_FROMNETMAPDEVICE_HH
#define CLICK_FROMNETMAPDEVICE_HH
#include <click/config.h>
#include <click/task.hh>
#include <click/etheraddress.hh>
#include <click/netmapdevice.hh>
#include "queuedevice.hh"
#include <vector>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

CLICK_DECLS

/*
 * =c
 *
 * FromNetmapDevice
 *
 *  * =item DEVNAME
 *
 * String.  Device number
 *  *
 * =item PROMISC
 *
 * Boolean.  FromNetmapDevice puts the device in promiscuous mode if PROMISC is
 * true. The default is false.
 *
 * =item BURST
 *
 * Unsigned integer. Maximal number of packets that will be processed before
 *  rescheduling Click default is 32.
 *
 * =item MAXTHREADS
 *
 * Maximal number of threads that this element will take to read packets from
 * 	the input queue. If unset (or negative) all threads not pinned with a
 * 	ThreadScheduler element will be shared among FromNetmapDevice elements and
 *  other input elements supporting multiqueue (extending QueueDevice)
 *
 * =item THREADOFFSET
 *
 * Specify which Click thread will handle this element. If multiple
 *  j threads are used, threads with id THREADOFFSET+j will be used. Default is
 *  to share the threads available on the device's NUMA node equally.
 *
 * =item MAXQUEUES
 *
 * Maximum number of hardware queue to use. Default is to use all available
 * 	queue. If you use RSS, do not set this below the number of queues receiving
 * 	packets hashed by RSS or you won't serve packets.
 *
 *
 */



class FromNetmapDevice: public QueueDevice {

public:

    FromNetmapDevice() CLICK_COLD;

    void selected(int, int);

    const char *class_name() const		{ return "FromNetmapDevice"; }
    const char *port_count() const		{ return PORTS_0_1; }
    const char *processing() const		{ return PUSH; }

    int configure_phase() const			{ return CONFIGURE_PHASE_PRIVILEGED - 5; }
    void* cast(const char*);


    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;
    int initialize(ErrorHandler*) CLICK_COLD;
    void cleanup(CleanupStage) CLICK_COLD;


    inline bool receive_packets(Task* task, int begin, int end, bool fromtask);

    bool run_task(Task *);

  protected:

    NetmapDevice* _device;
    bool _promisc;
    bool _blockant;
    unsigned int _burst;

    //Do not quit the task until we have sended all possible packets (until all queues are empty)
    bool _keephand;

    std::vector<int> _queue_for_fd;

    int queue_for_fd(int fd) {
        return _queue_for_fd[fd];
    }

    void add_handlers();

    String read_handler(Element *e, void *);

    int write_handler(const String &, Element *e, void *,
                                      ErrorHandler *);

};

CLICK_ENDDECLS
#endif
