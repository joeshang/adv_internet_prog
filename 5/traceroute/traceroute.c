/**
 * File: traceroute.c
 * Author: Joe Shang (ID: 1101220731)
 * Brief: The implementation of simple traceroute program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define MAX_HOPS 30

#define TIME_OUT          -3
#define PORT_UNREACHABLE  -1
#define ICMP_TIMEOUT      -2

void trace(struct sockaddr_in target_addr);

int recvicmp(int sockfd,
             struct sockaddr_in *hopaddr,
             unsigned short s_port,
             unsigned short d_port,
             struct timeval *tv);


int main (int argc, char *argv[])
{

   struct hostent *host;
   struct sockaddr_in target_addr;
   char ip_str[INET_ADDRSTRLEN];
   const char *p;

   if (argc != 2)
   {
      fprintf(stderr, "Usage %s <hostname>\n", argv[0]);
      exit(1);
   }

   if ((host = gethostbyname(argv[1])) == NULL)
   {
     perror("gethostbyname");
     exit(1);
   }

   bzero(&target_addr, sizeof(struct sockaddr_in));
   target_addr.sin_family = AF_INET; 
   target_addr.sin_addr = *(struct in_addr *)(host->h_addr);

   p = inet_ntop(AF_INET, &target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);   
   if (p == NULL){
     perror("inet_ntop");
     exit(1);
   }

   printf("traceroute to %s (%s), %d hops max, %d byte packets\n",
	   argv[1], p, MAX_HOPS, sizeof(struct iphdr) + sizeof(struct udphdr));

   trace(target_addr);   
   return 0;
}

void timesub(struct timeval *t1, struct timeval *t2)
{
   if (t2->tv_usec < t1->tv_usec)
   {
      t2->tv_sec = t2->tv_sec - t1->tv_sec -1;
      t2->tv_usec = t2->tv_usec + 1000000 - t1->tv_usec;
   }
   else
   {
      t2->tv_sec = t2->tv_sec - t1->tv_sec;
      t2->tv_usec = t2->tv_usec - t1->tv_usec;
   }
} 


void trace(struct sockaddr_in target_addr)
{
   int sockfd;
   int recvfd;
   int ttl = 0;
   int probe = 0;

   struct sockaddr_in udpaddr;
   struct sockaddr_in local_addr;
   struct sockaddr_in hopaddr;
   struct sockaddr_in oldaddr;
   
   struct timeval tv;
   struct timeval tv2;

   int code;
   int done = 0;
   unsigned short s_port;
   unsigned short d_port;
  
   char ip_str[INET_ADDRSTRLEN];
  
   s_port = (getpid() & 0xffff) | 0x8000;
   d_port = s_port + 200;

   bzero(&udpaddr, sizeof(struct sockaddr_in));
   bzero(&local_addr, sizeof(struct sockaddr_in));

   local_addr.sin_family = AF_INET;
   local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   local_addr.sin_port = htons(s_port);
  
   udpaddr.sin_family = AF_INET;
   udpaddr.sin_addr = target_addr.sin_addr;
   udpaddr.sin_port = htons(d_port);
   
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if (sockfd == -1)
   {
     perror("udp socket");
     exit(1);
   }

   recvfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
   if (recvfd == -1)
   {
     perror("recvfd");
     exit(1);
   }

   if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1) 
   {
     perror("bind");
     exit(1);
   }

   for (ttl = 1; ttl <= MAX_HOPS && (done == 0); ttl++)
   { 
      printf("%2d ",ttl);
      fflush(stdout);

      for (probe = 0; probe < 3; probe++)
      {
          if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) == -1) 
          {
             perror("setsockopt");
             exit(1);
          }

         gettimeofday(&tv,NULL);
         
         if (sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&udpaddr,
                sizeof(struct sockaddr)) == -1)
         {
            perror("sendto");
            exit(1);    
         }

         code = recvicmp(recvfd, &hopaddr, s_port, d_port, &tv2);
         alarm(0);

         if (code == TIME_OUT )
         {
            printf(" *");
         }

         if (code == PORT_UNREACHABLE ) 
         {            
            done = 1; 
         }

         if ((code == ICMP_TIMEOUT) || (done == 1))
         {
             if (oldaddr.sin_addr.s_addr != hopaddr.sin_addr.s_addr) 
             {
                printf(" %s",inet_ntop(AF_INET,
                                       &(hopaddr.sin_addr),
                                       ip_str, INET_ADDRSTRLEN));
                memcpy(&oldaddr, &hopaddr,sizeof(struct sockaddr_in));
             }        
             timesub(&tv, &tv2);
             printf(" %3f ms", tv2.tv_sec * 1000.0 + tv2.tv_usec / 1000.0);
           
         }
         code = 0; 
         fflush(stdout);
      }
   
      printf("\n");
   }
   close(sockfd);
   close(recvfd);
}

void sighandler(int signum)
{
  return;
}

int recvicmp(int sockfd,
             struct sockaddr_in *hopaddr,
             unsigned short s_port,
             unsigned short d_port,
             struct timeval *tv)
{

   socklen_t socklen;
   char recv_buf[BUF_SIZE];
   int recv_len;

   struct iphdr *ip;
   struct udphdr *udp;
   struct icmp *icmp;
   int ip_len;
   int ip_len2;

   signal(SIGALRM, sighandler);
   siginterrupt(SIGALRM, 1);
   alarm(3);
   
   for(;;)
   {
     socklen = sizeof(struct sockaddr);
     recv_len = recvfrom(sockfd, recv_buf, BUF_SIZE, 0,
               (struct sockaddr *)hopaddr, &socklen);

     if (recv_len < 0 ) 
     {
        if (errno == EINTR)
        {
           return TIME_OUT;
        }
        else
        {
           exit(1);
        }
     }

     if (recv_len < 28) 
     {
        printf("error in receiving ip packet\n");
        exit(1);
     }

     gettimeofday(tv, NULL);

     ip = (struct iphdr *)recv_buf;     
     ip_len = (ip->ihl) << 2;
     
     if (ip_len < 20)
     {
        printf("ip header length is error\n");
        exit(1);
     }

     icmp = (struct icmp *)(recv_buf + ip_len);
     ip = (struct iphdr *)(recv_buf + ip_len + sizeof(struct icmphdr));
     
     ip_len2 = ip->ihl << 2;
     udp = (struct udphdr *)(recv_buf + ip_len + 
                            sizeof(struct icmphdr)+ ip_len2);

     if ((ip->protocol == IPPROTO_UDP) &&     
          (ntohs(udp->source) == (s_port)) &&
          (ntohs(udp->dest) == (d_port))) 
     {
       if ((icmp->icmp_type == ICMP_TIMXCEED) &&
           (icmp->icmp_code == ICMP_TIMXCEED_INTRANS))
       {
           return ICMP_TIMEOUT;
       }
       else if(icmp->icmp_type == ICMP_UNREACH)
       {
           return PORT_UNREACHABLE;
       }
     }
   }

   return 0;    
}


