import processing.serial.*;

Serial outPort;

String portName = "tty.usbserial-A100S0L9";

int NUMBER_OF_SPEAKERS = 50;
int[] speakerPositions;

void sendData() {
  outPort.write(0xFE);
  for(int i = 0; i < NUMBER_OF_SPEAKERS; i++) {
    outPort.write(speakerPositions[i] & 0x7F);
  }
  outPort.write(0); // TODO: CRC
  outPort.write(0);
}

void setup() {
  size(600, 400);
  
  speakerPositions = new int[NUMBER_OF_SPEAKERS];
  
  String portName = Serial.list()[0];
  outPort = new Serial(this, portName, 57600);
}


void draw() {
  background(0);
  stroke(255);
  line(mouseX, 0, mouseX, screen.height);
  
  speakerPositions[1] = (int)map((int)mouseX,0,(int)screen.height, 0, 127);
  sendData();
}
