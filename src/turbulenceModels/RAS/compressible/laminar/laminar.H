/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    Foam::compressible::RASModels::laminar

Description
    Dummy turbulence model for laminar compressible flow.

SourceFiles
    laminar.C
    laminarCorrect.C

\*---------------------------------------------------------------------------*/

#ifndef compressibleLaminar_H
#define compressibleLaminar_H

#include "RASModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace compressible
{
namespace RASModels
{

/*---------------------------------------------------------------------------*\
                           Class laminar Declaration
\*---------------------------------------------------------------------------*/

class laminar
:
    public RASModel
{

public:

    //- Runtime type information
    TypeName("laminar");

    // Constructors

        //- from components
        laminar
        (
            const volScalarField& rho,
            const volVectorField& U,
            const surfaceScalarField& phi,
            basicThermo& thermophysicalModel
        );


    // Destructor

        ~laminar()
        {}


    // Member Functions

        //- Return the turbulence viscosity, i.e. 0 for laminar flow
        tmp<volScalarField> mut() const;

        //- Return the effective viscosity, i.e. the laminar viscosity
        tmp<volScalarField> muEff() const
        {
            return tmp<volScalarField>(new volScalarField("muEff", mu()));
        }

        //- Return the effective turbulent thermal diffusivity,
        //  i.e. the laminar thermal diffusivity
        tmp<volScalarField> alphaEff() const
        {
            return tmp<volScalarField>(new volScalarField("alphaEff", alpha()));
        }

        //- Return the turbulence kinetic energy, i.e. 0 for laminar flow
        tmp<volScalarField> k() const;

        //- Return the turbulence kinetic energy dissipation rate,
        //  i.e. 0 for laminar flow
        tmp<volScalarField> epsilon() const;

        //- Return the Reynolds stress tensor, i.e. 0 for laminar flow
        tmp<volSymmTensorField> R() const;

        //- Return the effective stress tensor, i.e. the laminar stress
        tmp<volSymmTensorField> devRhoReff() const;

        //- Return the source term for the momentum equation
        tmp<fvVectorMatrix> divDevRhoReff(volVectorField& U) const;

        //- Correct the laminar viscosity
        void correct();

        //- Read turbulenceProperties dictionary
        bool read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace compressible
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
