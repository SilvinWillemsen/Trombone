/*
  ==============================================================================

    Tube.cpp
    Created: 5 Sep 2020 1:11:57pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Tube.h"

//==============================================================================
Tube::Tube (NamedValueSet& parameters, double k) : k (k), L (*parameters.getVarPointer("L"))
{
    calculateThermodynamicConstants();
    
    h = c * k;
    N = floor(L / h);
    h = L / static_cast<double> (N);
    
    lambda = c * k / h;
    lambdaOverRhoC = lambda / (rho * c);
    
    // initialise state vectors
    vVecs.reserve (2);
    pVecs.reserve (2);

    for (int i = 0; i < 2; ++i)
    {
        vVecs.push_back (std::vector<double> (N, 0));
        pVecs.push_back (std::vector<double> (N, 0));
    }
    v.resize (2);
    p.resize (2);

    for (int i = 0; i < 2; ++i)
    {
        v[i] = &vVecs[i][0];
        p[i] = &pVecs[i][0];
    }
    
    calculateGeometry (parameters);
    
}

Tube::~Tube()
{
}

void Tube::paint (juce::Graphics& g)
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
    g.drawText ("Tube", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Tube::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Tube::calculateThermodynamicConstants()
{
    double deltaT = T - 26.85;
    c = 3.4723e2 * (1 + 0.00166 * deltaT);      // Speed of sound in air [m/s]
    rho = 1.1769 * (1 - 0.00335 * deltaT);      // Density of air [kg·m^{-3}]
//    eta = 1.846 * (1 + 0.0025 * deltaT);        // Shear viscosity [kg·s^{-1}·m^{-1}]
//    nu = 0.8410 * (1 - 0.0002 * deltaT);        // Root of Prandtl number [-]
//    gamma = 1.4017 * (1 - 0.00002 * deltaT);    // Ratio of specific heats [-]
    
}

void Tube::calculateVelocity()
{
    for (int l = 0; l < N - 1; ++l)
        v[0][l] = v[1][l] - lambdaOverRhoC * (p[l+1] - p[l]);
}

void Tube::calculatePressure()
{
    for (int l = 1; l < N - 1; ++l)
        p[0][l] = p[1][l] - rho * c * lambda / SBar[l] * (SHalf[l] * v[0][l] - SHalf[l-1] * v[0][l-1]);
    
    p[0][0] = p[1][0] - rho * c * lambda / SBar[0] * (-2.0 * (Ub + Ur) + 2.0 * SHalf[0] * v[0][0]);
    
}

void Tube::updateStates()
{
    vTmp = v[1];
    v[1] = v[0];
    v[0] = vTmp;
    
    pTmp = p[1];
    p[1] = p[0];
    p[0] = pTmp;
}

void Tube::calculateGeometry (NamedValueSet& parameters)
{
    S.resize (N, 0);
    SHalf.resize (N-1, 0);
    SBar.resize (N, 0);
    
    double mp = *parameters.getVarPointer ("mp");
    double tubeS = *parameters.getVarPointer ("tubeS");

    int mpL = N * double (*parameters.getVarPointer ("mpL"));
    int m2tL = N * double (*parameters.getVarPointer ("m2tL"));
    int bellL = N * double (*parameters.getVarPointer ("bellL"));
    
    double flare = *parameters.getVarPointer ("flare");
    
    for (int i = 0; i < N; ++i)
    {
        if (i < mpL)
            S[i] = mp;
        else if (i >= mpL && i < mpL + m2tL)
            S[i] = Global::linspace (mp, tubeS, m2tL, i-mpL);
        else if (i >= mpL + m2tL && i < N - bellL)
            S[i] = tubeS;
        else
            S[i] = tubeS * exp (flare * (i - (N - bellL)));
    }
    
    for (int i = 0; i < N - 1; ++i)
        SHalf[i] = (S[i] + S[i+1]) * 0.5;
    
    SBar[0] = S[0];
    
    for (int i = 0; i < N - 2; ++i)
        SBar[i+1] = (SHalf[i] + SHalf[i+1]) * 0.5;
    
    SBar[N-1] = S[N-1];
//
//    mp = linspace(0.001, 0.001, floor(N/40));           % mouthpiece
//    m2t = linspace(mp(end), 0.0005, floor(N/40));       % mouthpiece to tube
//
//    alpha = 0.25;                                       % bell
//    b = m2t(end) * exp(alpha * (0:18));
//    pointsLeft = N - length([mp, m2t, b]);
//    tube = linspace(m2t(end), m2t(end), pointsLeft);    % tube
//
//    S = [mp, m2t, tube, b]';                            % True geometry
//    %     S = 0.005 * (ones(length(S), 1));
//    % Calculate approximations to the geometry
//    SHalf = (S(1:N-1) + S(2:N)) * 0.5;                  % mu_{x+}
//    SBar = (SHalf(1:end-1) + SHalf(2:end)) * 0.5;
//    SBar = [S(1); SBar; S(end)];                        % mu_{x-}S_{l+1/2}
}

