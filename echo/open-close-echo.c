#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<malloc.h>

#define DEVICE "/dev/echo"


int main(){
    int fd = 0;
    if(access(DEVICE, F_OK) == -1) {
        printf("module %s not loaded\n",DEVICE);
        return 0;
    }
    else {
        printf("\n module %s loaded\n",DEVICE);
        printf("\n device will open\n");
        fd = open(DEVICE, O_RDWR);
        printf("\n device will close\n");
        close(fd);
        return 0;
    }
}
