/* $Id: libtele.c,v 1.7 2003/10/12 10:01:00 cegger Exp $
******************************************************************************

   libtele.c

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
                 2002 Tobias Hunger     [tobias@fresco.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include "config.h"
#include <ggi/internal/plat.h>
#include <ggi/gg.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#include <ggi/ggi.h>

#include "libtele.h"

#ifdef GGI_BIG_ENDIAN
# define MY_ENDIAN	TELE_BIG_ENDIAN
#else
# define MY_ENDIAN	TELE_LITTLE_ENDIAN
#endif

static T_Long calc_initial_seq_ctr(void)
{
	struct timeval tv;

	ggCurTime(&tv);

	return (T_Long) tv.tv_sec * 1103515245 + (T_Long) tv.tv_usec;
}

static void reverse_longwords(TeleEvent *ev)
{
	T_Long *start;
	int count;

	start = (T_Long *) ev;
	start++;

	for (count = ev->rawstart - 1; count > 0; count--) {
		
		T_Long val = *start;

		*start++ = ((val & 0xff000000) >> 24) |
		           ((val & 0x00ff0000) >>  8) |
		           ((val & 0x0000ff00) <<  8) |
		           ((val & 0x000000ff) << 24);
	}
}


/* ---------------------------------------------------------------------- */


static int do_poll_event(int fd)
{
	fd_set fds;

	int err;

	do {
		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		tv.tv_sec = tv.tv_usec = 0;
	
		err = select(fd+1, &fds, NULL, NULL, &tv);

	} while ((err < 0) && (errno == EINTR));

	if (err < 0) {
		perror("libtele: poll_event");
		return 0;
	}

	return FD_ISSET(fd, &fds);
}

static int do_read_event(int sock_fd, TeleEvent *ev)
{
	int num;
	unsigned int count;

	unsigned char *buf = (unsigned char *) ev;


	/* read first byte (which is the size) */

	do {
		num = read(sock_fd, buf, 1);

	} while ((num < 0) && (errno == EINTR));
	
	if (num < 0) {
		switch (errno) {
		case EPIPE:
#ifdef ECONNRESET
		case ECONNRESET:
#endif
#ifdef ECONNABORTED
		case ECONNABORTED:
#endif
#ifdef ESHUTDOWN
		case ESHUTDOWN:
#endif
#ifdef ETIMEDOUT
		case ETIMEDOUT:
#endif
			return TELE_ERROR_SHUTDOWN;
		}

		perror("libtele: read_event");
		return num; 
	}

	if ((num == 0) || (ev->size <= 1)) {
		/* End of file */
		return TELE_ERROR_SHUTDOWN;
	}

	if (ev->size < TELE_MINIMAL_EVENT) {
		fprintf(stderr, "libtele: received bogus event! "
			"(size=%d)\n", ev->size);
		return TELE_ERROR_BADEVENT;
	}

	/* read rest of event */

	count = ev->size*4 - 1;
	buf++;

	while (count > 0) {

		num = read(sock_fd, buf, count);

		if (num > 0) {
			buf   += num;
			count -= num;
		}

		if (num == 0) {
			/* End of file */
			return TELE_ERROR_SHUTDOWN;
		}

		if ((num < 0) && (errno == EINTR)) {
			/* introitus interruptus :) */
			continue;
		}

		if (num < 0) {
			switch (errno) {
				case EPIPE:
#ifdef ECONNRESET
			case ECONNRESET:
#endif
#ifdef ECONNABORTED
			case ECONNABORTED:
#endif
#ifdef ESHUTDOWN
			case ESHUTDOWN:
#endif
#ifdef ETIMEDOUT
			case ETIMEDOUT:
#endif
				return TELE_ERROR_SHUTDOWN;
			}

			perror("libtele: read_event");
			return num; 
		}
	}
		
	if (((ev->endian != TELE_BIG_ENDIAN) &&
	     (ev->endian != TELE_LITTLE_ENDIAN)) ||
	    (ev->rawstart > ev->size)) {

		fprintf(stderr, "libtele: received bogus event!\n");
		return TELE_ERROR_BADEVENT;
	}

	return ev->size*4;
}

