
#ifndef MC_ENDPOINT_JU2N66SJ
#define MC_ENDPOINT_JU2N66SJ

struct bufferevent;
struct sockaddr;

struct mc
{
  struct peer {
    struct sockaddr *address;
    int len;
  } p;
  struct bufferevent *bev;
};

void mc_init(struct mc *, struct sockaddr *, int len, struct bufferevent *bev);
void mc_close(struct mc *);

#endif /* end of include guard: MC_ENDPOINT_JU2N66SJ */
