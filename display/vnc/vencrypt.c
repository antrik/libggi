/* $Id: vencrypt.c,v 1.2 2008/09/16 19:29:15 pekberg Exp $
******************************************************************************

   display-vnc: RFB VeNCrypt protocol extension

   Copyright (C) 2008 Peter Rosin	[peda@lysator.liu.se]

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

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif
#endif

#include <sys/types.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <errno.h>

#include <ggi/internal/gg_replace.h>
#include <ggi/display/vnc.h>
#include <ggi/internal/ggi_debug.h>
#include "common.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/des.h>

#include "encoding.h"


int GGI_vnc_read_ready(ggi_vnc_client *client);
int GGI_vnc_write_ready(ggi_vnc_client *client);
int GGI_vnc_client_init(ggi_vnc_client *client);
int GGI_vnc_client_vnc_auth(ggi_vnc_client *client);
int GGI_vnc_client_vencrypt_security(ggi_vnc_client *client);

struct options {
	SSL_CTX *ssl_ctx;
	int x509;
	char *ciphers;
};

struct vencrypt {
	uint16_t version;
	BIO *ssl_bio;
	int write_wants_read;
	int read_wants_write;
};

static void
log_error(void)
{
	int res = ERR_get_error();
	while (res) {
		DPRINT("%s\n", ERR_error_string(res, NULL));
		res = ERR_get_error();
	}
}

static int
tls_read(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct vencrypt *vencrypt = client->vencrypt;
	SSL *ssl;
	unsigned char buf[100];
	int len;

	/* DPRINT("tls_read()\n"); */

	BIO_get_ssl(vencrypt->ssl_bio, &ssl);
	if (!ssl) {
		DPRINT("No SSL object?\n");
		GGI_vnc_close_client(client);
		return -1;
	}

	len = SSL_read(ssl, buf, sizeof(buf));

	switch (SSL_get_error(ssl, len)) {
	case SSL_ERROR_NONE:
		break;
	case SSL_ERROR_WANT_READ:
		goto run_actions;
	case SSL_ERROR_WANT_WRITE:
		vencrypt->read_wants_write = 1;
		DPRINT("SSL_read wants to write\n");
		priv->add_cwfd(priv->gii_ctx, client, client->cwfd);
		priv->del_crfd(priv->gii_ctx, client);
		goto run_actions;
	case SSL_ERROR_ZERO_RETURN:
		DPRINT("socket closed\n");
		log_error();
		GGI_vnc_close_client(client);
		return -1;
	case SSL_ERROR_SYSCALL:
		DPRINT("read error (%d %s)\n", errno, strerror(errno));
		log_error();
		GGI_vnc_close_client(client);
		return -1;
	default:
		DPRINT("read error\n");
		log_error();
		GGI_vnc_close_client(client);
		return -1;
	}

	if (len + client->buf_size > client->buf_limit) {
		DPRINT("Avoiding buffer overrun\n");
		GGI_vnc_close_client(client);
		return -1;
	}

	memcpy(client->buf + client->buf_size, buf, len);
	client->buf_size += len;

run_actions:
	if (client->action(client)) {
		GGI_vnc_close_client(client);
		return -1;
	}
	return 0;
}

static int
tls_write(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	/* DPRINT("tls_write()\n"); */

	if (!client->write_pending) {
		DPRINT("spurious write completed notification\n");
		return 0;
	}

	client->write_pending = 0;
	priv->del_cwfd(priv->gii_ctx, client, client->cwfd);

	if (client->safe_write(client, 1) < 0)
		return -1;

	if (client->write_pending)
		return 0;

	if (client->buf_size) {
		if (client->action(client)) {
			GGI_vnc_close_client(client);
			return -1;
		}
	}

	GGI_vnc_client_invalidate_nc_xyxy(client, 0, 0, 0, 0);
	return 0;
}

static int
tls_read_ready(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct vencrypt *vencrypt = client->vencrypt;

	/* DPRINT("tls_read_ready()\n"); */

	if (vencrypt->write_wants_read) {
		vencrypt->write_wants_read = 0;
		DPRINT("SSL_write gets to read\n");
		priv->add_cwfd(priv->gii_ctx, client, client->cwfd);
		if (tls_write(client))
			return -1;
		if (vencrypt->write_wants_read)
			return 0;
	}

	return tls_read(client);
}

