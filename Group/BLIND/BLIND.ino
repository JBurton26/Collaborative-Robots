//Included Libraries
#include <painlessMesh.h>
#include <SoftwareSerial.h>

//LED Blink stuff
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2
#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

//Mesh Info
#define   MESH_SSID       "robot12345"
#define   MESH_PASSWORD   "collab12345"
#define   MESH_PORT       5555

// Prototypes
void sendMessage();
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);
Scheduler     userScheduler; // to control your personal task
//Sets up mesh
painlessMesh  mesh;

//Variable Initialization
bool calc_delay = false;
SimpleList<uint32_t> nodes;
//Software Serial Is used to monitor the messages from the serial monitor while the program is running
SoftwareSerial mySerial(12,14); //RX,TX

void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval
// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

//Triggered Once at the start of a program
void setup() {
  //Serial Initiated
  Serial.begin(4800);
  mySerial.begin(4800);
  //on board LED initiated
  pinMode(LED, OUTPUT);

  //mesh tasks
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  //Sets up how the onboard LED will blink
  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
    // If on, switch off, else switch on
    if (onFlag)
      onFlag = false;
    else
      onFlag = true;
    blinkNoNodes.delay(BLINK_DURATION);

    if (blinkNoNodes.isLastIteration()) {
      // Finished blinking. Reset task for next run
      // blink number of nodes (including this node) times
      blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
      // Calculate delay based on current mesh time and BLINK_PERIOD
      // This results in blinks between nodes being synced
      blinkNoNodes.enableDelayed(BLINK_PERIOD -
                                 (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
    }
  });
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  digitalWrite(LED, !onFlag);
}
//Method that decides what the message is and sends the message (NOT USED AS THIS ROBOT IS BLIND)
void sendMessage() {
  String msg;
  mesh.sendBroadcast(msg);
  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }
}
//Method that deals with incoming messages, Sending certain corresponding characters through the SoftwareSerial to the Arduino
void receivedCallback(uint32_t from, String & msg) {
  Serial.println(msg);
  if (msg == "Forward"){
    mySerial.println("Z");
  } else if (msg == "Stop"){
    mySerial.println("X");
  }
}
//Method that deals with additions to the mesh, mainly for blinking the LED the correct number of times
void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
}
//Similar to above but adds the node to the mesh
void changedConnectionCallback() {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  calc_delay = true;
}
//Not Needed for task but needed for mesh
void nodeTimeAdjustedCallback(int32_t offset) {
}
void delayReceivedCallback(uint32_t from, int32_t delay) {
}
