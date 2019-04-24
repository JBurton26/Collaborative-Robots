//Included Libraries
#include <painlessMesh.h>
#include <Ultrasonic.h>
#include <SoftwareSerial.h>
//LED BLINK FOR EACH MESH NODE
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2
#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for
//Mesh Name and Password
#define   MESH_SSID       "robot12345"
#define   MESH_PASSWORD   "collab12345"
#define   MESH_PORT       5555

//Prototype Methods
void sendMessage();
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);
Scheduler     userScheduler; // to control your personal task
//initiate mesh object
painlessMesh  mesh;
//Software Serial so that the serial monitor can be used to monitor and test outputs
SoftwareSerial mySerial(12,14); //(RX,TX)
//Sensor and pins (Trig, Echo)
Ultrasonic ultrasonic(13,15);
//Stuff for mesh messages
bool calc_delay = false;
SimpleList<uint32_t> nodes;
void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval
// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;
//Triggered once when the program starts
void setup() {
  //Serial initiation
  Serial.begin(57600);
  mySerial.begin(4800);

  //Onboard LED
  pinMode(LED, OUTPUT);
  //Mesh Stuff for starting the mesh and setting it up
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);
  //Adds tasks to a scheduler
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  //Sets up blink
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
//loops but not at a set time
void loop() {
  userScheduler.execute(); //it will run mesh scheduler as well
  //update the mesh
  mesh.update();
  digitalWrite(LED, !onFlag);
}
//Method that sends message
void sendMessage() {
  //Determines what the message is going to be based on the sensor information
  String msg;
  if (ultrasonic.read() > 25){
    msg = "Forward";
    mySerial.println("Z");
  } else {
    msg = "Stop";
    mySerial.println("X");
  }
  //Sends the message
  mesh.sendBroadcast(msg);
  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }
  //Prints the message to the Serial Monitor along with the reading for testing purposes
  Serial.println("Sending message:"+msg+" reading:"+ultrasonic.read());
}
//Prints any messages it receives to the Serial Monitor
void receivedCallback(uint32_t from, String & msg) {
  Serial.println(msg);
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
  //get the list of nodes to calculate the delay in messages
  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  calc_delay = true;
}
//Does Nothing but is necessary for the mesh
void nodeTimeAdjustedCallback(int32_t offset) {
}
void delayReceivedCallback(uint32_t from, int32_t delay) {
}
