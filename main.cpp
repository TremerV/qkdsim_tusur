#include <unistd.h>
#include <conserial.h>
#include <iostream>
using namespace hwe;
using namespace std;
int main()
{


    Conserial example;
    cout << "Init"<<endl;
    example.Init();
     cout << "InitByButtons"<<endl;
    example.InitByButtons({1,1,1,1});
     cout << "Sendmessage"<<endl;
    example.Sendmessage({1,1,1,1},25);
     cout << "SetPlateAngle"<<endl;
    example.SetPlateAngle(1,40);
     cout << "GetPlatesAngles"<<endl;
    example.GetPlatesAngles();
     cout << "GetStartPlatesAngles"<<endl;
    example.GetStartPlatesAngles();

    //Состояние лазера
     cout << "SetLaserState"<<endl;
    example.SetLaserState(1);
     cout << "GetLaserState"<<endl;
    example.GetLaserState();
     cout << "SetLaserState"<<endl;
    example.SetLaserState(0);
     cout << "GetLaserState"<<endl;
    example.GetLaserState();
     cout << "GetMaxLaserPower"<<endl;
    example.GetMaxLaserPower();
     cout << "GetSignalLevels"<<endl;
    example.GetSignalLevels();
     cout << "GetLightNoises"<<endl;
    example.GetLightNoises();
     cout << "GetMaxSignalLevels"<<endl;
    example.GetMaxSignalLevels();
     cout << "GetRotateStep"<<endl;
    example.GetRotateStep();
     cout << "GetErrorCode"<<endl;
    example.GetErrorCode();
     cout << "RunTest"<<endl;
    example.RunTest(1);

    //Мощность лазера
     cout << "SetLaserPower"<<endl;
    example.SetLaserPower(0);
     cout << "GetLaserPower"<<endl;
    example.GetLaserPower();
     cout << "SetLaserPower"<<endl;
    example.SetLaserPower(50);
     cout << "GetLaserPower"<<endl;
    example.GetLaserPower();



    return 0;
}

