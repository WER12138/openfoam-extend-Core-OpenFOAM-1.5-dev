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
    Foam::passiveParticle

Description

SourceFiles
    passiveParticleI.H
    passiveParticle.C
    passiveParticleIO.C

\*---------------------------------------------------------------------------*/

#ifndef passiveParticle_H
#define passiveParticle_H

#include "Particle.H"
#include "IOstream.H"
#include "autoPtr.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class passiveParticle Declaration
\*---------------------------------------------------------------------------*/

class passiveParticle
:
    public Particle<passiveParticle>
{

public:

    // Constructors

        //- Construct from components
        passiveParticle
        (
            const Cloud<passiveParticle>& c,
            const vector& position,
            const label celli
        )
        :
            Particle<passiveParticle>(c, position, celli)
        {}

        //- Construct from Istream
        passiveParticle
        (
            const Cloud<passiveParticle>& c,
            Istream& is,
            bool readFields = true
        )
        :
            Particle<passiveParticle>(c, is)
        {}

        //- Construct and return a clone
        autoPtr<passiveParticle> clone() const
        {
            return autoPtr<passiveParticle>(new passiveParticle(*this));
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
