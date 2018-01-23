// -*- c-basic-offset: 4 -*-
/*
 * flowdirectorparser.hh -- Flow Director parsing API between
 * Click and DPDK.
 *
 * Copyright (c) 2018 Georgios Katsikas, RISE SICS AB
 * Copyright (c) 2018 Tom Barbette, University of Liège
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

#ifndef CLICK_FLOWDIRECTORPARSER_HH
#define CLICK_FLOWDIRECTORPARSER_HH

#include <click/config.h>
#include <click/error.hh>
#include <click/flowdirectorglue.hh>

#define FLOWDIR_ERROR ((int)-1)

CLICK_DECLS

/**
 * Flow parsing API
 */

/**
 * Obtains an instance of the Flow Director parser.
 *
 * @param errh an instance of the error handler
 * @return a parser object
 */
struct cmdline *flow_parser_init(
	ErrorHandler *errh
);

/**
 * Creates an instance of the Flow Director parser
 * on a given context of instructions, obtained
 * from DPDK.
 *
 * @param prompt a user prompt message
 * @param errh an instance of the error handler
 * @return a command line object
 */
struct cmdline *flow_parser_alloc(
	const char *prompt,
	ErrorHandler *errh
);

/**
 * Parses a given command.
 *
 * @param cl the command line instance
 * @param input_cmd the input commandto be parsed
 * @param errh an instance of the error handler
 * @return the status of the parsing
 */
int flow_parser_parse(
	struct cmdline *cl,
	char *input_cmd,
	ErrorHandler *errh
);

CLICK_ENDDECLS

#endif
