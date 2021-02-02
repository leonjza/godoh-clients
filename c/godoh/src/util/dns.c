#include "dns.h"

#include "debug.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>  //windows
#include <windns.h>   //DNS api's
#include <stdio.h>    //standard i/o
#include <winsock.h>  //winsock
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

// basically:
//  https://github.com/microsoft/Windows-classic-samples/blob/master/Samples/Win7Samples/netds/dns/dnsquery/DNSQuery.Cpp

char *dns_raw_txt_lookup(const char *domain) {

    DNS_STATUS status;               // return value of  DnsQuery_A() function.
    PDNS_RECORD pDnsRecord;          //pointer to DNS_RECORD structure
    LPTSTR pOwnerName = NULL;        //owner name to be queried
    DNS_FREE_TYPE freetype;
    freetype =  DnsFreeRecordListDeep;
    IN_ADDR ipaddr;

    pOwnerName = (char *)domain;

    status = DnsQuery_A(pOwnerName, DNS_TYPE_TEXT, DNS_QUERY_BYPASS_CACHE, NULL, &pDnsRecord, NULL);

    if (status) {
        Dprintf("[d] failed to query record %s\n", domain);
        return NULL;
    }

    if(pDnsRecord->Data.TXT.dwStringCount < 1) {
        return NULL;
    }

    // we only care about the first response
    char *txt = calloc(255, sizeof(char *));
    strncpy(txt, pDnsRecord->Data.TXT.pStringArray[0], strlen(pDnsRecord->Data.TXT.pStringArray[0]));

    Dprintf("[d] txt response has %lu records\n", pDnsRecord->Data.TXT.dwStringCount);

    return txt;
}

char *dns_raw_a_lookup(const char *domain) {

    DNS_STATUS status;               // return value of  DnsQuery_A() function.
    PDNS_RECORD pDnsRecord;          //pointer to DNS_RECORD structure
    LPTSTR pOwnerName = NULL;        //owner name to be queried
    DNS_FREE_TYPE freetype;
    freetype =  DnsFreeRecordListDeep;
    IN_ADDR ipaddr;

    pOwnerName = (char *)domain;

    status = DnsQuery_A(pOwnerName, DNS_TYPE_A, DNS_QUERY_BYPASS_CACHE, NULL, &pDnsRecord, NULL);

    if (status) {
        Dprintf("[d] failed to query record %s\n", domain);
        return NULL;
    }

    ipaddr.S_un.S_addr = (pDnsRecord->Data.A.IpAddress);

    int ip_len = strlen(inet_ntoa(ipaddr));
    char *res = calloc(ip_len, sizeof (char *));
    strncpy(res, inet_ntoa(ipaddr), ip_len);

    DnsRecordListFree(pDnsRecord, freetype);

    return res;
}

#else

#include <resolv.h>
#include <libc.h>

char *dns_raw_txt_lookup(const char *domain) {

    u_char answer[255]; // a single TXT response string is max 255
    int len;
    ns_msg msg;
    ns_rr rr;

    char *txt = calloc(255, sizeof(char *));

    len = res_query(domain, ns_c_in, ns_t_txt, answer, sizeof(answer));

    if (len <= 0)
        return txt;

    if (ns_initparse(answer, len, &msg) < 0)
        return txt;

    if (ns_msg_count(msg, ns_s_an) > 1)
        Dprintf("[d] dns response had more than 1 answers. we are only taking the first\n");

    if (ns_parserr(&msg, ns_s_an, 0, &rr)) // take the first rr, 0
        return txt;

    if (ns_rr_type(rr) != ns_t_txt)
        return txt;

    // first byte seems to be a size byte maybe?
    strncpy(txt, (char *) ns_rr_rdata(rr) + 1, ns_rr_rdlen(rr));

    return txt;

}

char *dns_raw_a_lookup(const char *domain) {

    u_char *response = calloc(255, sizeof(unsigned char *));
    ns_msg handle;
    ns_rr rr;

    Dprintf("[d] doing a lookup for: %s\n", domain);

    int len = res_query(domain, ns_c_in, ns_t_a, response, 255);
    if (len < 0) {
        Dprintf("[d] resp len was: %d\n", len);
    }

    ns_initparse(response, len, &handle);
    if (ns_msg_count(handle, ns_s_an) == 0) {
        Dprintf("[d] ! answer had no resource records\n");
        return NULL;
    }

    if (ns_parserr(&handle, ns_s_an, 0, &rr) < 0) {
        Dprintf("[d] ! could not parse the first answer section\n");
        return NULL;
    }

    char *answer = calloc(255, sizeof(char *));
    strcpy(answer, inet_ntoa(*(struct in_addr *) ns_rr_rdata(rr)));

    Dprintf("[d] a answer was: %s\n", answer);

    return answer;
}

#endif  // else for linux/macos
