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
    Foam::LindemannFallOffFunction

Description
    Lindemann fall-off function

SourceFiles
    LindemannFallOffFunctionI.H

\*---------------------------------------------------------------------------*/

#ifndef LindemannFallOffFunction_H
#define LindemannFallOffFunction_H

#include "scalar.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

class LindemannFallOffFunction;
Ostream& operator<<(Ostream&, const LindemannFallOffFunction&);


/*---------------------------------------------------------------------------*\
                           Class LindemannFallOffFunction Declaration
\*---------------------------------------------------------------------------*/

class LindemannFallOffFunction
{

public:

    // Constructors

        //- Construct null
        inline LindemannFallOffFunction();

        //- Construct from Istream
        inline LindemannFallOffFunction(Istream&);


    // Member Functions

        //- Return the type name
        static word type()
        {
            return "Lindemann";
        }

        inline scalar operator()
        (
            const scalar T,
            const scalar Pr
        ) const;


    // Ostream Operator

        friend Ostream& operator<<(Ostream&, const LindemannFallOffFunction&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "LindemannFallOffFunctionI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
