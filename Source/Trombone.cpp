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
    addAndMakeVisible (tube.get());
    lipModel = std::make_unique<LipModel> (parameters, k);
    lipModel->setTubeParameters (tube->getH(),
                                 tube->getRho(),
                                 tube->getC(),
                                 tube->getSBar(0),
                                 tube->getSHalf(0));
    addAndMakeVisible (lipModel.get());

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
}

void Trombone::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totArea = getLocalBounds();
    tube->setBounds (totArea.removeFromTop (getHeight() * 0.5));
    lipModel->setBounds (totArea);

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
    
//    calculateEnergy();
}

void Trombone::calculateEnergy()
{
    double totEnergy = tube->getKinEnergy() + tube->getPotEnergy() + lipModel->getLipEnergy() + lipModel->getCollisionEnergy();
    double energy1 = tube->getKinEnergy1() + tube->getPotEnergy1() + lipModel->getLipEnergy1() + lipModel->getCollisionEnergy1();
    
    double scaledTotEnergy = (totEnergy + lipModel->getPower() + lipModel->getDampEnergy() - energy1) / energy1;
//    std::cout << scaledTotEnergy << std::endl;
}

void Trombone::updateStates()
{
    tube->updateStates();
    lipModel->updateStates();
}
