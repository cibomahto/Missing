import hypermedia.video.*;
import processing.serial.*;

Serial myPort;  // Create object from Serial class


OpenCV opencv;

void setup() {

    size( 320, 240 );
    
    String portName = Serial.list()[0];
    myPort = new Serial(this, portName, 57600);

    opencv = new OpenCV(this);
    opencv.capture( width, height );
    opencv.cascade( OpenCV.CASCADE_FRONTALFACE_ALT );    // load the FRONTALFACE description file
}

void draw() {
    
    opencv.read();
    image( opencv.image(), 0, 0 );
    
    // detect anything ressembling a FRONTALFACE
    java.awt.Rectangle[] faces = opencv.detect();
    
    // draw detected face area(s)
    noFill();
    stroke(255,0,0);
    for( int i=0; i<faces.length; i++ ) {
        if (i == 0) {
          int pos = int(float(faces[i].x + faces[i].width/2)/width*9);
          println(pos);
          myPort.write(pos + '0');
          
          stroke(255,0,0);
        }
        else {
          stroke(0,128,0);
        }
        
        rect( faces[i].x, faces[i].y, faces[i].width, faces[i].height ); 
    }
}

