import controlP5.*;
import processing.serial.*;

ControlP5 cp5;
Serial outPort;

String VERSION_STRING = "1.0";

int NUMBER_OF_SPEAKERS = 50;   // Number of attached speakers


Boolean sendPositions = false;
Boolean rangeTest = false;
Boolean randomMotion = false;

int[] pos;                     // Speaker positions to send

int address = 0;
Boolean enableAddressProgram = false;

Boolean Reversed = false;

void sendPositionData() {
  outPort.write(0xFE);
  for(int i = 0; i < NUMBER_OF_SPEAKERS; i++) {
    outPort.write(pos[i] & 0x7F);
  }
  
  outPort.write(0); // TODO: CRC
  outPort.write(0);
}

void sendSetAddress() {
  outPort.write(0x81);
  outPort.write(address);
  
  outPort.write(0); // TODO: CRC
  outPort.write(0);
}

void sendSetReversed() {
  outPort.write(0x82);
  outPort.write(address);
  outPort.write(Reversed==false?0:1);
  println(Reversed==false?0:1);
  outPort.write(0); // TODO: CRC
  outPort.write(0);
}

void setup() {
  size(1200, 620);
  cp5 = new ControlP5(this);
  
  pos = new int[NUMBER_OF_SPEAKERS];
  
  String portName = "";
  
  for(String port : Serial.list()) {
    if(!port.contains("Bluetooth")) {
      portName = port;
    }
  }
  
  
  
  println(portName);
  
  outPort = new Serial(this, portName, 57600);
  
  
  // Position controls
  cp5.addToggle("sendPositions")
   .setPosition(10,10)
   ;

  cp5.addToggle("rangeTest")
   .setPosition(110,10)
   ;
   
  cp5.addToggle("randomMotion")
   .setPosition(210,10)
   ;
  
  for(int i = 0; i < NUMBER_OF_SPEAKERS; i++) {
    int speakersPerCol = 17;
    
    Slider s = cp5.addSlider("pos[" + i + "]")
     .setPosition(400 + (i/speakersPerCol)*250,10+(i%speakersPerCol)*35)
     .setSize(180,30)
     .setRange(0,127)
     .setSliderMode(Slider.FLEXIBLE)
     ;

    s.setValue(pos[i]);
    s.setId(i);
  }
  
  // Parameter set
   
  Slider s = cp5.addSlider("address")
   .setPosition(10,350)
   .setSize(180,30)
   .setRange(0,49)
   .setSliderMode(Slider.FLEXIBLE)
   .setDecimalPrecision(1)
   ;
   
  // Address set
  cp5.addToggle("enableAddressProgram")
   .setPosition(10,400)
   ;
  
  cp5.addBang("Store address")
   .setPosition(140, 400)
   ;

  // Address set
  cp5.addToggle("Reversed")
   .setPosition(10,450)
   ;
  
  cp5.addBang("Store reversed")
   .setPosition(140, 450)
   ;

  // Debug info
  cp5.addTextlabel("label1")
    .setText("Debugger version " + VERSION_STRING)
    .setPosition(10,575)
    ;

  if(portName != "") {
    cp5.addTextlabel("label2")
     .setText("Transmitting on " + portName)
     .setPosition(10,590)
     ;
  } else {
    cp5.addTextlabel("label2")
     .setText("Could not find a port to transmit on!")
     .setPosition(10,590)
     ;
  }    
}


float   rangeTestToggleTime = 0;
boolean rangeTestDirection  = false;

void draw() {
  background(0);
  stroke(255);
  
  if(rangeTest) {
    if(rangeTestToggleTime == 0) {
      rangeTestToggleTime = millis();
    }
    
    if(millis() > rangeTestToggleTime) {
      int newPosition;
      if(rangeTestDirection) {
        newPosition = 0;
      }
      else {
        newPosition = 127;
      }
      
      for(int i = 0; i < NUMBER_OF_SPEAKERS; i++) {
        pos[i] = newPosition;
      }

      rangeTestToggleTime = millis() + 10000;
      rangeTestDirection = !rangeTestDirection;
    }
    
  }

  // Send random positions
  
  if(randomMotion) {
    if(random(0,10)>8) {
      pos[(int)random(0,NUMBER_OF_SPEAKERS-1)] = (int)random(0,127);
    } 
  }
  
  if(sendPositions) {
    sendPositionData();
  }
}

void controlEvent(ControlEvent theEvent) {
  if (theEvent.isController()) {
    // Program Address
    if (theEvent.controller().name().startsWith("enableAddressProgram")) {
      enableAddressProgram = theEvent.value() == 1;
    }    
    
    if (theEvent.controller().name().startsWith("Store address")) {
      if(enableAddressProgram) {
        sendSetAddress();
      } 
    }
    
    // Reversed setting
    if (theEvent.controller().name().startsWith("Reversed")) {
      Reversed = theEvent.value() == 1;
    }    
    
    if (theEvent.controller().name().startsWith("Store reversed")) {
      sendSetReversed();
    }
    
    
    // Position
    if (theEvent.controller().name().startsWith("sendPositions")) {
      sendPositions = theEvent.value() == 1;
    }

    if (theEvent.controller().name().startsWith("rangeTest")) {
      rangeTest = theEvent.value() == 1;
    }

    if (theEvent.controller().name().startsWith("randomMotion")) {
      randomMotion = theEvent.value() == 1;
    }
    
    // check if theEvent is coming from a position controller
    if (theEvent.controller().name().startsWith("pos")) {
      // get the id of the controller and map the value
      // to an element inside the boxsize array.
      int id = theEvent.controller().id();
      if (id>=0 && id<pos.length) {
        pos[id] = (int)theEvent.value();
      }
    }
  }
}

