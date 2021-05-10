#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
#define DEVICE "/dev/echo"
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){

  int ret, fd;
  char stringToSend[BUFFER_LENGTH];

  printf("\n write & read started \n");
  fd = open(DEVICE, O_RDWR); // Open the device with read/write access
  if (fd < 0){
    perror("Failed to open the device...");
    return errno;
  }

  printf("\n Type in a short string to send to the kernel module:\n");
  scanf("%[^\n]%*c", stringToSend);    // Read in a string (with spaces)

  printf("\n Writing message to the device [%s] \n", stringToSend);
  ret = write(fd, stringToSend, strlen(stringToSend)); //Send the string to the LKM
  if (ret < 0){
    perror("Failed to write the message to the device.");
    return errno;
  }

  printf("\n ENTER to read back from the user \n");
  getchar();

  printf("\n Reading from the device \n");
  ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
  if (ret < 0){
    perror("Failed to read the message from the device.");
    return errno;
  }
  printf("\n The received message is: [%s]\n", receive);
  printf("\n write & read ended \n");
  return 0;
}
