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

\*---------------------------------------------------------------------------*/

#include "PtrList.H"
#include "SLList.H"
#include "Istream.H"
#include "Ostream.H"
#include "INew.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

template<class T>
template<class INew>
void PtrList<T>::read(Istream& is, const INew& inewt)
{
    is.fatalCheck("PtrList<T>::read(Istream& is, const INew& inewt)");

    token firstToken(is);

    is.fatalCheck
    (
        "PtrList<T>::read(Istream& is, const INew& inewt) : "
        "reading first token"
    );

    if (firstToken.isLabel())
    {
        // Read size of list
        label s = firstToken.labelToken();

        setSize(s);

        // Read beginning of contents
        char listDelimiter = is.readBeginList("PtrList");

        if (s)
        {
            if (listDelimiter == token::BEGIN_LIST)
            {
                forAll(*this, i)
                {
                    set(i, inewt(is));
                
                    is.fatalCheck
                    (
                        "PtrList<T>::read(Istream& is, const INew& inewt) : "
                        "reading entry"
                    );
                }
            }
            else
            {
                T* tPtr = inewt(is).ptr();
                set(0, tPtr);

                is.fatalCheck
                (
                    "PtrList<T>::read(Istream& is, const INew& inewt) : "
                    "reading the single entry"
                );

                for (label i=1; i<s; i++)
                {
                    set(i, tPtr->clone());
                }
            }
        }

        // Read end of contents
        is.readEndList("PtrList");
    }
    else if (firstToken.isPunctuation())
    {
        if (firstToken.pToken() != token::BEGIN_LIST)
        {
            FatalIOErrorIn
            (
                "PtrList<T>::read(Istream& is, const INew& inewt)",
                is
            )   << "incorrect first token, '(', found " << firstToken.info()
                << exit(FatalIOError);
        }

        SLList<T*> sllPtrs;

        token lastToken(is);
        while
        (
           !(
                lastToken.isPunctuation()
             && lastToken.pToken() == token::END_LIST
            )
        )
        {
            is.putBack(lastToken);
            sllPtrs.append(inewt(is).ptr());
            is >> lastToken;
        }

        setSize(sllPtrs.size());

        label i = 0;
        for
        (
            typename SLList<T*>::iterator iter = sllPtrs.begin();
            iter != sllPtrs.end();
            ++iter
        )
        {
            set(i++, iter());
        }
    }
    else
    {
        FatalIOErrorIn
        (
            "PtrList<T>::read(Istream& is, const INew& inewt)",
            is
        )   << "incorrect first token, expected <int> or '(', found "
            << firstToken.info()
            << exit(FatalIOError);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
template<class INew>
PtrList<T>::PtrList(Istream& is, const INew& inewt)
{
    read(is, inewt);
}


template<class T>
PtrList<T>::PtrList(Istream& is)
{
    read(is, INew<T>());
}


// * * * * * * * * * * * * * * * Istream Operator  * * * * * * * * * * * * * //

template<class T>
Istream& operator>>(Istream& is, PtrList<T>& L)
{
    L.clear();
    L.read(is, INew<T>());

    return is;
}


// * * * * * * * * * * * * * * * Ostream Operators * * * * * * * * * * * * * //

template<class T>
Ostream& operator<<(Ostream& os, const PtrList<T>& pL)
{
    // Write size of list
    os << nl << pL.size();

    // Write beginning of contents
    os << nl << token::BEGIN_LIST;

    // Write list contents
    forAll(pL, i)
    {
        os << nl << pL[i];
    }

    // Write end of contents
    os << nl << token::END_LIST << nl;

    // Check state of IOstream
    os.check("Ostream& operator<<(Ostream&, const PtrList&)");

    return os;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
