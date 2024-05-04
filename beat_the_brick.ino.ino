const int a1_pin = 5; //right motor
const int a2_pin = 6;
const int b1_pin = 9; //left motor
const int b2_pin = 10;
const int line_finder = 2;
const int trig_pin = 7;
const int echo_pin = 8;
const int led_pin = 11;
const int trig_pin_top = 12;
const int echo_pin_top = 13;


int timer_initiated = 0;
float prev_distance = 0;
float prev_distance_top = 0;
unsigned long start_time = millis();
unsigned long start_look = millis();
unsigned long curr_time = 0;

// turn around 360 degrees to scan for a closest object, and turn around again to face it
void face_closest_object() {
  
  float closest_valid_dist = 5000;
  int right = 1;

  // trial = 0 : look for the closest object (by turning 360 degrees) and save the distance in closest_valid_dist
  // trial = 1 : turn around and stop when facing the closest object (by checking closest_valid_dist)
  for(int trial=0; trial<2; trial++) {

    // if scanning for closest object (trial=0) or facing closest object which is on the right side (trial=1), turn right
    if(trial == 0 || (trial == 1 && right == 1)) {
      digitalWrite(a1_pin, LOW);
      analogWrite(a2_pin, 73);
      analogWrite(b1_pin, 65);
      digitalWrite(b2_pin, LOW);
    }

    // if facing closest object which is on the left side (trial=1), turn left
    else if(trial == 1 && right == 0) {
      analogWrite(a1_pin, 73);
      digitalWrite(a2_pin, LOW);
      digitalWrite(b1_pin, LOW);
      analogWrite(b2_pin, 65);
    }

    // while turning around, calculate distances of objects ahead
    for (int i=0; i < 100; i++){

      // calculate bottom object distance
      digitalWrite(trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig_pin, LOW);
      unsigned long duration = pulseIn(echo_pin, HIGH);
      float distance = ((float)(340 * duration) / 10000) / 2;

      // calculate top object distance
      digitalWrite(trig_pin_top, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig_pin_top, LOW);
      unsigned long duration_top = pulseIn(echo_pin_top, HIGH);
      float distance_top = ((float)(340 * duration_top) / 10000) / 2;

      // if scanning for closest object, update closest_valid_dist if a closer object is found
      if(trial == 0 && distance < 100 && distance+3 < distance_top && distance < closest_valid_dist) {        

        closest_valid_dist = distance;  

        // if i >= 50, vehicle has turned more than 180 degrees, so the closest object is in the left side  
        if(i >= 50) {
          right = 0;
        }
      }

      // if facing closest object, stop when the distance to the object ahead is within 0.5 error to closest_valid_dist
      else if(trial == 1 && distance_top - distance > 9 && distance < closest_valid_dist + 0.5 && distance > closest_valid_dist - 0.5) {
        stop();
        break;
      }
      else if(trial == 1 && i == 79) {
        turn_right();
      }
      delay(30);
    }

    // stop for 0.3 seconds
    digitalWrite(a1_pin, LOW);
    digitalWrite(a2_pin, LOW);
    digitalWrite(b1_pin, LOW);
    digitalWrite(b2_pin, LOW);
    delay(0.3 * 1000);
  }  
}

// stop for 0.5 seconds
void stop() {
  digitalWrite(a1_pin, LOW);
  digitalWrite(a2_pin, LOW);
  digitalWrite(b1_pin, LOW);
  digitalWrite(b2_pin, LOW);
  delay(0.5 * 1000);
}

// stop for 0.3 seconds, and go back for 0.8 seconds
void back() {
  digitalWrite(a1_pin, LOW);
  digitalWrite(a2_pin, LOW);
  digitalWrite(b1_pin, LOW);
  digitalWrite(b2_pin, LOW);
  delay(300);
  digitalWrite(a1_pin, LOW);
  analogWrite(a2_pin, 95);
  digitalWrite(b1_pin, LOW);
  analogWrite(b2_pin, 80);
  delay(800);
}

// turn right for 0.9 seconds
void turn_right() {
  analogWrite(a1_pin, 73);
  digitalWrite(a2_pin, LOW);
  digitalWrite(b1_pin, LOW);
  analogWrite(b2_pin, 65);
  delay(900);
}

void setup() {

  pinMode(a1_pin, OUTPUT);
  pinMode(a2_pin, OUTPUT);
  pinMode(b1_pin, OUTPUT);
  pinMode(b2_pin, OUTPUT);
  pinMode(line_finder, INPUT);

  pinMode(trig_pin_top, OUTPUT);
  pinMode(echo_pin_top, INPUT);

  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);

  pinMode(led_pin, OUTPUT);

  digitalWrite(a1_pin, LOW);
  digitalWrite(a2_pin, LOW);
  digitalWrite(b1_pin, LOW);
  digitalWrite(b2_pin, LOW);

  Serial.begin(9600);
}

void loop() {

  //calculate top distance
  digitalWrite(trig_pin_top, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin_top, LOW);
  unsigned long duration_top = pulseIn(echo_pin_top, HIGH);
  float distance_top = ((float)(340 * duration_top) / 10000) / 2;

  //calculate bottom distance
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  unsigned long duration = pulseIn(echo_pin, HIGH);
  float distance = ((float)(340 * duration) / 10000) / 2;

  // Serial.println(distance);
  // Serial.println(distance_top);
  // delay(1000);

  // go forward
  if(timer_initiated == 0) {
  analogWrite(a1_pin, 80);
  digitalWrite(a2_pin, LOW);
  analogWrite(b1_pin, 66);
  digitalWrite(b2_pin, LOW);
  }

  // when vehicle is on top of line, go back and look for closest object again
  if(digitalRead(line_finder) == 1) { 
    digitalWrite(led_pin, HIGH);
    back();

    face_closest_object();
    digitalWrite(led_pin, LOW);
  }


  // if the distance to the object ahead doesn't change for more than 3.5 seconds, the vehicle might be stuck somewhere, so go back and face the closest object again
  curr_time = millis();
  if(curr_time - start_time >= 3500) {
    start_time = curr_time;

    if(distance > 12 && distance < prev_distance + 1 && distance > prev_distance - 1) {
      digitalWrite(a1_pin, LOW);
      analogWrite(a2_pin, 85);
      digitalWrite(b1_pin, LOW);
      analogWrite(b2_pin, 80);
      delay(800);
      face_closest_object();
    }
    prev_distance = distance;
    prev_distance_top = distance_top;
  }

  // if the closest object is in front, push it
  if(distance != 0 && distance_top - distance > 9 && distance < 8) {
    analogWrite(a1_pin, 255);
    digitalWrite(a2_pin, LOW);
    analogWrite(b1_pin, 245);
    digitalWrite(b2_pin, LOW);
    delay(300);
    digitalWrite(a1_pin, LOW);
    digitalWrite(a2_pin, LOW);
    digitalWrite(b1_pin, LOW);
    digitalWrite(b2_pin, LOW);
    delay(400);
    digitalWrite(a1_pin, LOW);
    analogWrite(a2_pin, 85);
    digitalWrite(b1_pin, LOW);
    analogWrite(b2_pin, 80);
    delay(200);
  }
}