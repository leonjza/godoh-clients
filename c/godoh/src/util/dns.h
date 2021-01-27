#ifndef GODOH_DNS_H
#define GODOH_DNS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else

char *dns_raw_txt_lookup(const char *domain);

char *dns_raw_a_lookup(const char *domain);

#endif

#endif //GODOH_DNS_H
