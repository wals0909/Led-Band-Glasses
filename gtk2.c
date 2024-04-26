#include <gtk/gtk.h>
#include <ftdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define DOT   100000
#define DASH  300000
#define SLOW  500000
#define DORK  150000
#define DROOL 1000000
#define DAB   50000
#define SLP   40000

long millisec();
uint8_t sorc[108] = { };
uint8_t dest[108] = { };

gboolean loop1 = false;
pthread_t thread;

struct ftdi_context* create_new_ftdi(); 

// Headers
static void activate (GtkApplication *app, gpointer user_data);


int main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}

 // initialize new ftdi device
struct ftdi_context* create_new_ftdi() {

  struct ftdi_context *ftdi;
  struct ftdi_device_list *devlist, *curdev;
  char manufacturer[224], description[224];
  int retval = EXIT_SUCCESS;
  int i = 0;
  int ret = 0;
  int res;
  int f;

  if ((ftdi = ftdi_new()) == 0) {
    fprintf(stderr, "ftdi_new failed\n");
      
  } else {
    fprintf(stderr, "ftdi_new success\n");
  }

  // detect connected ftdi device(s)
  if ((res = ftdi_usb_find_all(ftdi, &devlist, 0x0403, 0x6001)) <0) {
    fprintf(stderr, "no ftdi devices found\n");
    fflush(stderr);
    ftdi_list_free(&devlist);
    ftdi_free(ftdi);
   
  } else {
    fprintf(stderr, "%d ftdi devices found.\n", res);
  }

  // loop through detected devices and attempt to get their information
  i = 0;
  for (curdev = devlist; curdev != NULL; i++) {
    printf("Checking device: %d\n", i);
    if ((ret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 224, description, 224, NULL, 0)) < 0) {
      fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
      retval = EXIT_FAILURE;
      ftdi_list_free(&devlist);
      ftdi_free(ftdi);
    }
    printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
    curdev = curdev->next;
  }

  // open ftdi context
  if ((ret = ftdi_usb_open_dev(ftdi, devlist->dev)) < 0)
    {
      fprintf(stderr, "unable to open ftdi: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
      ftdi_free(ftdi);
                
    }
  else {
    fprintf(stderr, "ftdi_open successful\n");
  }

  // set the base bitrate/baudrate of the device(s)
  ret = ftdi_set_baudrate(ftdi, 57600);
  if (ret < 0) {
    fprintf(stderr, "unable to set baud rate:\n");
  } else {
    printf("baudrate set.\n");
  }

  // set parameters in the devices
  f = ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
  if(f < 0) {
    fprintf(stderr, "unable to set line parameters: %d (%s).\n", ret, ftdi_get_error_string(ftdi));
  } else {
    printf("line parameters set.\n");
  }

  ftdi_list_free(&devlist);

  return ftdi;
}

// This routine rotates the values in the lower 13 channels (channels 0-12)
void rotate13(uint8_t arr[]) {
    uint8_t temp = arr[0]; // Store the first value
    for (int i = 0; i < 12; i++) {
        arr[i] = arr[i + 1]; // Shift all values to the left
    }
    arr[12] = temp; // Move the stored value to the last position
}

static size_t getEncodedBufferSize(size_t sourceSize) {
  size_t s;
  s = sourceSize + sourceSize / 254 + 1;
  //printf("buffer size is : %zd.\n", s);
  return s;
}

// Please note that the colors have been bit shifted. 
// The atmega328 has a typical 3 pin output, sending 3 bytes.
// The binary would look like: 00000000 00000000 11111111 or (0,0,255)
// All RGB colors is now being sent into 1 byte making the binary: 000000111. or (7)

