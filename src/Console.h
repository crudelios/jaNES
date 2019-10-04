#pragma once
class Console
{
public:
    void InsertCartridge();
    void DisplayDebugInfo(bool display = true);
    void PowerOn();
    void PowerOff();
    void Reset();
    bool HasProblem();
    void Resume();
    void Suspend();
    void LimitFPS();
};