static int
tls_write_ready(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_buf *buf = &client->wbuf;
	struct vencrypt *vencrypt = client->vencrypt;

	/* DPRINT("tls_write_ready()\n"); */

	if (vencrypt->read_wants_write) {
		vencrypt->read_wants_write = 0;
		DPRINT("SSL_read gets to write\n");
		priv->add_crfd(priv->gii_ctx, client);
		if (buf->size == buf->pos)
			priv->del_cwfd(priv->gii_ctx, client, client->cwfd);
		if (tls_read(client))
			return -1;
		if (vencrypt->write_wants_read || buf->size == buf->pos)
			return 0;
	}

	return tls_write(client);
}

static int
tls_safe_write(ggi_vnc_client *client, int close_on_error)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	ggi_vnc_buf *buf = &client->wbuf;
	struct vencrypt *vencrypt = client->vencrypt;
	SSL *ssl;
	int res;

	/* DPRINT("tls_safe_write()\n"); */

	if (client->write_pending) {
		DPRINT("impatient client...\n");
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	}

	BIO_get_ssl(vencrypt->ssl_bio, &ssl);
	if (!ssl) {
		DPRINT("No SSL object?\n");
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	}

again:
	res = SSL_write(ssl, buf->buf + buf->pos, buf->size - buf->pos);

	if (res == buf->size - buf->pos) {
		/* DPRINT("complete write\n"); */
		buf->size = buf->pos = 0;
		return res;
	}

	if (res > 0) {
		/* DPRINT("partial write\n"); */
		buf->pos += res;
		goto again;
	}

	switch (SSL_get_error(ssl, res)) {
	case SSL_ERROR_WANT_READ:
		DPRINT("SSL_write wants to read\n");
		client->write_pending = 1;
		vencrypt->write_wants_read = 1;
		priv->del_cwfd(priv->gii_ctx, client, client->cwfd);
		priv->del_crfd(priv->gii_ctx, client);
		priv->add_crfd(priv->gii_ctx, client);
		return 0;
	case SSL_ERROR_WANT_WRITE:
		/* DPRINT("SSL_write wants to write\n"); */
		client->write_pending = 1;
		priv->add_cwfd(priv->gii_ctx, client, client->cwfd);
		return 0;
	case SSL_ERROR_ZERO_RETURN:
		DPRINT("socket closed\n");
		log_error();
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	case SSL_ERROR_SYSCALL:
		DPRINT("write error (%d %s)\n", errno, strerror(errno));
		log_error();
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	case SSL_ERROR_WANT_X509_LOOKUP:
		DPRINT("x509 lookup\n");
		log_error();
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	case SSL_ERROR_SSL:
		DPRINT("SSL write error\n");
		log_error();
		if (close_on_error) {
			GGI_vnc_close_client(client);
			DPRINT("client closed\n");
		}
		return -1;
	default:
		DPRINT("write error\n");
		log_error();
		if (close_on_error)
			GGI_vnc_close_client(client);
		return -1;
	}
}

static struct vencrypt *
get_vencrypt(ggi_vnc_client *client)
{
	struct vencrypt *vencrypt;

	if (client->vencrypt)
		return client->vencrypt;

	vencrypt = malloc(sizeof(*vencrypt));
	if (!vencrypt)
		return NULL;
	memset(vencrypt, 0, sizeof(*vencrypt));

	return vencrypt;
}

static void
info_callback(SSL *ssl, int where, int ret)
{
	const char *str;
	int w = where & ~SSL_ST_MASK;

	if (w & SSL_ST_CONNECT)
		str = "SSL_connect";
	else if (w & SSL_ST_ACCEPT)
		str = "SSL_accept";
	else
		str = "undefined";

	if (where & SSL_CB_LOOP)
		DPRINT("%s: %s\n", str, SSL_state_string_long(ssl));
        else if (where & SSL_CB_ALERT)
		DPRINT("SSL3 alert %s: %s: %s\n",
			(where & SSL_CB_READ) ? "read" : "write",
			SSL_alert_type_string_long(ret),
			SSL_alert_desc_string_long(ret));
	else if (where & SSL_CB_EXIT) {
		if (ret == 0)
			DPRINT("%s: failed in %s\n",
				str, SSL_state_string_long(ssl));
		else if (ret < 0)
			DPRINT("%s: error in %s\n",
				str, SSL_state_string_long(ssl));
	}
	else if (where & SSL_CB_HANDSHAKE_START)
		DPRINT("handshake start: %s\n", SSL_state_string_long(ssl));
	else if (where & SSL_CB_HANDSHAKE_DONE)
		DPRINT("handshake done: %s\n", SSL_state_string_long(ssl));
	else
		DPRINT("%s: where %d ret %d\n", str, where, ret);
}

