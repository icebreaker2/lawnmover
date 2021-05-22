#ifndef IRCTRL_H
#define IRCTRL_H

#include <Arduino.h>
#include <IRremote.h>
#include "Mover.h"
#include "Motor.h"

// TODO make singleton as implementation uses static default IrReceive object

class IRCtrl {
    public:
        IRCtrl(const int irReceivePin, MoverService &moverService, MotorService &motorService);
        ~IRCtrl();

    private:
        const int kIrReceivePin;
        MoverService _moverService;
        MotorService _motorService;
        Timer<> _timer;

        boolean rawSpinCheck();

        void checkAndApplyIRCommand();
};


#endif // IRCTRL_H
