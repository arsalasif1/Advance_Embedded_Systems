/* Topic: Resource Management - Example 21: Tick Hook and Gatekeeper Task
   Description: This is to analyse and learn the use of Gate Keeper task and Tick hook in regards to freertos.
   This code uses single core i.e core 0 for operations
   Task 2 runs First because of highest priority 
   then Task1
   and At each tick hook publishes 3 times in fixed blocking time period 600ms of two tasks. Means tick hook runs every 1ms.
   Authors: Mohammad Arsal Asif - 399789
   Tallah Waqar - 400125
   License: MIT License (https://opensource.org/licenses/MIT)
*/

// Importing tick hook library for esp
#include <esp_freertos_hooks.h>

// Array to be printed by each task and interrupt
static char *pcStringsToPrint[] = {
  "Task 1 ****************************************************\r\n",
  "Task 2 ----------------------------------------------------\r\n",
  "Message printed from the tick hook interrupt ##############\r\n"
};

//Creating Queue Handle
QueueHandle_t xPrintingQueue;

//Tick hook interrupt function
void vTickHookFunction(void) {
  //Counter initilized as Static to ensure it publishes to queue after 200 Tics
  static int iCount = 0;
  iCount++;
  if (iCount >= 200) {
    //Publishing to Queue
    xQueueSendToFrontFromISR(xPrintingQueue, &(pcStringsToPrint[2]), NULL);
    iCount = 0;
  }
}



void setup() {
  
  Serial.begin(115200); //Initilizing Serial at baud 115200 for results
  esp_register_freertos_tick_hook_for_cpu(vTickHookFunction, 0); //Attaching tick hook interrupt with custom function and running on core 0
  xPrintingQueue = xQueueCreate(5, sizeof(char *)); // Creating queue of length 5 and size of char array
  //Creating Tasks using single core
  if (xPrintingQueue != NULL) {
    xTaskCreatePinnedToCore(vPrintingTask, "PrintingTask1", 1000, (void *)0, 1, NULL, 0); // Task 1 for publishing to queue
    xTaskCreatePinnedToCore(vPrintingTask, "PrintingTask2", 1000, (void *)1, 2, NULL, 0); // Task 2 for publishing to queue
    xTaskCreatePinnedToCore(vGatekeeperTask, "Gatekeeper", 1000, NULL, 0, NULL, 0); // Gatekeeper Task for Serial Access
  }
}

void loop() {
  // Arduino ide boiler plate
}


// Gatekeeper Task
void vGatekeeperTask(void *pvParameters) {
  char *pcMessageToPrint;

  for (;;) {
    // Checks for empty if not then it prints the Strings
    if (xQueueReceive(xPrintingQueue, &pcMessageToPrint, portMAX_DELAY)) {
      Serial.print(pcMessageToPrint);
    }

  }
}

//Unique Printing Task function both processes uses this to publish
void vPrintingTask(void *pvParameters) {
  int iIndexToString;
  //  const TickType_t xMaxBlockTimeTicks = 0x20; //Not using random delay
  iIndexToString = (int)pvParameters;

  for (;;) {
    xQueueSendToBack(xPrintingQueue, &(pcStringsToPrint[iIndexToString]), portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(600)); // Using Fixed delay to get a sequenced output
  }
}