static void
msg_callback(int write_p, int version, int content_type,
	const void *buf, size_t len, SSL *ssl, void *arg)
{
	const char *str = write_p ? "write" : "read";
	const char *ver;
	const char *content;

	switch (version) {
	case SSL2_VERSION:
		ver = "SSL2";
		break;
	case SSL3_VERSION:
		ver = "SSL3";
		break;
	case TLS1_VERSION:
		ver = "TLS1";
		break;
	default:
		ver = "unknown";
	}

	switch (content_type) {
	case 20:
		content = "change_cipher_spec";
		break;
	case 21:
		content = "alert";
		break;
	case 22:
		content = "handshake";
		break;
	default:
		content = "unknown";
	}

	DPRINT("msg %s: %s %s: len %d\n", str, ver, content, len);
}

static DH *
tmp_dh_callback(SSL *ssl, int is_export, int keylength)
{
	DH *dh;
	DPRINT("generating dh parameters (%d)...\n", keylength);
	dh = DH_generate_parameters(keylength, 2, NULL, NULL);
	if (dh)
		DPRINT("...dh parameters (%d) generated\n", keylength);
	return dh;
}

static int
start_tls(ggi_vnc_client *client, const char *ciphers)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct vencrypt *vencrypt = client->vencrypt;
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;
	SSL *ssl;
	BIO *socket_bio = NULL;

	DPRINT("start_tls()\n");

	vencrypt->ssl_bio = BIO_new_ssl(ssl_ctx, 0);
	if (!vencrypt->ssl_bio) {
		DPRINT("Failed to create SSL BIO\n");
		log_error();
		goto err;
	}

	BIO_get_ssl(vencrypt->ssl_bio, &ssl);
	if (!ssl) {
		DPRINT("No SSL object in SSL BIO\n");
		log_error();
		goto err;
	}

	if (opt->ciphers)
		ciphers = opt->ciphers;
	if (!SSL_set_cipher_list(ssl, ciphers)) {
		DPRINT("Failed to set cipher list\n");
		log_error();
		goto err;
	}

	socket_bio = BIO_new_socket(client->cfd, BIO_NOCLOSE);
	if (!socket_bio) {
		DPRINT("Failed to create socket BIO\n");
		log_error();
		goto err;
	}

	if (!BIO_push(vencrypt->ssl_bio, socket_bio)) {
		DPRINT("Failed to join SSL and socket BIOs\n");
		log_error();
		goto err;
	}
	socket_bio = NULL;

	client->write_ready = tls_write_ready;
	client->read_ready = tls_read_ready;
	client->safe_write = tls_safe_write;
	DPRINT("tls up\n");
	return 0;

err:
	if (socket_bio)
		BIO_free_all(socket_bio);
	if (vencrypt->ssl_bio)
		BIO_free_all(vencrypt->ssl_bio);
	return -1;
}

void
GGI_vnc_vencrypt_stop_tls(ggi_vnc_client *client)
{
	struct vencrypt *vencrypt = client->vencrypt;

	if (!vencrypt)
		return;

	DPRINT("stop_tls()\n");

	if (vencrypt->ssl_bio)
		BIO_free_all(vencrypt->ssl_bio);

	free(vencrypt);
	client->vencrypt = NULL;
}

static int
vnc_client_vencrypt_tls_none(ggi_vnc_client *client)
{
	if (GGI_vnc_write_ready(client))
		return -1;
	if (client->write_pending)
		return 0;

	if (start_tls(client, "aNULL:!eNULL:@STRENGTH"))
		return -1;

	client->action = GGI_vnc_client_init;

	if (client->protover == 7) {
		if (client->buf_size)
			return GGI_vnc_client_init(client);
		client->buf_size = 0;
		return 0;
	}
	client->buf_size = 0;

	/* ok */
	GGI_vnc_buf_reserve(&client->wbuf, 4);
	client->wbuf.size += 4;
	insert_hilo_32(&client->wbuf.buf[0], 0);
	return client->safe_write(client, 1) < 0;
}

