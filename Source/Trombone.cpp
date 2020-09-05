/*
  ==============================================================================

    Trombone.cpp
    Created: 5 Sep 2020 1:12:46pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Trombone.h"

//==============================================================================
Trombone::Trombone (NamedValueSet& parameters, double k) : k (k),
Pm (*parameters.getVarPointer ("Pm"))
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    tube = std::make_unique<Tube> (parameters, k);
    lipModel = std::make_unique<LipModel> (parameters, k);
    
    lipModel->setTubeParameters (tube->getH(),
                                 tube->getRho(),
                                 tube->getC(),
                                 tube->getSBar(0),
                                 tube->getSHalf(0));
}

Trombone::~Trombone()
{
}

void Trombone::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("Trombone", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Trombone::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Trombone::calculate()
{
    tube->calculateVelocity();
    lipModel->setTubeStates (tube->getP (1, 0), tube->getV (0, 0));
    lipModel->calculateCollision();
    lipModel->calculateDeltaP();
    lipModel->calculate();
    tube->setFlowVelocities (lipModel->getUb(), lipModel->getUr());
    tube->calculatePressure();
}

void Trombone::updateStates()
{
    tube->updateStates();
    lipModel->updateStates();
}