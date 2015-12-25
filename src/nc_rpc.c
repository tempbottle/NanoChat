#include "nc.h"

void*
nc_rpc_loop(void *void_opts)
{
  nc_opts *opts = (nc_opts*) void_opts;
  int sock = nn_socket(AF_SP, NN_REP);
  nn_bind(sock, opts->url);
  nc_log_writef("info", "RPC was started on URL: %s.", opts->url);
  
  for(;;) {
    char *buf = NULL;
    int bytes = nn_recv(sock, &buf, NN_MSG, 0);
    int cmp_rc;
    
    nc_log_writef("info", "Incoming RPC request: %s.", buf);

    cmp_rc = strncmp(buf, RCMD_OTOC, RCMD_LEN);

    if(cmp_rc == 0) {

      /* one to one chat */
      
      char otoc_url[URL_MAX];
      static int otoc_port_init = INIT_OTOC_PORT;
      char otoc_port[OTOC_PORT_LEN];
      int otoc_sock = nn_socket(AF_SP, NN_PAIR);

      snprintf(otoc_port, OTOC_PORT_LEN, "%d", otoc_port_init++);
      nc_utils_make_url(otoc_url, opts->host, otoc_port);
      nn_bind(otoc_sock, otoc_url);

      nc_log_writef("info", "New oto chat was connected on URL: %s.", otoc_url);
      nn_send(sock, otoc_port, OTOC_PORT_LEN /*(int) strlen(otoc_port) */, 0);

      nc_dal_save_otoc(otoc_sock);
      
      fprintf(stdout, "[*] You have new event.\n>>");
      fflush(stdout);

    } else {

      /* unknown command */
      
      nn_send(sock, buf, bytes, 0);
    }

    nc_utils_empty_string(buf);
    nn_freemsg(buf);
  }
  return NULL;
}

void
nc_rpc_start(nc_opts *opts)
{
  pthread_t rpc_loop;
  pthread_create(&rpc_loop, NULL, nc_rpc_loop, opts);
}