static int do_write_event(int sock_fd, TeleEvent *ev)
{
	int num;
	unsigned int count = ev->size*4;

	unsigned char *buf = (unsigned char *) ev;

	while (count > 0) {

		num = write(sock_fd, buf, count);

		if (num > 0) {
			buf   += num;
			count -= num;
		}

		if ((num < 0) && (errno == EINTR)) {
			/* exodus interruptus :) */
			continue;
		}

		if (num < 0) {
			switch (errno) {
				case EPIPE:
#ifdef ECONNRESET
			case ECONNRESET:
#endif
#ifdef ECONNABORTED
			case ECONNABORTED:
#endif
#ifdef ESHUTDOWN
			case ESHUTDOWN:
#endif
#ifdef ETIMEDOUT
			case ETIMEDOUT:
#endif
				return TELE_ERROR_SHUTDOWN;
			}

			perror("libtele: write_event");
			return num; 
		}
	}

	return ev->size*4;
}

/*** For conceptual use only :-) ***/
#if 0
static int tclient_write_flush(TeleClient *c)
{
	int num;
	int count = c->writebuf_head - c->writebuf_tail;
	int sock_fd = c->sock_fd;

	unsigned char *buf = ((unsigned char *) c->writebuf) + c->writebuf_tail;

	/* Clear O_NONBLOCK temporarily */
	fcntl(sock_fd, F_SETFL,
		fnctl(sock_fd, F_GETFL) & ~O_NONBLOCK);

	while (count > 0) {

		num = write(sock_fd, buf, count);

		if (num > 0) {
			buf   += num;
			count -= num;
		}

		if ((num < 0) && (errno == EINTR)) {
			/* exodus interruptus :) */
			continue;
		}

		if (num < 0) {
			switch (errno) {
				case EPIPE:
#ifdef ECONNRESET
			case ECONNRESET:
#endif
#ifdef ECONNABORTED
			case ECONNABORTED:
#endif
#ifdef ESHUTDOWN
			case ESHUTDOWN:
#endif
#ifdef ETIMEDOUT
			case ETIMEDOUT:
#endif
				return TELE_ERROR_SHUTDOWN;
			}

			perror("libtele: write_event");

			fcntl(sock_fd, F_SETFL,
				fnctl(sock_fd, F_GETFL) | O_NONBLOCK);

			return num; 
		}
	}

	fcntl(sock_fd, F_SETFL,
		fnctl(sock_fd, F_GETFL) | O_NONBLOCK);

	return ev->size*4;
}

static int tclient_write_nonblock(TeleClient *c, TeleEvent *ev)
{
	int num, len;
	int evsize = ev->size*4;
	int sock_fd = c->sock_fd; 
	unsigned char *buf = ((unsigned char *) c->writebuf) + c->writebuf_tail;

	/* Head minus tail equals body :-) */
	int len = c->writebuf_head - c->writebuf_tail;

	/* Empty buffers if full */
	if (c->writebuf_head + evsize > TELE_WRITEBUF_LEN) {
		tclient_write_flush(c);
	}
	
	/* Event is larger than buffer!  Do a blocking write. */
	if (evsize > TELE_WRITEBUF_LEN) {
		do_write_event(c->sock_fd, ev);
		return evsize;
	}

	/* Copy event to end of buffer (really the head :) */
	memcpy(buf + c->writebuf_head, ev, (size_t)evsize);
	c->writebuf_head += evsize;
	
	while (len > 0) {

		num = write(sock_fd, buf, len);

		if (num > 0) {
			buf += num;
			len -= num;

			c->writebuf_tail += num;
		}

		if ((num < 0) && (errno == EINTR)) {
			/* exodus interruptus :) */
			continue;
		}

		if (num < 0) {
			switch (errno) {
				case EPIPE:
#ifdef ECONNRESET
			case ECONNRESET:
#endif
#ifdef ECONNABORTED
			case ECONNABORTED:
#endif
#ifdef ESHUTDOWN
			case ESHUTDOWN:
#endif
#ifdef ETIMEDOUT
			case ETIMEDOUT:
#endif
				return TELE_ERROR_SHUTDOWN;

			case EWOULDBLOCK:
			case EAGAIN:
				
				return 0;	/* Ok, we queued it */ 
			}

			perror("libtele: write_nonblock");
			return num; 
		}
	}

	return evsize;
}
#endif

