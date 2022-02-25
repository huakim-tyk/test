
// sockaddr_in, sockaddr, 
#include <netinet/in.h>
// inet_aton, inet_ntoa
#include <arpa/inet.h>
// gethostbyname, hostent
#include <netdb.h>
// getpid
#include <unistd.h>
// timeval, gettimeofday, time
#include <sys/time.h>
#include <time.h>
// srand, rand
#include <stdlib.h>
// printf
#include <iostream>
// thread
#include <thread>
// mutex 
#include <mutex>
// signal, kill
#include <signal.h>
// cout, endl
#include <iostream>
//
#include <string.h>
//
using namespace std;
namespace interrupt{
    const char * path;
    int * sent;
    int * received;
    void handler(int dummy){
        int snt = *sent;
        int rec = *received;
        cout<<endl
            <<"==="
            <<path
            <<" statistics==="
            <<endl
            <<snt
            <<" packets transmitted, "
            <<rec
            <<" packets received, "
            <<((snt!=0)?((snt-rec)/snt)*100.0:0.0)
            <<" percent packet lost"
            <<endl;
        exit(0);
    }
}

// заполнить адрес
bool dns_lookup(const char *addr_host, sockaddr_in* addr_con){
    //
    addr_con->sin_port = htons(8080);
    //
	struct hostent *host_entity;
    //
    if (inet_aton(addr_host, &addr_con->sin_addr) > 0){
        //
        addr_con->sin_family = AF_INET;
        return true;
    }
    //
	if ((host_entity = gethostbyname(addr_host)) == nullptr){
		// адрес не найден
		return false;
	}
	
	// 
	addr_con->sin_family = host_entity->h_addrtype;
	addr_con->sin_addr.s_addr = *(long*)host_entity->h_addr;
    //
	return true;	
}

#define MTU 1500
#define RECV_TIMEOUT_USEC 100000

struct icmp{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t ident;
    uint16_t seq;
};

struct ip
{
    uint8_t VIHL;
    uint8_t TOS;
    uint16_t TotLen;
    uint16_t id;
    uint16_t flagoff;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    struct in_addr iaSrc;
    struct in_addr iaDst;
};

// функция вычисляет хеш сумму
uint16_t calculate_checksum(u_char* buffer, int bytes){
    uint32_t checksum = 0;
    //
    u_char* end = buffer + bytes;
    // 
    if (bytes % 2 == 1) {
        end = buffer + bytes - 1;
        checksum += (*end) << 8;
    }
    // 
    while (buffer < end) {
        checksum += buffer[0] << 8;
        checksum += buffer[1];
        buffer += 2;
    }
    // 
    uint32_t carray = checksum >> 16;
    while (carray) {
        checksum = (checksum & 0xffff) + carray;
        carray = checksum >> 16;
    }
    // 
    checksum = ~checksum;
    return checksum & 0xffff;
};

// функция генерирует случайный массив латинских букв
void random_chars(char * str, int size){
    //
    srand(time(nullptr));
    // продолжить пока size > 0
    while (size > 0){
        // присвоить рандомный номер
        *str = 'a' + (rand()%26);
        // увеличить указатель
        str++;
        size--;
    }
}

// функция отправляет пакет на сервер
int send_echo_request(int sock, struct sockaddr_in* addr, int ident, 
    int seq, char * data, int data_len, timeval*start, timeval*stop){
    //
    auto ic = (icmp*) data;
    ic->checksum = 0;
    // заполнить icmp
    ic->type = 8;
    ic->code = 0;
    ic->ident = htons(ident);
    ic->seq = htons(seq);

    // вычислить хеш-сумму и записать
    ic->checksum = htons(calculate_checksum((u_char*)data, data_len));

    gettimeofday(start, nullptr);
    // оптравить
    int bytes = sendto(sock, data, data_len, 0,
        (struct sockaddr*)addr, sizeof(*addr));
    //
    gettimeofday(stop, nullptr);
    //
    if (bytes == -1) return -1;
    //
    return 0;
}