static int
vnc_client_vencrypt_tls_vnc_auth(ggi_vnc_client *client)
{
	if (GGI_vnc_write_ready(client))
		return -1;
	if (client->write_pending)
		return 0;

	if (start_tls(client, "aNULL:!eNULL:@STRENGTH"))
		return -1;

	return GGI_vnc_client_vnc_auth(client);
}

static int
vnc_client_vencrypt_x509_none(ggi_vnc_client *client)
{
	if (GGI_vnc_write_ready(client))
		return -1;
	if (client->write_pending)
		return 0;

	if (start_tls(client, "ALL:!aNULL:@STRENGTH"))
		return -1;

	client->action = GGI_vnc_client_init;

	if (client->protover == 7) {
		if (client->buf_size)
			return GGI_vnc_client_init(client);
		client->buf_size = 0;
		return 0;
	}
	client->buf_size = 0;

	/* ok */
	GGI_vnc_buf_reserve(&client->wbuf, 4);
	client->wbuf.size += 4;
	insert_hilo_32(&client->wbuf.buf[0], 0);
	return client->safe_write(client, 1) < 0;
}

static int
vnc_client_vencrypt_x509_vnc_auth(ggi_vnc_client *client)
{
	if (GGI_vnc_write_ready(client))
		return -1;
	if (client->write_pending)
		return 0;

	if (start_tls(client, "ALL:!aNULL:@STRENGTH"))
		return -1;

	return GGI_vnc_client_vnc_auth(client);
}

static int
vnc_client_vencrypt_subtype(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct vencrypt *vencrypt = client->vencrypt;
	uint32_t security_type;
	int msg_size;

	if (client->write_pending)
		return 0;

	DPRINT("client_vencrypt_subtype\n");

	if (vencrypt->version == 0x0001) {
		msg_size = 1;
		if (client->buf_size < 1)
			/* wait for more data */
			return 0;

		security_type = client->buf[0];
		if (security_type < 19 || security_type > 25)
			return -1;

		security_type += 256 - 19;
	}
	else {
		msg_size = 4;
		if (client->buf_size < 4)
			/* wait for more data */
			return 0;

		memcpy(&security_type, client->buf, sizeof(security_type));
		security_type = ntohl(security_type);
	}

	if (client->protover == 7 && security_type == 1)
		/* Client init msg may follow */
		memmove(client->buf, client->buf + msg_size,
			client->buf_size -= msg_size);
	else if (client->buf_size > msg_size) {
		DPRINT("Too much data.\n");
		return -1;
	}

	switch (security_type) {
	case 257:
		if (priv->passwd || priv->viewpw)
			break;

		priv->del_crfd(priv->gii_ctx, client);
		client->write_ready = vnc_client_vencrypt_tls_none;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 1);
		client->wbuf.size += 1;
		client->wbuf.buf[0] = 1;
		client->safe_write(client, 1);
		if (!client->write_pending)
			vnc_client_vencrypt_tls_none(client);
		return 0;

	case 258:
		if (!priv->passwd && !priv->viewpw)
			break;

		priv->del_crfd(priv->gii_ctx, client);
		client->write_ready = vnc_client_vencrypt_tls_vnc_auth;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 1);
		client->wbuf.size += 1;
		client->wbuf.buf[0] = 1;
		client->safe_write(client, 1);
		if (!client->write_pending)
			vnc_client_vencrypt_tls_vnc_auth(client);
		return 0;

	case 260:
		if (priv->passwd || priv->viewpw)
			break;

		priv->del_crfd(priv->gii_ctx, client);
		client->write_ready = vnc_client_vencrypt_x509_none;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 1);
		client->wbuf.size += 1;
		client->wbuf.buf[0] = 1;
		client->safe_write(client, 1);
		if (!client->write_pending)
			vnc_client_vencrypt_x509_none(client);
		return 0;

	case 261:
		if (!priv->passwd && !priv->viewpw)
			break;

		priv->del_crfd(priv->gii_ctx, client);
		client->write_ready = vnc_client_vencrypt_x509_vnc_auth;

		/* ok */
		GGI_vnc_buf_reserve(&client->wbuf, 1);
		client->wbuf.size += 1;
		client->wbuf.buf[0] = 1;
		client->safe_write(client, 1);
		if (!client->write_pending)
			vnc_client_vencrypt_x509_vnc_auth(client);
		return 0;
	}

	DPRINT("Invalid vencrypt sub-type requested (%u)\n", security_type);
	return -1;
}