static void *do_prepare_event(TeleEvent *event, TeleEventType type, 
			      int data_size, int raw_size,
			      T_Long sequence)
{
	TeleEvent *ev = event;
	
	int size = TELE_MINIMAL_EVENT + ((data_size + raw_size + 3) / 4);

	struct timeval cur_time;

	
	if ((data_size % 4) != 0) {
		fprintf(stderr, "DO_PREPARE_EVENT: ILLEGAL DATA SIZE ! "
			"(%d bytes)\n", data_size);
		exit(1);
	}

	if (size > 255) {
		fprintf(stderr, "DO_PREPARE_EVENT: ILLEGAL SIZE ! "
			"(%d longwords)\n", size);
		exit(1);
	}

	ggCurTime(&cur_time);

	ev->size = size;
	ev->rawstart = TELE_MINIMAL_EVENT + (data_size / 4);

	ev->type = type;
	ev->device = 0;
	ev->sequence = sequence;
	
	ev->time.sec  = cur_time.tv_sec;
	ev->time.nsec = cur_time.tv_usec * 1000;

	return (void *) ev->data;
}


/* ---------------------------------------------------------------------- */


#ifdef HAVE_UNIX_DOMAIN_SOCKET
static int tclient_open_unix(TeleClient *c, const char *addr)
{
	struct sockaddr_un dest_un;

	int err;

	c->inet = 0;
	c->display = 0;		/* ??? */
	c->endianness = MY_ENDIAN;

	dest_un.sun_family = AF_UNIX;
	strcpy(dest_un.sun_path, addr);

	c->sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (c->sock_fd < 0) {
		perror("tclient: socket");
		return -1;
	}

	do {
		err = connect(c->sock_fd, (struct sockaddr *) &dest_un, 
			      sizeof(dest_un));
	} while ((err < 0) && (errno == EINTR));

	if (err < 0) {
		perror("tclient: connect");
		close(c->sock_fd);
		return -1;
	}

	return TELE_OK;
}
#endif /* HAVE_UNIX_DOMAIN_SOCKET */

static int tclient_open_inet(TeleClient *c, const char *addr)
{
	struct sockaddr_in dest_in;

	int err;

	unsigned int n = 0;
	const char *port = addr;
	char name[512];
	unsigned int display_num = 27780;
	struct hostent *h;


	/* Find a colon: this is the port to use
	   FIXME: allow TELE_PORT_BASE + display_num
	*/
	while (*port && *port++ != ':') n++;

	sscanf(port, "%u", &display_num);


	/* Obtain numerical address from string */
	ggstrlcpy(name, addr, sizeof(name));

	h = gethostbyname(name);

	if (!h) {
#ifdef HAVE_HERROR
		herror("tclient: gethostbyname");
#else
		fprintf(stderr, "tclient: gethostbyname error.\n");
#endif
		return -1;
	}


	c->inet = 1;
	c->display = display_num;
	c->endianness = MY_ENDIAN;

	dest_in.sin_family      = AF_INET;
	dest_in.sin_port        = htons(display_num);
	dest_in.sin_addr        = *((struct in_addr *)h->h_addr);

	c->sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (c->sock_fd < 0) {
		perror("tclient: socket");
		return -1;
	}

	do {
		err = connect(c->sock_fd, (struct sockaddr *) &dest_in, 
			      sizeof(dest_in));
	} while ((err < 0) && (errno == EINTR));

	if (err < 0) {
		perror("tclient: connect");
		close(c->sock_fd);
		return -1;
	}

	return TELE_OK;
}

