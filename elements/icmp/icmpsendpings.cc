/* -*- c-basic-offset: 2 -*- */
/*
 * icmpsendpings.{cc,hh} -- Send ICMP ping packets.
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

#include <click/config.h>
#include "icmpsendpings.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/click_ip.h>
#include <click/click_icmp.h>
#include <click/packet_anno.hh>

ICMPSendPings::ICMPSendPings()
  : _timer(this)
{
  MOD_INC_USE_COUNT;
  add_output();
}

ICMPSendPings::~ICMPSendPings()
{
  MOD_DEC_USE_COUNT;
}

ICMPSendPings *
ICMPSendPings::clone() const
{
  return new ICMPSendPings;
}

int
ICMPSendPings::configure(const Vector<String> &conf, ErrorHandler *errh)
{
  _ip_id = 1;
  _icmp_id = 0;
  _interval = 1000;
  return cp_va_parse(conf, this, errh,
                     cpIPAddress, "source IP address", &_src,
                     cpIPAddress, "destination IP address", &_dst,
		     cpKeywords,
		     "INTERVAL", cpSecondsAsMilli, "time between pings (s)", &_interval,
		     "IDENTIFIER", cpUnsignedShort, "ICMP echo identifier", &_icmp_id,
		     0);
}

int
ICMPSendPings::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_after_ms(_interval);
  return 0;
}

void
ICMPSendPings::uninitialize()
{
  _timer.unschedule();
}

void
ICMPSendPings::run_scheduled()
{
  WritablePacket *q = Packet::make(sizeof(click_ip) +
                                   sizeof(struct icmp_sequenced));
  memset(q->data(), '\0', q->length());

  click_ip *nip = reinterpret_cast<click_ip *>(q->data());
  nip->ip_v = 4;
  nip->ip_hl = sizeof(click_ip) >> 2;
  nip->ip_len = htons(q->length());
  nip->ip_id = htons(_ip_id);
  nip->ip_p = IP_PROTO_ICMP; /* icmp */
  nip->ip_ttl = 200;
  nip->ip_src = _src;
  nip->ip_dst = _dst;
  nip->ip_sum = click_in_cksum((unsigned char *)nip, sizeof(click_ip));

  icmp_sequenced *icp = (struct icmp_sequenced *) (nip + 1);
  icp->icmp_type = ICMP_ECHO;
  icp->icmp_code = 0;
  icp->identifier = htons(_icmp_id);
  icp->sequence = htons(_ip_id);

  icp->icmp_cksum = click_in_cksum((unsigned char *)icp, sizeof(icmp_sequenced));

  q->set_dst_ip_anno(IPAddress(_dst));
  q->set_ip_header(nip, sizeof(click_ip));

  output(0).push(q);

  _timer.schedule_after_ms(_interval);
  _ip_id++;
}

EXPORT_ELEMENT(ICMPSendPings)
