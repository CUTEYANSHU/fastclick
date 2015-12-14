// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * FlowStrip.{cc,hh} -- element FlowStrips bytes from front of packet
 * Robert Morris, Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include "flowstrip.hh"

#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
CLICK_DECLS

FlowStrip::FlowStrip()
{
	in_batch_mode = BATCH_MODE_NEEDED;
}

int
FlowStrip::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return Args(conf, this, errh).read_mp("LENGTH", _nbytes).complete();
}

PacketBatch *
FlowStrip::simple_action_batch(PacketBatch *head)
{
	Packet* current = head;
	while (current != NULL) {
		current->pull(_nbytes);
		current = current->next();
	}
	return head;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(flow)
EXPORT_ELEMENT(FlowStrip)
ELEMENT_MT_SAFE(FlowStrip)