int tclient_open(TeleClient *c, const char *addrspec)
{
	int err;
	unsigned int n = 0;
	const char *addr = addrspec;

	/* Find a colon: this is the type of socket to use */
	while (*addr && *addr++ != ':') n++;

	/* Select type of socket depending on first field */

	/* Inet sockets */
	if (n==0 || !strncmp(addrspec, "inet", n)) {
		fprintf(stderr, "tclient: Trying inet socket (%s)...\n", addr);
		
		err = tclient_open_inet(c, addr);
	}
	/* Unix domain sockets */
	else if (!strncmp(addrspec, "unix", n)) {
#ifdef HAVE_UNIX_DOMAIN_SOCKET
		fprintf(stderr, "tclient: Trying unix socket (%s)...\n", addr);
		
		err = tclient_open_unix(c, addr);
#else /* HAVE_UNIX_DOMAIN_SOCKET */
		fprintf(stderr, "tclient: Unix-domain sockets not supported\n");
		err = -1;			/* shut up compiler. */
#endif /* HAVE_UNIX_DOMAIN_SOCKET */
	}
	else {
		fprintf(stderr, "tclient: unknown socket type (%*s)\n",
			n, addrspec);

		err = -1;
	}


	if (err < 0) {
		return err;
	}

	/* Block pipe signals so we can detect the other end
	 * disappearing without being killed ourselves.
	 */
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	
	/* Calculate sequence counter */

	c->seq_ctr = calc_initial_seq_ctr();
	
	return err;
}
	
int tclient_close(TeleClient *c)
{
	close(c->sock_fd);

	return TELE_OK;
}

void *tclient_new_event(TeleClient *c, TeleEvent *event, 
		        TeleEventType type, int data_size, int raw_size)
{
	event->endian = c->endianness;

	c->seq_ctr++;

	return do_prepare_event(event, type, data_size, raw_size,
				c->seq_ctr);
}

int tclient_poll(TeleClient *c)
{
	return do_poll_event(c->sock_fd);
}

int tclient_read(TeleClient *c, TeleEvent *event)
{
	int result = do_read_event(c->sock_fd, event);

	if (result < 0) {
		return result;
	}

	if (c->endianness != event->endian) {
		reverse_longwords(event);
		event->endian = TELE_REVERSE_ENDIAN;
	} else {
		event->endian = TELE_NORMAL_ENDIAN;
	}

	return result;
}

int tclient_write(TeleClient *c, TeleEvent *event)
{
	return do_write_event(c->sock_fd, event);
}


/* ---------------------------------------------------------------------- */


