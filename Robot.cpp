#include "robot.h"


/*Constructor for Robot Object
 *Initializes values for Robot
 */
Robot::Robot(){
    //Set values to pointers
    leftMotor = new FEHMotor(FEHMotor::Motor0);
    rightMotor = new FEHMotor(FEHMotor::Motor1);

    cdsCell = new AnalogInputPin(FEHIO::P0_0);

    LRservo = new FEHServo(FEHServo::Servo0);
    UDservo = new FEHServo(FEHServo::Servo1);

    rps = new FEHWONKA();

    leftEncoder = new FEHEncoder(FEHIO::P1_0);
    rightEncoder = new FEHEncoder(FEHIO::P1_4);

    switch1 = new DigitalInputPin(FEHIO::P2_0);
    switch2 = new DigitalInputPin(FEHIO::P2_1);
    switch3 = new DigitalInputPin(FEHIO::P2_2);

    //declare constants
    startLightValue = 10;
    scoopLightValue = 20;
    PI = 3.14159265;
    TOTALCOUNTS = 20;
    WHEELRADIUS = 1.35;

    //Calibrate servos
    LRservo->SetMin(500);
    LRservo->SetMax(2366);
    UDservo->SetMin(500);
    UDservo->SetMax(2328);


    rps->InitializeMenu();
    LCD.WriteLine("Sucessfully Initialized RPS");
    rps->Enable();
    LCD.WriteLine("Sucessfully Enabled RPS");

    ovenCount = rps->Oven();

    LCD.WriteLine("Successfully initialized robot");
    Sleep(1.0);
    LCD.Clear(FEHLCD::Black);
    printStartScreen();
    while(switch1->Value() && switch2->Value() && switch3->Value());
}

int Robot::getOvenCount(){
    return ovenCount;
}

void Robot::findAngle(){
    bool finished = false;
    int LRangle = 90;
    int UDangle = 90;

    LRservo->SetDegree(LRangle);
    UDservo->SetDegree(UDangle);

    while(!finished){
        //print menu
        LCD.Clear(FEHLCD::Black);
        LCD.WriteLine("");
        LCD.WriteLine("Press button to select servo: ");

        //wait for input
        while(switch1->Value() && switch2->Value() && switch3->Value());

        //decision tree
        if(!switch1->Value()){
            LCD.WriteLine("UD servo selected. Please release button.");
            Sleep(1.0);
            bool innerFinish = false;
            while(!innerFinish){
                LCD.Clear(FEHLCD::Black);
                LCD.Write("UD servo angle: ");
                LCD.WriteLine(UDangle);
                UDservo->SetDegree(UDangle);
                //wait for input
                while(switch1->Value()&&switch2->Value()&&switch3->Value());
                if(!switch1->Value()){
                    UDangle--;
                }
                else if(!switch2->Value()){
                    innerFinish = true;
                }
                else if(!switch3->Value()){
                    UDangle++;
                }
                Sleep(.03);
            }
            finished = false;

        }
        else if(!switch2->Value()){
            finished = true;
        }
        else if(!switch3->Value()){
            LCD.WriteLine("LR servo selected. Please release button.");
            Sleep(1.0);
            bool innerFinish = false;
            while(!innerFinish){
                LCD.Clear(FEHLCD::Black);
                LCD.Write("LR servo angle: ");
                LCD.WriteLine(LRangle);
                LRservo->SetDegree(LRangle);
                //wait for input
                while(switch1->Value()&&switch2->Value()&&switch3->Value());
                if(!switch1->Value()){
                    LRangle--;
                }
                else if(!switch2->Value()){
                    innerFinish = true;
                }
                else if(!switch3->Value()){
                    LRangle++;
                }
                Sleep(.03);
            }
            finished = false;
        }
        Sleep(.5);
    }
}

int Robot::checkCDS(){
    int cdsPercentage = (cdsCell->Value() / 3.3) * 100;
    return cdsPercentage;
}

bool Robot::cdsStart(){
    bool ready = false;
    if(cdsCell->Value() < startLightValue){
        ready = true;
    }
    return ready;
}

int Robot::cdsColor(){
    float redVal = 5;
    float blueVal = 22;
    float blackVal = 50;
    if(checkCDS() < 12){
        return REDLIGHT;
    }
    else if((checkCDS() > 15)&&(checkCDS() < 40)){
        return BLUELIGHT;
    }
    else if(checkCDS() >= 40){
        return 0;
    }
    else{
        return 0;
    }
}

/*Move forward a specific distance
 *distance given in INCHES
 *because we're in AMERICA
 *Land of the Free. Home of the Brave.*/

void Robot::moveForward(float distance, int power){
    int motorPercent = power;

    leftEncoder->ResetCounts();
    rightEncoder->ResetCounts();


    int estCounts = (TOTALCOUNTS*distance)/(2*PI*WHEELRADIUS);

    leftMotor->SetPercent(motorPercent);
    rightMotor->SetPercent(motorPercent-(power/15.0));

    int avgCounts = (leftEncoder->Counts() + rightEncoder->Counts())/2.0;

    //while the average counts between the wheels are less than the final desired count
    while(avgCounts < estCounts){
        //normal run
        if(avgCounts > 4){
            rightMotor->SetPercent(motorPercent+((leftEncoder->Counts()) - rightEncoder->Counts()));
        }
        avgCounts = (leftEncoder->Counts() + rightEncoder->Counts())/2.0;
    }
    //stop wheel movement
    leftMotor->SetPercent(0);
    rightMotor->SetPercent(0);
}

