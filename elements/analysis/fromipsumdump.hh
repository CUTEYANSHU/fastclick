// -*- mode: c++; c-basic-offset: 4 -*-
#ifndef CLICK_FROMIPSUMDUMP_HH
#define CLICK_FROMIPSUMDUMP_HH
#include <click/element.hh>
#include <click/task.hh>
#include <click/ipflowid.hh>
#include "ipsumdumpinfo.hh"
CLICK_DECLS

/*
=c

FromIPSummaryDump(FILENAME [, I<KEYWORDS>])

=s analysis

reads packets from an IP summary dump file

=d

Reads IP packet descriptors from a file produced by ToIPSummaryDump, then
creates packets containing info from the descriptors and pushes them out the
output. Optionally stops the driver when there are no more packets.

The file may be compressed with gzip(1) or bzip2(1); FromIPSummaryDump will
run zcat(1) or bzcat(1) to uncompress it.

FromIPSummaryDump reads from the file named FILENAME unless FILENAME is a
single dash `C<->', in which case it reads from the standard input. It will
not uncompress the standard input, however.

Keyword arguments are:

=over 8

=item STOP

Boolean. If true, then FromIPSummaryDump will ask the router to stop when it
is done reading. Default is false.

=item ACTIVE

Boolean. If false, then FromIPSummaryDump will not emit packets (until the
`C<active>' handler is written). Default is true.

=item ZERO

Boolean. Determines the contents of packet data not set by the dump. If true,
this data is zero. If false (the default), this data is random garbage.

=item PROTO

Byte (0-255). Sets the IP protocol used for output packets when the dump
doesn't specify a protocol. Default is 6 (TCP).

=item MULTIPACKET

Boolean. If true, then FromIPSummaryDump will emit multiple packets for each
line---specifically, it will emit as many packets as the packet count field
specifies. Default is false.

=item SAMPLE

Unsigned real number between 0 and 1. FromIPSummaryDump will output each
packet with probability SAMPLE. Default is 1. FromIPSummaryDump uses
fixed-point arithmetic, so the actual sampling probability may differ
substantially from the requested sampling probability. Use the
C<sampling_prob> handler to find out the actual probability. If MULTIPACKET is
true, then the sampling probability applies separately to the multiple packets
generated per record.

=item DEFAULT_CONTENTS

String, containing a space-separated list of content names (see
ToIPSummaryDump for the possibilities). Defines the default contents of the
dump.

=item DEFAULT_FLOWID

String, containing a space-separated flow ID (source address, source port,
destination address, destination port, and, optionally, protocol). Defines the
IP addresses and ports used by default.

=back

Only available in user-level processes.

=n

Packets generated by FromIPSummaryDump always have IP version 4 and IP header
length 5. The rest of the packet data is zero or garbage, unless set by the
dump. Generated packets will usually have incorrect lengths, but the extra
header length annotations are set correctly.

=h sampling_prob read-only

Returns the sampling probability (see the SAMPLE keyword argument).

=h active read/write

Value is a Boolean.

=h encap read-only

Returns `IP'. Useful for ToDump's USE_ENCAP_FROM option.

=h filesize read-only

Returns the length of the FromIPSummaryDump file, in bytes, or "-" if that
length cannot be determined.

=h filepos read-only

Returns FromIPSummaryDump's position in the file, in bytes.

=h stop write-only

When written, sets `active' to false and stops the driver.

=a

ToIPSummaryDump */

class FromIPSummaryDump : public Element, public IPSummaryDumpInfo { public:

    FromIPSummaryDump();
    ~FromIPSummaryDump();

    const char *class_name() const	{ return "FromIPSummaryDump"; }
    const char *processing() const	{ return AGNOSTIC; }
    FromIPSummaryDump *clone() const	{ return new FromIPSummaryDump; }

    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);
    void add_handlers();

    void run_scheduled();
    Packet *pull(int);

    enum { DO_TCPOPT_MSS = 1, DO_TCPOPT_WSCALE = 2, DO_TCPOPT_SACK = 4,
	   DO_TCPOPT_TIMESTAMP = 8, DO_TCPOPT_UNKNOWN = 16,
	   DO_TCPOPT_PADDING = 32,
	   DO_TCPOPT_ALL = 0xFFFFFFFFU };    
    static int parse_tcp_opt_ascii(const char *, int, String *, int);
    
  private:

    enum { BUFFER_SIZE = 32768, SAMPLING_SHIFT = 28 };
    
    int _fd;
    char *_buffer;
    int _pos;
    int _len;
    int _buffer_len;
    int _save_char;

    Vector<int> _contents;
    uint16_t _default_proto;
    uint32_t _sampling_prob;
    IPFlowID _flowid;
    uint32_t _aggregate;

    bool _stop : 1;
    bool _format_complaint : 1;
    bool _zero : 1;
    bool _active : 1;
    bool _multipacket : 1;
    bool _have_flowid : 1;
    bool _use_flowid : 1;
    bool _have_aggregate : 1;
    bool _use_aggregate : 1;
    bool _binary : 1;
    Packet *_work_packet;
    uint32_t _multipacket_extra_length;
    int _binary_size;

    Task _task;
    Vector<String> _words;	// for speed

    String _filename;
    FILE *_pipe;
    off_t _file_offset;
    int _minor_version;
    IPFlowID _given_flowid;

    int error_helper(ErrorHandler *, const char *);
    int read_buffer(ErrorHandler *);
    int read_line(String &, ErrorHandler *);

    void bang_data(const String &, ErrorHandler *);
    void bang_flowid(const String &, click_ip *, ErrorHandler *);
    void bang_aggregate(const String &, ErrorHandler *);
    void bang_binary(const String &, ErrorHandler *);
    void check_defaults();
    Packet *read_packet(ErrorHandler *);
    Packet *handle_multipacket(Packet *);

    static String read_handler(Element *, void *);
    static int write_handler(const String &, Element *, void *, ErrorHandler *);
    
};

CLICK_ENDDECLS
#endif