int tserver_init(TeleServer *s, int display)
{
#ifdef HAVE_UNIX_DOMAIN_SOCKET
	struct sockaddr_un me_un;
#endif /* HAVE_UNIX_DOMAIN_SOCKET */
	struct sockaddr_in me_in;

	struct sockaddr *me = NULL;
	unsigned int me_len = 0;

	int inet = (display < 10);

	if ((display < 0) || (display > 19)) {
		fprintf(stderr, "tserver: Bad display (%d).\n", display);
		return -1;
	}
	
	display %= 10;

	s->inet = inet;
	s->display = display;
	s->endianness = MY_ENDIAN;

	if (s->inet) {
		fprintf(stderr, "tserver: Creating inet socket [%d]\n",
				TELE_PORT_BASE + display);

		me_in.sin_family      = AF_INET;
		me_in.sin_port        = htons(TELE_PORT_BASE + display);
		me_in.sin_addr.s_addr = INADDR_ANY;

		me = (struct sockaddr *) &me_in;
		me_len = sizeof(me_in);

		s->conn_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	} else {
#ifdef HAVE_UNIX_DOMAIN_SOCKET
		fprintf(stderr, "tserver: Creating unix socket... [%d]\n",
				TELE_PORT_BASE + display);

		me_un.sun_family = AF_UNIX;

		sprintf(me_un.sun_path, "%s%d", TELE_FIFO_BASE, 
			s->display);
		
		me = (struct sockaddr *) &me_un;
		me_len = sizeof(me_un);

		s->conn_fd = socket(AF_UNIX, SOCK_STREAM, 0);
#else /* HAVE_UNIX_DOMAIN_SOCKET */
		fprintf(stderr, "tserver: Unix domain sockets not supported\n");
		s->conn_fd = -1;
#endif /* HAVE_UNIX_DOMAIN_SOCKET */
	}

	if (s->conn_fd < 0) {
		perror("tserver: socket");
		return -1;
	}

	if (bind(s->conn_fd, me, me_len) < 0) {
		perror("tserver: bind");
		close(s->conn_fd);
		return -1;
	}

	if (listen(s->conn_fd, 5) < 0) {
		perror("tserver: listen");
		close(s->conn_fd);
		return -1;
	}

	return TELE_OK;
}

int tserver_exit(TeleServer *s)
{
	close(s->conn_fd);

	if (! s->inet) {
		char filename[200];

#ifdef HAVE_SNPRINTF
		snprintf(filename, 200, "%s%d",
			TELE_FIFO_BASE, s->display);
#else
		sprintf(filename, "%s%d",
			TELE_FIFO_BASE, s->display);
#endif

		unlink(filename);
	}

	return TELE_OK;
}

int tserver_check(TeleServer *s)
{
	return do_poll_event(s->conn_fd);
}

int tserver_open(TeleServer *s, TeleUser *u)
{
#ifdef HAVE_UNIX_DOMAIN_SOCKET
	struct sockaddr_un you_un;
#endif /* HAVE_UNIX_DOMAIN_SOCKET */
	struct sockaddr_in you_in;

	struct sockaddr *you = NULL;
	int you_len = 0;


	u->server = s;

	if (s->inet) {
		you = (struct sockaddr *) &you_in;
		you_len = sizeof(you_in);
	} 
#ifdef HAVE_UNIX_DOMAIN_SOCKET
	else {
		you = (struct sockaddr *) &you_un;
		you_len = sizeof(you_un);
	}
#endif /* HAVE_UNIX_DOMAIN_SOCKET */

	do {
		u->sock_fd = accept(s->conn_fd, you, &you_len);

	} while ((u->sock_fd < 0) && (errno == EINTR));

	if (u->sock_fd < 0) {
		perror("tserver: accept");
		return -1;
	}

	/* Block all pipe signals so we can detect the other end
	 * disappearing.
	 */
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	
	/* Calculate sequence counter */

	u->seq_ctr = calc_initial_seq_ctr();
	
	return TELE_OK;
}

void *tserver_new_event(TeleUser *u, TeleEvent *event, 
		        TeleEventType type, int data_size, int raw_size)
{
	event->endian = u->server->endianness;

	u->seq_ctr++;

	return do_prepare_event(event, type, data_size, raw_size,
				u->seq_ctr);
}

int tserver_poll(TeleUser *u)
{
	return do_poll_event(u->sock_fd);
}

int tserver_read(TeleUser *u, TeleEvent *event)
{
	int result = do_read_event(u->sock_fd, event);

	if (result < 0) {
		return result;
	}

	if (u->server->endianness != event->endian) {
		reverse_longwords(event);
		event->endian = TELE_REVERSE_ENDIAN;
	} else {
		event->endian = TELE_NORMAL_ENDIAN;
	}

	return result;
}

int tserver_write(TeleUser *u, TeleEvent *event)
{
	return do_write_event(u->sock_fd, event);
}

int tserver_close(TeleUser *u)
{
	close(u->sock_fd); 

	return TELE_OK;
}