static int
vnc_client_vencrypt_version(ggi_vnc_client *client)
{
	struct ggi_visual *vis = client->owner;
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	struct options *opt = priv->vencrypt;
	struct vencrypt *vencrypt = client->vencrypt;

	if (client->write_pending)
		return 0;

	DPRINT("client_vencrypt_version\n");

	if (client->buf_size > 2) {
		DPRINT("Too much data.\n");
		return -1;
	}

	if (client->buf_size < 2)
		/* wait for more data */
		return 0;

	client->action = vnc_client_vencrypt_subtype;
	client->buf_size = 0;

	memcpy(&vencrypt->version, client->buf, sizeof(vencrypt->version));
	vencrypt->version = ntohs(vencrypt->version);

	if (!vencrypt->version)
		return -1;

	if (vencrypt->version > 0x0002) {
		DPRINT("vencrypt version %d.%d not supported\n",
			vencrypt->version / 0x100,
			vencrypt->version % 0x100);
		GGI_vnc_buf_reserve(&client->wbuf, 1);
		client->wbuf.size += 1;
		client->wbuf.buf[0] = 0xff;
		client->safe_write(client, 1);
		return -1;
	}

	DPRINT("vencrypt version %d.%d\n",
		vencrypt->version / 0x100,
		vencrypt->version % 0x100);

	if (vencrypt->version == 0x0001) {
		GGI_vnc_buf_reserve(&client->wbuf, 3);
		client->wbuf.size += 3;
		client->wbuf.buf[0] = 0;
		client->wbuf.buf[1] = 1;
		if (!priv->passwd && !priv->viewpw) {
			if (opt->x509)
				client->wbuf.buf[2] = 23;
			else
				client->wbuf.buf[2] = 20;
		}
		else {
			if (opt->x509)
				client->wbuf.buf[2] = 24;
			else
				client->wbuf.buf[2] = 21;
		}
	}
	else {
		GGI_vnc_buf_reserve(&client->wbuf, 6);
		client->wbuf.size += 6;
		client->wbuf.buf[0] = 0;
		client->wbuf.buf[1] = 1;
		if (!priv->passwd && !priv->viewpw) {
			if (opt->x509)
				insert_hilo_32(&client->wbuf.buf[2], 260);
			else
				insert_hilo_32(&client->wbuf.buf[2], 257);
		}
		else {
			if (opt->x509)
				insert_hilo_32(&client->wbuf.buf[2], 261);
			else
				insert_hilo_32(&client->wbuf.buf[2], 258);
		}
	}

	client->safe_write(client, 1);
	return 0;
}

int
GGI_vnc_client_vencrypt_security(ggi_vnc_client *client)
{
	struct vencrypt *vencrypt;
	client->action = vnc_client_vencrypt_version;
	client->buf_size = 0;

	DPRINT("client_vencrypt_security\n");

	vencrypt = get_vencrypt(client);
	if (!vencrypt)
		return -1;
	client->vencrypt = vencrypt;

	GGI_vnc_buf_reserve(&client->wbuf, 2);
	client->wbuf.size += 2;
	client->wbuf.buf[0] = 0;
	client->wbuf.buf[1] = 2;
	client->safe_write(client, 1);
	return 0;
}

static int
pem_passwd_cb(char *buf, int size, int rwflag, void *userdata)
{
	/* No *good* way to have the "user" enter the PEM password.
	 * So, simply don't allow a PEM password...
	 */
	return 0;
}

