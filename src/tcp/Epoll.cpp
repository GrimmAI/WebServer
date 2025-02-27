#include "Epoll.h"
#include <sys/epoll.h>

Epoll::Epoll() {
    epfd = epoll_create1(0);
    

}

Epoll::Epoll(int epfd_) {
    epfd = epfd_;
}

Epoll::~Epoll() {

}