void test_on (GtkWidget *widget, gpointer  data)
{
  int ret = 0;
  int nbytes;
  //int r = 224;
  //int g = 28;
  //int b = 3;
  //int y = 252;

  uint8_t testPack[108] = {
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,};

  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  nbytes = ftdi_write_data(ftdi1, testPack, m);
  
  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void test_off (GtkWidget *widget, gpointer  data)
{
  
  int ret = 0;
  int nbytes;
 
  uint8_t dbytePack[108] = {0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0};

  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  if (loop1) {
    
  loop1 = false;

  pthread_cancel(thread); // Cancel the thread

  }

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  
  nbytes = ftdi_write_data(ftdi1, dbytePack, m);
  


  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void xmas_pulse (GtkWidget *widget, gpointer  data)
{
  int ret = 0;
  int nbytes;
  int i = 0; 
  //int r = 224;
  //int g = 28;
  //int b = 3;
  //int y = 252;
 uint8_t dPack[108] = { };

 uint8_t X1Pack[108] =  {  224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28, 
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,};

 uint8_t X2Pack[108] =  {  252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,};  

 uint8_t X3Pack[108] =  {  224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,};

 uint8_t X4Pack[108] =  {  28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252, 
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,};


  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  //nbytes = ftdi_write_data(ftdi1, testPack, m);

  for (i = 0; i < 2; i++) {
        nbytes = ftdi_write_data(ftdi1, X1Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X2Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X3Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X4Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
      }

  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}


void *xmas_sparkle_loop (void *data)
{
  int ret = 0;
  int nbytes;
  int i = 0; 

 uint8_t dPack[108] = { };

 uint8_t X1Pack[108] =  {  224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28, 
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,};

 uint8_t X2Pack[108] =  {  252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,};  

 uint8_t X3Pack[108] =  {  224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,};

 uint8_t X4Pack[108] =  {  28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252, 
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,};


  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  

  while(loop1) {
        nbytes = ftdi_write_data(ftdi1, X1Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X2Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X3Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, X4Pack, m);
        usleep(SLP);
        nbytes = ftdi_write_data(ftdi1, dPack, m);
        usleep(SLP);
      }

  return NULL;

  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void xmas_sparkle_button1(GtkWidget *widget, gpointer data) {
    // Start a new thread for the loop
    loop1 = TRUE;
    pthread_create(&thread, NULL, xmas_sparkle_loop, NULL);
}

void *slow_xmas_sparkle_loop2(void *data) {

  int ret = 0;
  int nbytes;
  int i = 0; 

 uint8_t dPack[108] = { };

 uint8_t X1Pack[108] =  {  224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28, 
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,
                           224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                           28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                           224,28,224,  28,224,28,   252,252,224, 28,224,28,};

 uint8_t X2Pack[108] =  {  252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,  
                           252,224,28,  224,28,224,  28,224,28,   252,224,28,  
                           224,28,224,  252,28,224,  28,224,28,   224,28,252,  
                           252,224,28,  224,28,224,  28,252,28,   224,28,224,};  

 uint8_t X3Pack[108] =  {  224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,
                           224,28,252,  252,224,28,  224,28,224,  28,224,28, 
                           252,224,28,  224,28,224,  252,28,224,  28,224,28, 
                           224,28,252,  252,224,28,  224,28,252,  28,224,28,};

 uint8_t X4Pack[108] =  {  28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252, 
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,
                           28,224,28,   224,28,252,  252,224,28,  224,28,224, 
                           28,224,28,   252,224,28,  224,28,224,  252,28,224, 
                           28,224,28,   224,28,252,  252,224,28,  252,28,252,};


  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  

  while(loop1) {
        nbytes = ftdi_write_data(ftdi1, X1Pack, m);
        usleep(DASH);
        usleep(DASH);
        nbytes = ftdi_write_data(ftdi1, X2Pack, m);
        usleep(DASH);
        usleep(DASH);
        nbytes = ftdi_write_data(ftdi1, X3Pack, m);
        usleep(DASH);
        usleep(DASH);
        nbytes = ftdi_write_data(ftdi1, X4Pack, m);
        usleep(DASH);
        usleep(DASH);
        
      }

  return NULL;


  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n"); 
  
}

void xmas_sparkle_button2(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, slow_xmas_sparkle_loop2, NULL);
}

void green_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t greenPack[108] = { 28,28,28,   28,28,28,  28,28,28,  28,28,28, 
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28, 
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,
                           28,28,28,   28,28,28,  28,28,28,  28,28,28,};
size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, greenPack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);
// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void twnk8_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t twnk8Pack[108] = { 252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                           128,28,80,   128,248,80,  128,224,80,  28,252,80,
                           128,144,80,   128,252,80,  128,72,80,  252,128,80,
                           252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                           128,28,80,   128,248,80,  128,224,80,  28,252,80,
                           128,144,80,   128,252,80,  128,72,80,  252,128,80,
                           252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                           128,28,80,   128,248,80,  128,224,80,  28,252,80,
                           128,144,80,   128,252,80,  128,72,80,  252,128,80,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, twnk8Pack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);
// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void goldPack_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t goldPack[108] = { 248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                          248,248,248,  248,248,248,  248,248,248,  248,248,248,
                          248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                          248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                          248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                          248,248,248,  248,248,248,  248,248,248,  248,248,248,
                          248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                          248,248,248,  248,248,248,  248,248,248,  248,248,248,
                          248,248,248,  248,248,248,  248,248,248,  248,248,248,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, goldPack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void twnk9_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t twnk9[108] = {    72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                          72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                          72,72,72,    3,72,248,   72,28,72,    248,72,75,
                          72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                          72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                          72,72,72,    3,72,248,   72,28,72,    248,72,75,
                          72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                          72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                          72,72,72,    3,72,248,   72,28,72,    248,72,75,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, twnk9, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void twnk10_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t twnk10[108] = {   72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,
                          72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,
                          72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, twnk10, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void cyanPack_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t cyanPack[108] = { 31,31,31,   31,31,31,   31,31,31,   31,31,31,
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,    
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,    
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,    
                          31,31,31,   31,31,31,   31,31,31,   31,31,31,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, cyanPack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void twnk11_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t twnk11[108] = {   72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,
                          72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,
                          72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, twnk11, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void magenta_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t magenta[108] = {   227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,
                           227,227,227,   227,227,227,   227,227,227,   227,227,227,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, magenta, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void gold_on (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t gold[108] = {   248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                        248,248,248,  248,248,248,  248,248,248,  248,248,248,
                        248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                        248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                        248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                        248,248,248,  248,248,248,  248,248,248,  248,248,248,
                        248,248,248,  248,248,248,  248,248,248,  248,248,248, 
                        248,248,248,  248,248,248,  248,248,248,  248,248,248,
                        248,248,248,  248,248,248,  248,248,248,  248,248,248,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, gold, m);
usleep(DORK);


// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void med_rainbow (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t rainbow5[108] = {   254,224,28,   3,252,31,   227,0,237,   19,248,192,
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,
                            237,19,248,   192,157,0,   27,171,254, 227,0,157,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,
                            237,19,248,   192,157,0,   27,171,254, 227,0,157,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,
                            237,19,248,   192,157,0,   27,171,254, 227,0,157,};

uint8_t rainbow6[108] = {   192,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,192,157,   0,27,171,   254,224,28,
                            3,252,31,   227,0,237,   19,248,254,   224,157,0,
                            192,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,192,157,   0,27,171,   254,224,28,
                            3,252,31,   227,0,237,   19,248,254,   224,157,0,
                            192,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,192,157,   0,27,171,   254,224,28,
                            3,252,31,   227,0,237,   19,248,254,   224,157,0,};

uint8_t rainbow7[108] = {   248,192,157,   25,27,171,   254,224,28,   3,252,31,
                            227,66,237,   19,248,192,   157,25,27,   171,254,224,
                            28,3,252,   31,227,66,   237,19,248,     192,157,25,
                            248,192,157,   25,27,171,   254,224,28,   3,252,31,
                            227,66,237,   19,248,192,   157,25,27,   171,254,224,
                            28,3,252,   31,227,66,   237,19,248,     192,157,25,
                            248,192,157,   25,27,171,   254,224,28,   3,252,31,
                            227,66,237,   19,248,192,   157,25,27,   171,254,224,
                            28,3,252,   31,227,66,   237,19,248,     192,157,25,};

uint8_t rainbow8[108] = {   19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,66,   237,19,248,   192,157,25,   27,171,254,
                            224,28,3,   252,31,227,   66,237,19,   248,192,157,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,66,   237,19,248,   192,157,25,   27,171,254,
                            224,28,3,   252,31,227,   66,237,19,   248,192,157,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,66,   237,19,248,   192,157,25,   27,171,254,
                            224,28,3,   252,31,227,   66,237,19,   248,192,157,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, rainbow5, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow6, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, rainbow6, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow7, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, rainbow7, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow8, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, rainbow8, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void red_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t red[108] = {   224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224, 
                        224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224, 
                        224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224,
                        224,224,224,   224,224,224,   224,224,224,   224,224,224, };

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, red, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void short_rainbow (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t rainbow1[108] = {   254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,};

uint8_t rainbow2[108] = {   171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,   
                            171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,
                            171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,};

uint8_t rainbow3[108] = {   27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,
                            27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,
                            27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,};

uint8_t rainbow4[108] = {   0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,
                            0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,
                            0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 
nbytes = ftdi_write_data(ftdi1, rainbow1, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow2, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow3, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, rainbow4, m);
usleep(DASH);
nbytes = ftdi_write_data(ftdi1, dPack, m);
// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void orange_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t orange[108] = {  244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244, 
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,  
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,
                         244,244,244,   244,244,244,   244,244,244,   244,244,244,  };

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, orange, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void white_flash (GtkWidget *widget, gpointer  data)
{
  int ret = 0;
  int nbytes;
  uint8_t dPack[108] = { };

  uint8_t white[108] = {  254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,
                          254, 254, 254,  254, 254, 254,  254, 254, 254,  254, 254, 254,};

  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  nbytes = ftdi_write_data(ftdi1, white, m);
  usleep(DAB);
  nbytes = ftdi_write_data(ftdi1, dPack, m);

  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void xmas_solid (GtkWidget *widget, gpointer  data)
{
  int ret = 0;
  int nbytes;

  uint8_t X1Pack[108] =  {   224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                             28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                             224,28,224,  28,224,28,   252,252,224, 28,224,28, 
                             224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                             28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                             224,28,224,  28,224,28,   252,252,224, 28,224,28,
                             224,28,224,  28,224,28,   252,224,28,  224,28,224, 
                             28,224,252,  28,224,28,   224,28,224,  28,252,252, 
                             224,28,224,  28,224,28,   252,252,224, 28,224,28,};

  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  nbytes = ftdi_write_data(ftdi1, X1Pack, m);
  usleep(DROOL);
  

  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void yellow_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t yellow[108] = {   252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252, 
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,   
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,
                          252,252,252,   252,252,252,   252,252,252,   252,252,252,   };

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, yellow, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void *asterion (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t twnk1[108] = {   72,72,16,   72,72,72,   128,72,36,   72,255,72,   
                         72,72,72,   72,146,72,   72,72,72,   72,72,72,
                         140,72,72,   72,72,72,   72,72,72,   72,146,172,
                         72,72,16,   72,72,72,   128,72,36,   72,255,72,   
                         72,72,72,   72,146,72,   72,72,72,   72,72,72,
                         140,72,72,   72,72,72,   72,72,72,   72,146,172,
                         72,72,16,   72,72,72,   128,72,36,   72,255,72,   
                         72,72,72,   72,146,72,   72,72,72,   72,72,72,
                         140,72,72,   72,72,72,   72,72,72,   72,146,172,};

uint8_t twnk2[108] = {   72,144,72,   72,72,72,   72,224,72,   144,72,36,
                         72,72,72,   72,72,182,   72,72,72,   72,72,72,
                         72,0,72,   72,72,72,   72,140,72,   72,72,36,                        
                         72,144,72,   72,72,72,   72,224,72,   144,72,36,
                         72,72,72,   72,72,182,   72,72,72,   72,72,72,
                         72,0,72,   72,72,72,   72,140,72,   72,72,36, 
                         72,144,72,   72,72,72,   72,224,72,   144,72,36,
                         72,72,72,   72,72,182,   72,72,72,   72,72,72,
                         72,0,72,   72,72,72,   72,140,72,   72,72,36, };

uint8_t twnk3[108] = {   72,72,72,   72,72,72,   74,72,72,   72,36,72,
                         72,72,128,   72,72,72,   72,72,72,   72,16,72,
                         72,0,72,   72,72,72,   2,72,72,   72,128,72,
                         72,72,72,   72,72,72,   74,72,72,   72,36,72,
                         72,72,128,   72,72,72,   72,72,72,   72,16,72,
                         72,0,72,   72,72,72,   2,72,72,   72,128,72,
                         72,72,72,   72,72,72,   74,72,72,   72,36,72,
                         72,72,128,   72,72,72,   72,72,72,   72,16,72,
                         72,0,72,   72,72,72,   2,72,72,   72,128,72,};

uint8_t twnk4[108] = {   128,73,72,   248,72,72,   254,72,72,   72,72,72,
                         72,72,72,   72,0,72,   72,72,72,   72,72,72,
                         0,72,236,   72,72,72,   72,72,72,  72,72,72,  
                         128,73,72,   248,72,72,   254,72,72,   72,72,72,
                         72,72,72,   72,0,72,   72,72,72,   72,72,72,
                         0,72,236,   72,72,72,   72,72,72,  72,72,72,
                         128,73,72,   248,72,72,   254,72,72,   72,72,72,
                         72,72,72,   72,0,72,   72,72,72,   72,72,72,
                         0,72,236,   72,72,72,   72,72,72,  72,72,72,};

uint8_t twnk5[108] = {   72,224,72,   72,72,3,   72,248,72,   72,72,72,
                         72,224,73,   28,72,72,   72,248,72,   72,72,72,
                         72,248,72,   72,72,3,   72,224,72,   72,72,72,
                         72,224,72,   72,72,3,   72,248,72,   72,72,72,
                         72,224,73,   28,72,72,   72,248,72,   72,72,72,
                         72,248,72,   72,72,3,   72,224,72,   72,72,72,
                         72,224,72,   72,72,3,   72,248,72,   72,72,72,
                         72,224,73,   28,72,72,   72,248,72,   72,72,72,
                         72,248,72,   72,72,3,   72,224,72,   72,72,72,};

uint8_t twnk6[108] = {   72,28,248,   72,72,72,   72,72,224,   72,28,72,
                         72,72,72,   72,28,72,   3,72,72,   72,28,72,
                         72,72,224,   72,28,72,   3,72,72,   72,28,72,
                         72,28,248,   72,72,72,   72,72,224,   72,28,72,
                         72,72,72,   72,28,72,   3,72,72,   72,28,72,
                         72,72,224,   72,28,72,   3,72,72,   72,28,72,
                         72,28,248,   72,72,72,   72,72,224,   72,28,72,
                         72,72,72,   72,28,72,   3,72,72,   72,28,72,
                         72,72,224,   72,28,72,   3,72,72,   72,28,72,};

uint8_t twnk7[108] = {   72,72,72,   248,72,28,   72,72,72,   248,72,72,
                         248,3,72,   248,72,72,   72,72,72,   248,72,28,
                         72,72,72,   248,72,72,   72,3,72,   28,72,72,
                         72,72,72,   248,72,28,   72,72,72,   248,72,72,
                         248,3,72,   248,72,72,   72,72,72,   248,72,28,
                         72,72,72,   248,72,72,   72,3,72,   28,72,72,
                         72,72,72,   248,72,28,   72,72,72,   248,72,72,
                         248,3,72,   248,72,72,   72,72,72,   248,72,28,
                         72,72,72,   248,72,72,   72,3,72,   28,72,72,};

uint8_t twnk8[108] = {   252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                         128,28,80,   128,248,80,  128,224,80,  28,252,80,
                         128,144,80,   128,252,80,  128,72,80,  252,128,80,
                         252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                         128,28,80,   128,248,80,  128,224,80,  28,252,80,
                         128,144,80,   128,252,80,  128,72,80,  252,128,80,
                         252,248,80,   128,248,80,  128,248,80,  128,248,80, 
                         128,28,80,   128,248,80,  128,224,80,  28,252,80,
                         128,144,80,   128,252,80,  128,72,80,  252,128,80,};

uint8_t twnk9[108] = {   72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                         72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                         72,72,72,    3,72,248,   72,28,72,    248,72,75,
                         72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                         72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                         72,72,72,    3,72,248,   72,28,72,    248,72,75,
                         72,72,248,   72,72,72,   72,72,248,   72,72,72,   
                         72,72,248,   72,72,248,  75,72,72,    72,72,248,   
                         72,72,72,    3,72,248,   72,28,72,    248,72,75,};

uint8_t twnk10[108] = {   72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,
                          72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,
                          72,254,72,   248,72,72,  248,72,72,   248,72,72,
                          248,72,72,   248,72,72,  248,72,72,   224,3,28,    
                          248,72,72,   72,72,72,   248,72,254,   248,72,72,};

uint8_t twnk11[108] = {   72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,
                          72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,
                          72,72,255,   72,248,72,  28,248,72,   72,248,3,
                          72,248,72,   72,248,72,  72,248,72,   72,224,72,
                          72,248,72,   28,72,72,   72,248,72,   72,242,72,};

uint8_t twnk12[108] = {   72,72,144,   72,72,72,   144,72,72,   72,72,72,
                          72,72,72,   72,182,72,   144,72,72,   72,36,72,
                          140,72,72,   72,72,72,   36,72,72,   72,72,72,
                          72,72,144,   72,72,72,   144,72,72,   72,72,72,
                          72,72,72,   72,182,72,   144,72,72,   72,36,72,
                          140,72,72,   72,72,72,   36,72,72,   72,72,72,
                          72,72,144,   72,72,72,   144,72,72,   72,72,72,
                          72,72,72,   72,182,72,   144,72,72,   72,36,72,
                          140,72,72,   72,72,72,   36,72,72,   72,72,72,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){

        nbytes = ftdi_write_data(ftdi1, twnk1, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk2, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk3, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk4, m);
        usleep(DROOL);  
        
        nbytes = ftdi_write_data(ftdi1, twnk5, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk6, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk7, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk8, m);
        usleep(DROOL);

        nbytes = ftdi_write_data(ftdi1, twnk9, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk10, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk11, m);
        usleep(DROOL);
        nbytes = ftdi_write_data(ftdi1, twnk12, m);
        usleep(DROOL); 
        
}

return NULL;

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void twnk_button1(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, asterion, NULL);
}

void stop_loop (GtkWidget *widget, gpointer data) {

  int ret = 0;
  int nbytes;
 
  uint8_t dbytePack[108] = {0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0};

  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);

  // set loop to false before cancelling the thread. 
  // otherwise thread will cancel while loop is set to true. 
  if (loop1) {
    
  loop1 = false;

  pthread_cancel(thread); // Cancel the thread

  }
  

  struct ftdi_context *ftdi1 = create_new_ftdi(); 
  nbytes = ftdi_write_data(ftdi1, dbytePack, m);
    
  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n"); 
    
}

void *rainbow_cycle (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t rainbow1[108] = {   254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,
                            254,224,28,   3,252,31,   227,0,237,   19,248,192,   
                            157,0,27,   171,254,224,   28,3,252,   31,227,0,   
                            237,19,248,   192,157,0,   27,171,254, 224,28,3,};

uint8_t rainbow2[108] = {   171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,   
                            171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,
                            171,254,224,   28,3,252,   31,227,66,   237,0,248,
                            0,157,0,   27,171,254,   224,28,3,   252,31,227,
                            0,237,19,   248,0,157,   155,27,171,   254,224,28,};

uint8_t rainbow3[108] = {   27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,
                            27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,
                            27,171,254,   224,28,3,   252,31,227,   0,237,19,
                            248,192,157,   0,27,171,   254,224,28,   3,252,31,
                            227,0,237,   19,248,192,   157,0,27,   171,254,224,};

uint8_t rainbow4[108] = {   0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,
                            0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,
                            0,27,171,   254,224,28,   3,252,31,   227,0,237,
                            19,248,192,   157,25,27,   171,254,224,   28,3,252,
                            31,227,0,   237,19,248,   192,157,25,   27,171,254,};

size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){
nbytes = ftdi_write_data(ftdi1, rainbow1, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, rainbow2, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, rainbow3, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, rainbow4, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void rainbow_button1(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, rainbow_cycle, NULL);
}

void *snowman1 (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t snowman[108] = {   248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,248,254,
                           248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,248,254,
                           248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,248,254,};



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){
nbytes = ftdi_write_data(ftdi1, snowman,  m);
usleep(SLP);
rotate13(snowman);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, snowman,  m);
usleep(SLP);
rotate13(snowman);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void snowman_button1(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, snowman1, NULL);
}


void *snowman2 (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };

uint8_t snowman[108] = {   248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,254,254,
                           248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,254,254,
                           248,254,254,   254,254,254,   254,254,254,   254,254,254,
                           254,254,254,   254,248,254,   254,254,254,   254,254,254, 
                           254,254,254,   254,254,254,   254,254,254,   254,254,254,};



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){
nbytes = ftdi_write_data(ftdi1, snowman,  m);
usleep(SLP);
rotate13(snowman);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, snowman,  m);
usleep(SLP);
rotate13(snowman);
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void snowman_button2(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, snowman2, NULL);
}

void *tree1 (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t tree[108] = {   248,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,224,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   224,28,28,   28,28,28,
                        248,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,224,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   224,28,28,   28,28,28,
                        248,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,224,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   224,28,28,   28,28,28,};



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){
nbytes = ftdi_write_data(ftdi1, tree,     m);
usleep(SLP);
rotate13(tree   );
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, tree,     m);
usleep(SLP);
rotate13(tree   );
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void tree_button1(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, tree1, NULL);
}

void *tree2 (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t tree[108] = {   254,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,248,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   248,28,28,   28,28,28,
                        254,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,248,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   248,28,28,   28,28,28,
                        254,28,28,   28,28,28,   28,28,28,   28,28,28,
                        28,224,248,   28,28,28,   28,28,28,   28,28,28,
                        28,28,28,   28,28,224,   248,28,28,   28,28,28,};



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

while(loop1){
nbytes = ftdi_write_data(ftdi1, tree,     m);
usleep(SLP);
rotate13(tree   );
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
nbytes = ftdi_write_data(ftdi1, tree,     m);
usleep(SLP);
rotate13(tree   );
nbytes = ftdi_write_data(ftdi1, dPack, m);
usleep(SLP);
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void tree_button2(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, tree2, NULL);
}

void *marqee_left (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t igPack[108] = {   254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, };



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 


 
// Copy initial RGB values to sorc array
for (int i = 0; i < 108; i++) {
    sorc[i] = igPack[i];
}

while (loop1) {

  // Store the rollover values
  uint8_t temp = sorc[0]; // Store the first byte

  for (int i = 0; i < 108; i++) {
    if (i < 107)
      sorc[i] = sorc[i + 1]; // Shift bytes to the left
    else
      sorc[i] = temp; // Place the stored byte at the last position
  }

  nbytes = ftdi_write_data(ftdi1, sorc, m); // Write the updated RGB values
  usleep(DAB); // Sleep for a short duration

}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void marqee_button1(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, marqee_left, NULL);
}

void *marqee_right (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t igPack[108] = {   254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, };



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 


 
// Initialize sorc array with initial RGB values
for (int i = 0; i < 108; i++) {
        sorc[i] = igPack[i];
}

while (loop1) {
  // Store the rollover value
  uint8_t temp = sorc[107];

  // Shift array elements to the right
  for (int i = 107; i > 0; i--) {
    sorc[i] = sorc[i - 1];
  }

  sorc[0] = temp; // Place the stored byte at the first position

  nbytes = ftdi_write_data(ftdi1, sorc, m); // Write the updated RGB values
  usleep(DAB); // Sleep for a short duration
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void marqee_button2(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, marqee_right, NULL);
}

void *marqee_slow_left (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t igPack[108] = {   254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, };



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 


 
// Copy initial RGB values to sorc array
for (int i = 0; i < 108; i++) {
    sorc[i] = igPack[i];
}

while (loop1) {

  // Store the rollover values
  uint8_t temp = sorc[0]; // Store the first byte

  for (int i = 0; i < 108; i++) {
    if (i < 107)
      sorc[i] = sorc[i + 1]; // Shift bytes to the left
    else
      sorc[i] = temp; // Place the stored byte at the last position
  }

  nbytes = ftdi_write_data(ftdi1, sorc, m); // Write the updated RGB values
  usleep(SLOW); // Sleep for a short duration

}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void marqee_button3(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, marqee_slow_left, NULL);
}

void *marqee_slow_right (void *data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };


uint8_t igPack[108] = {   254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, 
                        254,254,254,   254,224,224,   224,224,28,   28,28,28,
                        3,3,3,   3,254,254,   254,254,224,   224,224,224,
                        28,28,28,   28,3,3,   3,3,3,   3,3,254, };



size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 


 
// Initialize sorc array with initial RGB values
for (int i = 0; i < 108; i++) {
        sorc[i] = igPack[i];
}

while (loop1) {
  // Store the rollover value
  uint8_t temp = sorc[107];

  // Shift array elements to the right
  for (int i = 107; i > 0; i--) {
    sorc[i] = sorc[i - 1];
  }

  sorc[0] = temp; // Place the stored byte at the first position

  nbytes = ftdi_write_data(ftdi1, sorc, m); // Write the updated RGB values
  usleep(SLOW); // Sleep for a short duration
}

return NULL; 

// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

void marqee_button4(GtkWidget *widget, gpointer data) {

    // Start a new thread for the loop
    loop1 = TRUE; 
    pthread_create(&thread, NULL, marqee_slow_right, NULL);
}

void blue_flash (GtkWidget *widget, gpointer  data) {

int ret = 0;
int nbytes;
int i = 0; 
uint8_t dPack[108] = { };
uint8_t bluePack[108] = {   3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,
                            3,3,3,   3,3,3,   3,3,3,   3,3,3,};
size_t l = sizeof(dest);
size_t m = getEncodedBufferSize(l);

struct ftdi_context *ftdi1 = create_new_ftdi(); 

nbytes = ftdi_write_data(ftdi1, bluePack, m);
usleep(DAB);
nbytes = ftdi_write_data(ftdi1, dPack, m);
// close ftdi device connection
if ((ret = ftdi_usb_close(ftdi1)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi1)); 
    }

  ftdi_free(ftdi1);
  
  printf("End of program.\n");
  
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *label;
    /* Create a new window, and set its title */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "UIdaho Band Glasses");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size (GTK_WINDOW (window), 700, 500);
    /* Here we construct the container that is going to pack our buttons */
    grid = gtk_grid_new();

    /* Pack the container in the window */
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    /* Create a label for the title */
    label = gtk_label_new("Menu Buttons");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1); // Span 2 columns for the title
   
    button = gtk_button_new_with_label("All On");
    g_signal_connect(button, "clicked", G_CALLBACK(test_on), NULL);

    /* Place the first button in the grid cell (0, 0), and make it fill
    * just 1 cell horizontally and vertically (ie no spanning)
    */
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);

    button = gtk_button_new_with_label("All Off");
    g_signal_connect(button, "clicked", G_CALLBACK(test_off), NULL);

    /* Place the second button in the grid cell (1, 0), and make it fill
    * just 1 cell horizontally and vertically (ie no spanning)
    */
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

    button = gtk_button_new_with_label("Quit");
     g_signal_connect (button, "clicked", G_CALLBACK (test_off), NULL);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
   
    /* Place the Quit button in the grid cell (0, 1), and make it
    * span 2 columns.
    */
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);
    
    button = gtk_button_new_with_label("Stop loop");
    g_signal_connect (button, "clicked", G_CALLBACK (stop_loop), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);

    button = gtk_button_new_with_label("White Flash");
    g_signal_connect (button, "clicked", G_CALLBACK (white_flash), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 1, 1);

    /*Second round of buttons*/
    label = gtk_label_new("Flash Buttons");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Blue flash");
    g_signal_connect(button, "clicked", G_CALLBACK(blue_flash), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 1, 1, 1, 1);

    button = gtk_button_new_with_label("Red flash ");
    g_signal_connect(button, "clicked", G_CALLBACK(red_flash ), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 1, 2, 1, 1);

    button = gtk_button_new_with_label("Green Flash");
    g_signal_connect (button, "clicked", G_CALLBACK (green_flash ), NULL);
   
    gtk_grid_attach(GTK_GRID(grid), button, 1, 3, 1, 1);

    button = gtk_button_new_with_label("Cyan flash");
    g_signal_connect (button, "clicked", G_CALLBACK (cyanPack_flash), NULL);
   
    gtk_grid_attach(GTK_GRID(grid), button, 1, 4, 1, 1);

    button = gtk_button_new_with_label("Gold flash ");
    g_signal_connect (button, "clicked", G_CALLBACK (goldPack_flash ), NULL);
   
    gtk_grid_attach(GTK_GRID(grid), button, 1, 5, 1, 1);


     /*third round of buttons*/
    label = gtk_label_new("Flash Buttons 2");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 2, 0, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Magenta flash ");
    g_signal_connect (button, "clicked", G_CALLBACK (magenta_flash ), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 1, 1, 1);

    button = gtk_button_new_with_label("Orange flash");
    g_signal_connect (button, "clicked", G_CALLBACK (orange_flash), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 2, 2, 1, 1);

    button = gtk_button_new_with_label("Yellow flash");
    g_signal_connect(button, "clicked", G_CALLBACK(yellow_flash), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 3, 1, 1);

    button = gtk_button_new_with_label("Twnk8 Flash");
    g_signal_connect (button, "clicked", G_CALLBACK (twnk8_flash), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 4, 1, 1);

    button = gtk_button_new_with_label("Twnk9 flash");
    g_signal_connect(button, "clicked", G_CALLBACK(twnk9_flash), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 5, 1, 1);


    /*etc round of buttons*/
    label = gtk_label_new("Christmas Buttons 2");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 3, 0, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Slow xmas sparkle loop ");
    g_signal_connect(button, "clicked", G_CALLBACK(xmas_sparkle_button2), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 3, 1, 1, 1);

    button = gtk_button_new_with_label("Xmas Pulse");
    g_signal_connect(button, "clicked", G_CALLBACK(xmas_pulse), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 3, 2, 1, 1);

    /*Fourth round of buttons*/
    label = gtk_label_new("Rainbow Buttons");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 6, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Twnk10 flash");
    g_signal_connect(button, "clicked", G_CALLBACK(twnk10_flash), NULL);    
    
    gtk_grid_attach(GTK_GRID(grid), button, 0, 7, 1, 1);

    button = gtk_button_new_with_label("Twnk11 flash");
    g_signal_connect (button, "clicked", G_CALLBACK (twnk11_flash), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 8, 1, 1);

    button = gtk_button_new_with_label("Short rainbow");
    g_signal_connect (button, "clicked", G_CALLBACK (short_rainbow), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 0, 9, 1, 1);

    button = gtk_button_new_with_label("Medium rainbow");
    g_signal_connect(button, "clicked", G_CALLBACK(med_rainbow), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 10, 1, 1);

    button = gtk_button_new_with_label("Rainbow cycle ");
    g_signal_connect (button, "clicked", G_CALLBACK (rainbow_button1 ), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 0, 11, 1, 1);

    /*Fifth round of buttons*/
    label = gtk_label_new("Marque Buttons");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 1, 6, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Slow marque right");
    g_signal_connect (button, "clicked", G_CALLBACK (marqee_button4), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 1, 7, 1, 1);

    button = gtk_button_new_with_label("Slow marque left");
    g_signal_connect (button, "clicked", G_CALLBACK (marqee_button3), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 1, 8, 1, 1);

    button = gtk_button_new_with_label("Asterion");
    g_signal_connect (button, "clicked", G_CALLBACK (twnk_button1), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 1, 9, 1, 1);

    button = gtk_button_new_with_label("Marque Left ");
    g_signal_connect (button, "clicked", G_CALLBACK (marqee_button1), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 1, 10, 1, 1);

    button = gtk_button_new_with_label("Marque Right");
    g_signal_connect(button, "clicked", G_CALLBACK(marqee_button2), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 1, 11, 1, 1);

    /*sixth round of buttons*/
    label = gtk_label_new("Christmas Buttons");
    gtk_widget_set_size_request(label, 150, -1);

    gtk_grid_attach(GTK_GRID(grid), label, 2, 6, 1, 1); // Span 2 columns for the title

    button = gtk_button_new_with_label("Snowman 1");
    
    g_signal_connect(button, "clicked", G_CALLBACK(snowman_button1), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 7, 1, 1);

    button = gtk_button_new_with_label("Snowman 2 ");
    g_signal_connect(button, "clicked", G_CALLBACK(snowman_button2), NULL);

    gtk_grid_attach(GTK_GRID(grid), button, 2, 8, 1, 1);

    button = gtk_button_new_with_label("Tree 1");
    g_signal_connect (button, "clicked", G_CALLBACK (tree_button1), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 9, 1, 1);

    button = gtk_button_new_with_label("Tree 2");
    g_signal_connect (button, "clicked", G_CALLBACK (tree_button2), NULL);
    
    gtk_grid_attach(GTK_GRID(grid), button, 2, 10, 1, 1);

    button = gtk_button_new_with_label("Xmas sparkle loop");
    g_signal_connect(button, "clicked", G_CALLBACK(xmas_sparkle_button1), NULL);
 
    gtk_grid_attach(GTK_GRID(grid), button, 2, 11, 1, 1);

    /* Apply styling to set the background color to gray */
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, "window { background-color: gray; }", -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    /* Now that we are done packing our widgets, we show them all
    * in one go, by calling gtk_widget_show_all() on the window.
    * This call recursively calls gtk_widget_show() on all widgets
    * that are contained in the window, directly or indirectly.
    */
    gtk_widget_show_all(window);
}