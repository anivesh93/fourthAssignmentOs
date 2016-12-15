#ifndef libnetfiles
#define libnetfiles

int netserverinit(char *hostname, char *mode);
int netopen(const char *pathname, int flags);
int netread(int netfd, char *buffer, int num_bytes);
int netwrite(int netfd, const void *buf, int nbyte);
#endif