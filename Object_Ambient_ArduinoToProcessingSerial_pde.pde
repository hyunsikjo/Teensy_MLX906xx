import processing.serial.*;

Serial mySerial; //Serial 
PFont myFont;    //Font
float maxTemp, minTemp, avgTemp, maxAmbient, minAmbient, avgAmbient;
float ObjectTemp;
float AmbientTemp;
float[] arrayTemp = new float[11], arrayAmbient = new float[11];
boolean firstFlag;

void setup() {
  size(1300, 600);
  background(255);
  mySerial=new Serial(this, Serial.list()[0], 9600);
  fill(20);
//  myFont = createFont("arialIMT-48", 32);
//  textFont(myFont);
}

void draw() {
  background(255);
  graph();
}

void serialEvent(Serial s) {
  String data = null;
  
  try {
    data = s.readStringUntil('*');
    String[] dataList = split(data, ',');
    ObjectTemp = float(dataList[0]);
    AmbientTemp = float(dataList[1]);
    
    if(!firstFlag) 
    {
      maxTemp = ObjectTemp;
      minTemp = ObjectTemp;
      avgTemp = ObjectTemp;
      
      maxAmbient = AmbientTemp;
      minAmbient = AmbientTemp;
      avgAmbient = AmbientTemp;
      
      arrayTemp[0] = ObjectTemp;
      arrayAmbient[0] = AmbientTemp;
      firstFlag = true;
    }
    else 
    {
      //object temp min, max
      if(ObjectTemp > maxTemp) 
      {
        maxTemp = ObjectTemp;
      }
      else if(ObjectTemp <= minTemp) 
      {
        minTemp = ObjectTemp;
      }
      
      //Ambient temp min, max
      if(AmbientTemp > maxAmbient) 
      {
        maxAmbient = AmbientTemp;
      }
      else if(AmbientTemp <= minAmbient) 
      {
        minAmbient = AmbientTemp;
      }
      
      //average calculate   
      avgTemp = (avgTemp + ObjectTemp) * 0.5;
      avgAmbient = (avgAmbient + AmbientTemp) * 0.5;

      for(int i=0; i<arrayTemp.length; i++) 
      {
        if(i == (arrayTemp.length-1)) 
        {
          arrayTemp[0] = ObjectTemp;
          arrayAmbient[0] = AmbientTemp;
          continue;
        }
        arrayTemp[(arrayTemp.length-1)-i] = arrayTemp[(arrayTemp.length-2)-i]; //array bit shift
        arrayAmbient[(arrayAmbient.length-1)-i] = arrayAmbient[(arrayAmbient.length-2)-i]; //array bit shift
      }
    }
  }
  catch(Exception e) {
  }
}

void graph() {
  strokeWeight(1);
  stroke(0);
  fill(20);
  textSize(20);
  
  textAlign(CENTER, CENTER);
  text("Time : " + hour() + " : " + minute(), 650, 50);
  text("Object Temperature : " + String.format("%.2f", ObjectTemp), 300, 50);
  text("Ambient Temperature : " + String.format("%.2f", AmbientTemp), 950, 50);
  
  //Graph Object temperatue range setting
  text("(C)", 30, 100);
  
  for(int i=0; i<8; i++) 
  {
    textAlign(RIGHT, CENTER);
    text(-20+10*i, 50, 500-i*50); //vertical number setting
  }
  textAlign(CENTER, CENTER);
  text("Time", 600, 520);
  text("Max : " + maxTemp, 160, 550);
  text("Min : " + minTemp, 340, 550);
//  text("Avr : " + avgTemp, 520, 550);
  text("Avr : " + String.format("%.2f", avgTemp), 520, 550);
  line(100,100,100,500);
  line(100,500,600,500);  

  //Graph Ambient temperatue range setting
  text("(C)", 630, 100);
  for(int i=0; i<8; i++) 
  {
    textAlign(RIGHT, CENTER);
    text(-20+10*i, 680, 500-i*50); //vertical number setting
  }
  textAlign(CENTER, CENTER);
  text("Time", 1200, 520);
  text("Max : " + maxAmbient, 760, 550);
  text("Min : " + minAmbient, 940, 550);
//  text("Avr : " + avgAmbient, 1100, 550);
  text("Avr : " + String.format("%.2f", avgAmbient), 1100, 550);
  line(700,100,700,500);
  line(700,500,1200,500);
  
  for(int i=0; i<arrayTemp.length; i++) 
  {
    if(i != arrayTemp.length-1) 
    {
      textSize(10);
      strokeWeight(10);
      
      stroke(255,0,0);
      point(600-i*50, 400-arrayTemp[i]*(400*0.012));
      text(String.format("%.2f", arrayTemp[i]), 600-i*50, 400-arrayTemp[i]*(400*0.012)-20);
    }
    
    if(i == arrayTemp.length-1) continue;
    strokeWeight(1);
    
    stroke(255,0,0);
    line(600-i*50, 400-arrayTemp[i]*(400*0.012), 600-(i+1)*50, 400-arrayTemp[i+1]*(400*0.012));
 
    if(i != arrayAmbient.length-1) 
    {
      textSize(10);
      strokeWeight(10); 
      
      stroke(0,0,255);
      point(1200-i*50, 400-arrayAmbient[i]*(400*0.012));    
      text(String.format("%.2f", arrayAmbient[i]), 1200-i*50, 400-arrayAmbient[i]*(400*0.012)-20);
    }
      
    if(i == arrayAmbient.length-1) continue;
    strokeWeight(1);
 
    stroke(0,0,255);
    line(1200-i*50, 400-arrayAmbient[i]*(400*0.012), 1200-(i+1)*50, 400-arrayAmbient[i+1]*(400*0.012));
  }
}
  