void Robot::moveBackward(float distance, int power){
    int motorPercent= power*(-1);

    leftEncoder->ResetCounts();
    rightEncoder->ResetCounts();


    int estCounts = (TOTALCOUNTS*distance)/(2*PI*WHEELRADIUS);

    leftMotor->SetPercent(motorPercent);
    rightMotor->SetPercent(motorPercent);

    int avgCounts = (leftEncoder->Counts() + rightEncoder->Counts())/2.0;
    //while the average counts between the wheels are less than the final desired count
    while(avgCounts < estCounts){
        //normal run
        LCD.Write("Left: ");
        LCD.Write(leftEncoder->Counts());
        LCD.Write("  Right: ");
        LCD.WriteLine(rightEncoder->Counts());
        if(avgCounts > 4){
            rightMotor->SetPercent(motorPercent+(rightEncoder->Counts() - leftEncoder->Counts()));
            LCD.Write("Correction: ");
            LCD.WriteLine(rightEncoder->Counts() - leftEncoder->Counts());
        }
        avgCounts = (leftEncoder->Counts() + rightEncoder->Counts())/2.0;
    }
    //stop wheel movement
    leftMotor->SetPercent(0);
    rightMotor->SetPercent(0);
}

void Robot::timeBack(int time, int power){
    leftMotor->SetPercent(power*(-1));
    rightMotor->SetPercent(power*(-1));
    Sleep(time*1.0);
    leftMotor->SetPercent(0);
    rightMotor->SetPercent(0);
}

void Robot::turnLeft(int degrees){
    int motorPercent= 40;
    int tolerance = 3;

    rps->Enable();
    leftEncoder->ResetCounts();
    rightEncoder->ResetCounts();

    int startDeg = rps->Heading();

    int finalDeg = startDeg + degrees;
    if(finalDeg >= 180){
        finalDeg -= 180;
    }

    LCD.Write("Final: ");
    LCD.WriteLine(finalDeg);
    Sleep(.5);
    leftMotor->SetPercent((-1)*motorPercent);
    rightMotor->SetPercent(motorPercent);

    while(!((rps->Heading() <= finalDeg + tolerance)&&( finalDeg - tolerance <= rps->Heading()))){
        LCD.Write("Current: ");
        LCD.WriteLine(rps->Heading());
    }

    //stop wheel movement
    leftMotor->SetPercent(0);
    rightMotor->SetPercent(0);

}

void Robot::turnRight(int degrees){
    int motorPercent= 40;
    int tolerance = 3;

    rps->Enable();
    leftEncoder->ResetCounts();
    rightEncoder->ResetCounts();

    int startDeg = rps->Heading();

    int finalDeg = startDeg-degrees;
    if(finalDeg < 0){
        finalDeg += 180;
    }

    LCD.Write("Final: ");
    LCD.WriteLine(finalDeg);
    Sleep(.5);
    leftMotor->SetPercent(motorPercent);
    rightMotor->SetPercent((-1)*motorPercent);

    while(!((rps->Heading() <= finalDeg + tolerance)&&( finalDeg - tolerance <= rps->Heading()))){
        LCD.Write("Current: ");
        LCD.WriteLine(rps->Heading());
    }

    //stop wheel movement
    leftMotor->SetPercent(0);
    rightMotor->SetPercent(0);
}

void Robot::setArmAngle(int LRangle, int UDangle){
    UDservo->SetDegree(UDangle);
    LRservo->SetDegree(LRangle);
}

void Robot::printStartScreen(){
    LCD.WriteLine("Connections: ");
    LCD.WriteLine("P0_0 - CDS Cell");
    LCD.WriteLine("P1_0 - Left Wheel Encoder");
    LCD.WriteLine("P1_4 - Right Wheel Encoder");
    LCD.WriteLine("P2_0..2 - Switch Board");
    LCD.WriteLine("MOT0 - Left Wheel Motor");
    LCD.WriteLine("MOT1 - Right Wheel Motor");
    LCD.WriteLine("SERVO0 - LR Arm Servo");
    LCD.WriteLine("SERVO1 - UD Arm Servo");
}

bool Robot::validRPS(){
    bool working = false;
    rps->Enable();
    LCD.Clear(FEHLCD::Black);
    LCD.Write("Please Move RPS Around.");
    for(int i = 0; i < 100; i++){
        LCD.WriteLine(rps->Heading());
        if(rps->Heading() != 0){
            working = true;
        }
        Sleep(.01);
    }
    LCD.Write("Waiting for Packet");
    if(rps->WaitForPacket() != 0){
        working = true;
    }
    else{
        working = false;
    }

    return working;
}
