/*
  ==============================================================================

    Trombone.h
    Created: 5 Sep 2020 1:12:46pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "Tube.h"
#include "LipModel.h"

//==============================================================================
/*
*/
class Trombone  : public juce::Component
{
public:
    Trombone (NamedValueSet& parameters, double k);
    ~Trombone() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void calculate();
    float getOutput() { return lipModel->getY(); };
    void updateStates();
private:
    std::unique_ptr<Tube> tube;
    std::unique_ptr<LipModel> lipModel;
    
    double k, Pm;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trombone)
};
