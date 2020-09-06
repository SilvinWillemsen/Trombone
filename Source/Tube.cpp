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
    vVecs.resize (2);
    pVecs.resize (2);

    for (int i = 0; i < 2; ++i)
    {
        vVecs[i] = std::vector<double> (N, 0);
        pVecs[i] = std::vector<double> (N, 0);
    }
    
    if (raisedCos)
    {
        int start = N * 0.5 - 5;
        int end = N * 0.5 + 5;
        double scaling = 100000.0;
        for (int n = 0; n < 2; ++n)
        {
            for (int l = start; l < end; ++l)
            {
                pVecs[n][l] = scaling * (1.0 - cos (2.0 * double_Pi * (l-start) / static_cast<float>(end - start))) * 0.5;
            }
        }
    }
    
    v.resize (2);
    p.resize (2);

    for (int i = 0; i < 2; ++i)
    {
        v[i] = &vVecs[i][0];
        p[i] = &pVecs[i][0];
    }
    
    calculateGeometry (parameters);
    calculateRadii();
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

    g.setColour (Colours::gold);
    Path stringPathTop = drawGeometry (g, -1);
    Path stringPathBottom = drawGeometry (g, 1);
    g.strokePath (stringPathTop, PathStrokeType(2.0f));
    g.strokePath (stringPathBottom, PathStrokeType(2.0f));
//    std::cout << "repainted" << std::endl;

}

Path Tube::drawGeometry (Graphics& g, int topOrBottom)
{
    double visualScaling = 1000.0;
    Path stringPath;
    stringPath.startNewSubPath (0, topOrBottom * radii[0] * visualScaling + getHeight() * 0.5);
    int stateWidth = getWidth();
    auto spacing = stateWidth / static_cast<double>(N);
    auto x = spacing;
    
    for (int y = 1; y < N; y++)
    {
        stringPath.lineTo(x, topOrBottom * radii[y] * visualScaling + getHeight() * 0.5);

        x += spacing;
    }
    return stringPath;
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
        v[0][l] = v[1][l] - lambdaOverRhoC * (p[1][l+1] - p[1][l]);
}

void Tube::calculatePressure()
{
    for (int l = 1; l < N - 1; ++l)
        p[0][l] = p[1][l] - rho * c * lambda * oOSBar[l] * (SHalf[l] * v[0][l] - SHalf[l-1] * v[0][l-1]);
    
    p[0][0] = p[1][0] - rho * c * lambda * oOSBar[0] * (-2.0 * (Ub + Ur) + 2.0 * SHalf[0] * v[0][0]);
    
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
    oOSBar.resize (N, 0);
    
    double mp = *parameters.getVarPointer ("mp");
    double tubeS = *parameters.getVarPointer ("tubeS");

    int mpL = N * double (*parameters.getVarPointer ("mpL"));
    int m2tL = N * double (*parameters.getVarPointer ("m2tL"));
    int bellL = N * double (*parameters.getVarPointer ("bellL"));
    
    double flare = *parameters.getVarPointer ("flare");
    double x0 = *parameters.getVarPointer ("x0");
    double b = *parameters.getVarPointer ("b");

    for (int i = 0; i < N; ++i)
    {
        if (i < mpL)
            S[i] = mp;
        else if (i >= mpL && i < mpL + m2tL)
            S[i] = Global::linspace (mp, tubeS, m2tL, i-mpL);
        else if (i >= mpL + m2tL && i < N - bellL)
            S[i] = tubeS;
        else
            S[N-1-(i - (N - bellL))] = pow(b * pow(((i - (N - bellL)) / (2.0 * bellL) + x0), -flare), 2) * double_Pi;
    }
    
    for (int i = 0; i < N - 1; ++i)
        SHalf[i] = (S[i] + S[i+1]) * 0.5;
    
    SBar[0] = S[0];
    
    for (int i = 0; i < N - 2; ++i)
        SBar[i+1] = (SHalf[i] + SHalf[i+1]) * 0.5;
    
    SBar[N-1] = S[N-1];
    for (int i = 0; i < N; ++i)
        oOSBar[i] = 1.0 / SBar[i];
}

void Tube::calculateRadii()
{
    radii.resize (N, 0);
    for (int i = 0; i < N; ++i)
        radii[i] = sqrt (S[i]) / double_Pi;
    
}
double Tube::getKinEnergy()
{
    double kinEnergy = 0;
    for (int i = 0; i < N; ++i)
    {
        kinEnergy += 1.0 / (2.0 * rho * c * c) * h * (SBar[i] * p[1][i] * p[1][i] * (i == 0 || i == N-1 ? 0.5 : 1));
    }
    if (kinEnergy1 < 0)
        kinEnergy1 = kinEnergy;
    return kinEnergy;

}

double Tube::getPotEnergy()
{
    double potEnergy = 0;
    for (int i = 0; i < N-1; ++i)
        potEnergy += rho * 0.5 * h * (SHalf[i] * v[0][i] * v[1][i]);
    
    if (potEnergy1 < 0)
        potEnergy1 = potEnergy;
    
    return potEnergy;
}