int recv_echo_reply(int sock, int ident, in_addr_t t, int seq, int *i, 
        timeval*start, timeval*stop, char * buffer, int reply_size){
    struct sockaddr_in peer_addr;

    // receive another packet
    socklen_t addr_len = sizeof(peer_addr);
    //
    gettimeofday(start, nullptr);
    // bytes
    int bytes = recvfrom(sock, buffer, reply_size, 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    //
    gettimeofday(stop, nullptr);
    //
    if (bytes == -1) {
        // normal return when timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return -1;
    }

    // find icmp packet in ip packet
    auto ic = (struct icmp*)(buffer + 20);

    // проверить тип
    if (ic->type != 0 || ic->code != 0) {
        return 1;
    }
    
    // проверить идентификатор
    if (ntohs(ic->ident) != ident) {
        return 1;
    }
    
    // проверить seq
    if (seq == ic->seq){
        return 1;
    }
    
    // проверить адрес
    if (t != peer_addr.sin_addr.s_addr);
    //
    *i = bytes;

    return 0;
}

void dns_error(const char * name){
    cout<<"cannot find name "<< name <<endl;
}

void sock_error(){
    cout<<"cannot create raw socket"<<endl;
}

void dest_rec(){
    cout<<"destination address required"<<endl;
}

void time_error(timeval tv){
    cout<<"cannot set timeout of "<<
    ((tv.tv_sec*1000000)+tv.tv_usec)<<" ms"<<endl;
}

void send_error(){
    cout<<"send failure"<<endl;
}

void ttl_error(int ttl){
    cout<<"cannot set ttl of "<<ttl<<endl;
}

void recv_error(){
    cout<<"receive failure"<<endl;
}

// 
void sleep(timeval&start, timeval&stop, timeval&max){
    // 
    int i;
    //
    i = max.tv_sec - (stop.tv_sec - start.tv_sec);
    if (i > 0) sleep(i);
     //
    i = max.tv_usec - (stop.tv_usec - start.tv_usec);
    if (i > 0) usleep(i);
}

// pind
void ping(const char *name, int size = 44, int ttl = 65, int count = -1,
          timeval tv = {0, 100000}, timeval step = {1, 0}){
    // создать буфер
    auto buf = new char[size];
    // 
    int reply_size = size + 512;
    // создать буфер ответа
    auto reply = (ip*) new char[reply_size];
    // получить и проверить адрес
    struct sockaddr_in addr;
    if (!dns_lookup(name, &addr)) {dns_error(name); return;};
    // создать сокет для протокола icmp
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // проверить сокет
    if (sock == -1) {sock_error(); return;};
    // присвоить значение ttl (time to live)
    if (setsockopt(sock,SOL_IP, IP_TTL, &ttl, sizeof(int))==-1){
        ttl_error(ttl); return;
    };
    // присвоить таймаут
    if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))==-1){
        time_error(tv); return;
    };
    
    timeval start1 = {0,0}, 
            start2 = {0,0}, 
            stop1  = {0,0}, 
            stop2  = {0,0};
    //

    int pid = getpid();
    
    bool run = true;
    ulong seq = 0;
    //
    int i = 0, bytes = 0;
    // пропустить ожидание
    // create lock
    auto lock = new mutex;
    // run thread
    new thread([](mutex*lock, timeval * vl)->void{
        l1:
        // sleep
        sleep(vl->tv_sec);
        usleep(vl->tv_usec);
        // unlock thread
        lock->unlock();
        goto l1;
    }, lock, &step);
    //
    int sent = 0;
    int received = 0;
    //
    interrupt::sent = &sent;
    interrupt::received = &received;
    interrupt::path = name;
    //
    signal(SIGINT, interrupt::handler);
    //
    l1:{
        //
        if (count == 0) return;
        else if (count > 0) count --;
        //
        if (i == 1) { goto l2; }
        // увеличить seq
        seq ++;
        //
        lock->lock();
        // создать случайную последовательность
        random_chars(buf, size);
        //
        gettimeofday(&start1, nullptr);
        // отправить пакет
        if (send_echo_request(sock, &addr, pid, 
                        seq, buf, size, &start1, &stop1)==-1)
        // вывести возможную ошибку
            {send_error(); goto l1; }
        else{
            sent ++;
        }
        gettimeofday(&stop1, nullptr);
        l2:
        
        // 
        gettimeofday(&start2, nullptr);
        // получить пакет
        if ((i = recv_echo_reply(sock, pid, addr.sin_addr.s_addr, seq, 
            &bytes, &start2, &stop2, (char*)reply, reply_size))==-1)
        // вывести возможную ошибку
            {recv_error(); goto l1; }
        else if (i == 0){
            received ++;
        };
        //
        gettimeofday(&stop2, nullptr);
        
        cout<<bytes
            <<" bytes from "
            <<inet_ntoa(reply->iaSrc)
            <<": icmp_seq="
            <<seq
            <<" ttl="
            <<(unsigned int)reply->ttl
            <<" time="
            <<((stop1.tv_sec-start1.tv_sec+
                stop2.tv_sec-start2.tv_sec)*1000000 +
               (stop1.tv_usec-start1.tv_usec+
                stop2.tv_usec-start2.tv_usec))
            <<" ms"<<endl;  
      //
        goto l1;
    } 

    return ;
}

void ms_to_timeval(int ms, timeval* tv){
    tv->tv_sec  = ms / 1000000;
    tv->tv_usec = ms % 1000000;
}

int main(int argc, char** argv){
    //
    int size = 44; 
    int ttl = 65;
    int count = -1;
    // 
    timeval tv = {0, 100000};
    //  
    timeval step = {1, 0};
    //
    char * path = nullptr;
    //
    char * u;
    //
    char operation = 0;;
    //
    for (int i = 1; i < argc; i ++){
        //
        argv++;
        //
        u = *(argv);
        // проверить, начинается ли выражение с -
        // если да, то выполнить операцию
        if (u[0] == 0) continue;
        if (u[0] == '-'){
            // если длина строки 2, то проверить выражение
            if (strlen(u)==2){
                //
                operation = u[1];
                //
                if (operation == 'h'){
cout<<endl<<"Usage"<<endl<<"  ping [options] <destination>"
<<endl<<endl<<"Options:"
<<"  <destination>      dns name or ip address"<<endl
<<"  -c <count>         stop after <count> replies"<<endl
<<"  -l <size>          use <size> as number of data bytes to be sent"
<<endl<<"  -t <ttl>           define time to live"<<endl
<<"  -w <timeout>       time to wait for response"<<endl<<
"  -i <interval>      seconds between sending each packet"<<endl<<endl;
                    exit(0);
                }
                else{
                    
        argv++; i++;
        // 
        if (i == argc) break;
        //
        if (operation != 0){
            switch (operation){
                case 't':
                ttl = atoi(*argv);
                break;
                case 'l':
                size = atoi(*argv);
                break;
                case 'i':
                ms_to_timeval(atoi(*argv), &step);
                break;
                case 'c':
                count = atoi(*argv);
                break;
                case 'w':
                ms_to_timeval(atoi(*argv), &tv);
                default:
                break;
            };
            operation = 0;
            continue;
        }
                }
            }//
            //
            continue;
        }
        //
        if (path == nullptr){
            path = u;
        }
    }
    //
    ping(path, size, ttl, count, tv, step);
    //
    kill(getpid(), SIGINT);
}

