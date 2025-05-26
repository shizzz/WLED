#pragma once
#include "MonitorTypes.h"
#include "usermod_v2_klipper_monitor.h"

class QueueManager;

class QueueManager
{
private:

    unsigned long _interval;
    unsigned long _lastRunTime;
    unsigned int _totalWeight = 0;
    unsigned int _currentWeightCounter = 0;
    bool _active = false;

    PresetSettings* _activePreset;
    PresetSettings* getNextPreset();
public:
    QueueManager(unsigned long interval = 100) : _interval(interval), _lastRunTime(0) {}

    ~QueueManager() {
        _activePreset = nullptr;
        _presetPool.clear();
        _totalWeight = 0;
        _currentWeightCounter = 0;
    }

    std::vector<PresetSettings*> _presetPool;
    void addPreset(PresetSettings* preset);
    void tick();
    void setInterval(unsigned long interval);
    void setActivePreset(uint16_t presetId);
    bool getActive();
    PresetSettings* activePreset();
    std::vector<PresetSettings*> getPresetPool();
};
