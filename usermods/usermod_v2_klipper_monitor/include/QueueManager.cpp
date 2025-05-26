#include "QueueManager.h"
#include <vector>

// Add an entity to the queue
void QueueManager::addPreset(PresetSettings* entity) {
    _presetPool.push_back(entity);
    _totalWeight += entity->weight;

    if (!_active) {
        _active = true;
    }
}

// Run the queue manager - call this in your loop()
void QueueManager::tick() {
    unsigned long currentTime = millis();
    
    // Check if enough time has passed since last run
    if (currentTime - _lastRunTime < _interval) {
        return;
    }
    
    _lastRunTime = currentTime;

    if (_presetPool.empty()) return;

    // Find the next entity to run based on weights
    _activePreset = getNextPreset();
}

// Set the interval between task executions
void QueueManager::setInterval(unsigned long interval) {
    _interval = interval;
}

void QueueManager::setActivePreset(uint16_t presetId)
{
    presetId = presetId - 1;
    if (presetId < 0 || presetId > PRESET_COUNT - 1)
    {
        _active = false;
        _activePreset = nullptr;
        return;
    }
    
    _active = true;
    _activePreset = _presetPool[presetId];
}

// Select the next entity to run based on weights
PresetSettings* QueueManager::getNextPreset() {
    if (_totalWeight == 0) return nullptr;

    // Round-robin with weight consideration
    while (true) {
        for (auto entity : _presetPool) {
            _currentWeightCounter += entity->weight;
            if (_currentWeightCounter >= _totalWeight) {
                _currentWeightCounter = 0;
                return entity;
            }
        }
    }
}

PresetSettings* QueueManager::activePreset() {
    return _activePreset;
}

bool QueueManager::getActive() {
    return _active;
}

std::vector<PresetSettings*> QueueManager::getPresetPool() {
    return _presetPool;
}