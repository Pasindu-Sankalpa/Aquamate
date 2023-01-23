#define PIN_A0 35          // the pH meter Analog output is connected with the Arduino’s Analog
int avg_phValue;   //Store the average value of the sensor feedback
float phValue;
int ph_buf[10],temp;

void setup()
{
  Serial.begin(9600);
  Serial.println("Ready");    //Test the serial monitor
}

void loop()
{
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  {
    ph_buf[i]=analogRead(PIN_A0);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(ph_buf[i]>ph_buf[j])
      {
        temp=ph_buf[i];
        ph_buf[i]=ph_buf[j];
        ph_buf[j]=temp;
      }
    }
  }
  avg_phValue=0;
  for(int i=2;i<8;i++)            //take the average value of 6 center sample
    avg_phValue+=ph_buf[i];
  phValue=avg_phValue*3.3/4095/6; //convert the analog into millivolt
  phValue=phValue*3.3;            //convert the millivolt into pH value
  Serial.print("    pH:");
  Serial.print(phValue, 2);
  Serial.println(" ");
  delay(1000);
}
