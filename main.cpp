#include <unistd.h>
#include <conserial.h>
using namespace hwe;
int main()
{


    Conserial example;

    example.Init();
    example.InitByButtons();
    example.Sendmessage({1,1,1,1},25);
    example.SetPlateAngle(1,40);

    example.GetPlatesAngles();
    example.GetStartPlatesAngles();

    //Состояние лазера
    example.SetLaserState(1);
    example.GetLaserState();
    example.SetLaserState(0);
    example.GetLaserState();
    example.GetMaxLaserPower();

    example.GetSignalLevels();
    example.GetLightNoises();
    example.GetMaxSignalLevels();
    example.GetRotateStep();
    example.GetErrorCode();
    example.RunTest(1);

    //Мощность лазера
    example.SetLaserPower(0);
    example.GetLaserPower();
    example.SetLaserPower(50);
    example.GetLaserPower();



    return 0;
}
