#include "Channel.h"
#include "Epoll.h"



void Channel::enableReading(){
    events = EPOLLIN | EPOLLET;
    ep->updateChannel(this);
}