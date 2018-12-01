/* structure that hold data*/
typedef struct{
  int sender;
  char *msg;
}Data;

/* this variable hold queue handle */
xQueueHandle xQueue;
TaskHandle_t xTask1;
TaskHandle_t xTask2;
void setup() {

  Serial.begin(112500);
  /* create the queue which size can contains 5 elements of Data */
  xQueue = xQueueCreate(5, sizeof(Data));

  xTaskCreatePinnedToCore(
      sendTask,           /* Task function. */
      "sendTask",        /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &xTask1,                /* Task handle to keep track of created task */
      0);                    /* pin task to core 0 */
  xTaskCreatePinnedToCore(
      receiveTask,           /* Task function. */
      "receiveTask",        /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &xTask2,            /* Task handle to keep track of created task */
      1);                 /* pin task to core 1 */   
}

void loop() {

}

void sendTask( void * parameter )
{
  /* keep the status of sending data */
  BaseType_t xStatus;
  /* time to block the task until the queue has free space */
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  /* create data to send */
  Data data;
  /* sender 1 has id is 1 */
  data.sender = 1;
  for(;;){
    Serial.print("sendTask run on core ");
    /* get the core that the task was pinned to */
    Serial.print(xTaskGetAffinity(xTask1));
    Serial.println(" is sending data");
    data.msg = (char *)malloc(20);
    memset(data.msg, 0, 20);
    memcpy(data.msg, "hello world", strlen("hello world"));
    /* send data to front of the queue */
    xStatus = xQueueSendToFront( xQueue, &data, xTicksToWait );
    /* check whether sending is ok or not */
    if( xStatus == pdPASS ) {
      /* increase counter of sender 1 */
      Serial.println("sendTask sent data");
    }
    /* we delay here so that receiveTask has chance to receive data */
    delay(1000);
  }
  vTaskDelete( NULL );
}

void receiveTask( void * parameter )
{
  /* keep the status of receiving data */
  BaseType_t xStatus;
  /* time to block the task until data is available */
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  Data data;
  for(;;){
    /* receive data from the queue */
    xStatus = xQueueReceive( xQueue, &data, xTicksToWait );
    /* check whether receiving is ok or not */
    if(xStatus == pdPASS){
      Serial.print("receiveTask run on core ");
      /* get the core that the task was pinned to */
      Serial.print(xTaskGetAffinity(xTask2));
      /* print the data to terminal */
      Serial.print(" got data: ");
      Serial.print("sender = ");
      Serial.print(data.sender);
      Serial.print(" msg = ");
      Serial.println(data.msg);
      free(data.msg);
    }
  }
  vTaskDelete( NULL );
}
