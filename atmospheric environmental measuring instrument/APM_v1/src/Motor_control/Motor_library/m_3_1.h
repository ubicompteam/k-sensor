/*
APM의 모터를 구동하기 위한 라이브러리 ver1.0
*/

#ifndef m_3_1_h
#define m_3_1_h

class m_3_1
{
    public:
        m_3_1(int enPin, int stepPin, int dirPin, int swich);

        //reset motor
        void BottomSetup();
        void TopSetup();

        void setspeed(long inputSpeed);
        void moveto();
        void moveStep(long steps);
        void setRound(long inputRound);
        void setDiraction();
        void stopMotor();
        void startMotor();

        //pm2,3을 위한 함수
        void moveto_pm();

        //pm1의 위치 조정을 위한 함수
        void TopLocation(int num);

        //bottom의 위치 조정을 위한 함수
        void BottomLocation(int inputAngle);
        
    private:
        //motor pin
        int enPin;
        int stepPin;
        int dirPin;

        //switch pin
        int swich;

        //motor speed
        int speeds;

        //motor round
        long rounds;

        //motor save steps
        long leftSteps;

        //pre millis
        unsigned long preMillis;

        //motor diraction
        bool diraction;
};

#endif