void *
GGI_vnc_vencrypt_init(void)
{
	struct options *opt;

	opt = malloc(sizeof(*opt));
	if (!opt) {
		DPRINT("Out of memory\n");
		return NULL;
	}
	memset(opt, 0, sizeof(*opt));

	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();

	opt->ssl_ctx = SSL_CTX_new(TLSv1_server_method());

	if (!opt->ssl_ctx) {
		DPRINT("Failed to create SSL context\n");
		log_error();
		free(opt);
		return NULL;
	}

	SSL_CTX_set_info_callback(opt->ssl_ctx, info_callback);
	SSL_CTX_set_msg_callback(opt->ssl_ctx, msg_callback);

	SSL_CTX_set_mode(opt->ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
	SSL_CTX_set_mode(opt->ssl_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	SSL_CTX_set_default_passwd_cb(opt->ssl_ctx, pem_passwd_cb);

	return opt;
}

void
GGI_vnc_vencrypt_fini(ggi_vnc_priv *priv)
{
	struct options *opt = priv->vencrypt;

	if (!opt)
		return;

	if (opt->ssl_ctx)
		SSL_CTX_free(opt->ssl_ctx);
	if (opt->ciphers)
		free(opt->ciphers);
	free(opt);
	priv->vencrypt = NULL;
}

int
GGI_vnc_vencrypt_set_method(ggi_vnc_priv *priv, const char *method)
{
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;
	int ok;

	if (!method || !method[0])
		return GGI_OK;

	if (!strcmp(method, "TLSv1"))
		ok = SSL_CTX_set_ssl_version(ssl_ctx, TLSv1_server_method());
	else if (!strcmp(method, "SSLv2"))
		ok = SSL_CTX_set_ssl_version(ssl_ctx, SSLv2_server_method());
	else if (!strcmp(method, "SSLv3"))
		ok = SSL_CTX_set_ssl_version(ssl_ctx, SSLv3_server_method());
	else if (!strcmp(method, "SSLv23"))
		ok = SSL_CTX_set_ssl_version(ssl_ctx, SSLv23_server_method());
	else
		return GGI_EARGINVAL;

	if (!ok) {
		DPRINT("Failed to set SSL version\n");
		log_error();
		return GGI_ENOTFOUND;
	}

	return GGI_OK;
}

int
GGI_vnc_vencrypt_set_cert(ggi_vnc_priv *priv, const char *cert)
{
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;

	if (!cert || !cert[0])
		return GGI_OK;

	if (!SSL_CTX_use_certificate_chain_file(ssl_ctx, cert)) {
		DPRINT("Failed to load certificate chain\n");
		log_error();
		return GGI_ENOTFOUND;
	}

	opt->x509 = 1;

	return GGI_OK;
}

int
GGI_vnc_vencrypt_set_dh(ggi_vnc_priv *priv, const char *dh_file)
{
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;
	DH *dh;
	FILE *f;

	if (!dh_file || !dh_file[0]) {
		DPRINT("No dh params, generate on the fly...\n");
		SSL_CTX_set_tmp_dh_callback(ssl_ctx, tmp_dh_callback);
		return GGI_OK;
	}

	f = fopen(dh_file, "rt");
	if (!f) {
		DPRINT("Failed to open dh parameters \"%s\"\n", dh_file);
		return GGI_ENOFILE;
	}
	dh = PEM_read_DHparams(f, NULL, NULL, NULL);
	fclose(f);
	if (!dh) {
		DPRINT("Failed to read dh parameters\n");
		return GGI_ENOTFOUND;
	}
	if (!SSL_CTX_set_tmp_dh(ssl_ctx, dh)) {
		DH_free(dh);
		DPRINT("Failed to set dh parameters\n");
		log_error();
		return GGI_ENOTFOUND;
	}
	DH_free(dh);

	return GGI_OK;
}

int
GGI_vnc_vencrypt_set_ciphers(ggi_vnc_priv *priv, const char *ciphers)
{
	struct options *opt = priv->vencrypt;

	if (!ciphers || !ciphers[0])
		return GGI_OK;

	opt->ciphers = strdup(ciphers);
	if (!opt->ciphers)
		return GGI_ENOMEM;

	return GGI_OK;
}

int
GGI_vnc_vencrypt_set_priv_key(ggi_vnc_priv *priv, const char *priv_key)
{
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;

	if (!priv_key || !priv_key[0])
		return GGI_OK;

	if (!SSL_CTX_use_PrivateKey_file(ssl_ctx, priv_key, SSL_FILETYPE_PEM))
	{
		DPRINT("Failed to load private key\n");
		log_error();
		return GGI_ENOTFOUND;
	}

	return GGI_OK;
}

int
GGI_vnc_vencrypt_set_verify_locations(ggi_vnc_priv *priv,
	const char *file, const char *dir)
{
	struct options *opt = priv->vencrypt;
	SSL_CTX *ssl_ctx = opt->ssl_ctx;

	if (file && !file[0])
		file = NULL;
	if (dir && !dir[0])
		dir = NULL;

	if (!file && !dir) {
		SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
		return GGI_OK;
	}

	if (!SSL_CTX_load_verify_locations(ssl_ctx, file, dir)) {
		DPRINT("Failed to load verify locations\n");
		log_error();
		return GGI_ENOTFOUND;
	}

	SSL_CTX_set_verify(ssl_ctx,
		SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

	return GGI_OK;
}
