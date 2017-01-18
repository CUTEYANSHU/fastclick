#ifndef CLICK_TIMESTAMPDIFF_HH
#define CLICK_TIMESTAMPDIFF_HH

#include <vector>

#include <click/batchelement.hh>

CLICK_DECLS

/*
=c

TimestampDiff()

=s timestamps

Compute the difference between the recorded timestamp of a packet using
RecordTimestamp and the number inside the packet payload probably setted using
NumberPacket

=a

RecordTimestamp, NumberPacket

*/
class TimestampDiff : public BatchElement {
public:
    TimestampDiff() CLICK_COLD;
    ~TimestampDiff() CLICK_COLD;

    const char *class_name() const { return "TimestampDiff"; }
    const char *port_count() const { return PORTS_1_1; }
    const char *processing() const { return PUSH; }
    const char *flow_code() const { return "x/x"; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
    void add_handlers() CLICK_COLD;
    static String read_handler(Element*, void*) CLICK_COLD;

    void push(int, Packet *);
#if HAVE_BATCH
    void push_batch(int, PacketBatch *);
#endif

private:
    std::vector<unsigned> _delays;
    int _offset;
    void smaction(Packet* p);
};

CLICK_ENDDECLS

#endif
