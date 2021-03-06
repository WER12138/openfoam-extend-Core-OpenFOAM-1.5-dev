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

#include "error.H"
#include "Swap.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
inline Foam::UList<T>::UList()
:
    v_(0),
    size_(0)
{}


template<class T>
inline Foam::UList<T>::UList(T* __restrict__ v, label size)
:
    v_(v),
    size_(size)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Reset in slicing.  HJ. 19/Oct/2008
template<class T>
inline void Foam::UList<T>::reset(T* __restrict__ v, label size)
{
    v_ = v;
    size_ = size;
}


template<class T>
inline Foam::label Foam::UList<T>::fcIndex(const label i) const
{
    return (i == size()-1 ? 0 : i+1); 
}


template<class T>
inline Foam::label Foam::UList<T>::rcIndex(const label i) const
{
    return (i == 0 ? size()-1 : i-1); 
}


// Check start is within valid range (0 ... size-1).
template<class T>
inline void Foam::UList<T>::checkStart(const label start) const
{
    if (start<0 || (start && start>=size_))
    {
        FatalErrorIn("UList<T>::checkStart(const label)")
            << "start " << start << " out of range 0 ... " << max(size_-1, 0)
            << abort(FatalError);
    }
}


// Check size is within valid range (0 ... size).
template<class T>
inline void Foam::UList<T>::checkSize(const label size) const
{
    if (size<0 || size>size_)
    {
        FatalErrorIn("UList<T>::checkSize(const label)")
            << "size " << size << " out of range 0 ... " << size_
            << abort(FatalError);
    }
}


// Check index i is within valid range (0 ... size-1).
template<class T>
inline void Foam::UList<T>::checkIndex(const label i) const
{
    if (!size_)
    {
        FatalErrorIn("UList<T>::checkIndex(const label)")
            << "attempt to access element from zero sized list"
            << abort(FatalError);
    }
    else if (i<0 || i>=size_)
    {
        FatalErrorIn("UList<T>::checkIndex(const label)")
            << "index " << i << " out of range 0 ... " << size_-1
            << abort(FatalError);
    }
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

// Return subscript-checked element access
template<class T>
inline T& Foam::UList<T>::operator[](const label i)
{
#   ifdef FULLDEBUG
    checkIndex(i);
#   endif
    return v_[i];
}


// Return subscript-checked const element access
template<class T>
inline const T& Foam::UList<T>::operator[](const label i) const
{
#   ifdef FULLDEBUG
    checkIndex(i);
#   endif
    return v_[i];
}


// Allow cast to a const List<T>&
template<class T>
inline Foam::UList<T>::operator const Foam::List<T>&() const
{
    return *reinterpret_cast<const List<T>*>(this);
}


// * * * * * * * * * * * * * * STL Member Functions  * * * * * * * * * * * * //

template<class T>
inline typename Foam::UList<T>::iterator
Foam::UList<T>::begin()
{
    return v_;
}

template<class T>
inline typename Foam::UList<T>::const_iterator
Foam::UList<T>::begin() const
{
    return v_;
}

template<class T>
inline typename Foam::UList<T>::iterator
Foam::UList<T>::end()
{
    return &v_[size_];
}

template<class T>
inline typename Foam::UList<T>::const_iterator
Foam::UList<T>::end() const
{
    return &v_[size_];
}

template<class T>
inline typename Foam::UList<T>::iterator
Foam::UList<T>::rbegin()
{
    return &v_[size_-1];
}

template<class T>
inline typename Foam::UList<T>::const_iterator
Foam::UList<T>::rbegin() const
{
    return &v_[size_-1];
}

template<class T>
inline typename Foam::UList<T>::iterator
Foam::UList<T>::rend()
{
    return &v_[-1];
}

template<class T>
inline typename Foam::UList<T>::const_iterator
Foam::UList<T>::rend() const
{
    return &v_[-1];
}

template<class T>
inline Foam::label Foam::UList<T>::size() const
{
    return size_;
}


template<class T>
inline Foam::label Foam::UList<T>::max_size() const
{
    return labelMax;
}


template<class T>
inline bool Foam::UList<T>::empty() const
{
    return (size_ == 0);
}


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

template<class T>
inline void Foam::reverse(UList<T>& ul, const label n)
{
    for (int i=0; i<n/2; i++)
    {
        Swap(ul[i], ul[n-1-i]);
    }
}

template<class T>
inline void Foam::reverse(UList<T>& ul)
{
    reverse(ul, ul.size());
}


// ************************************************************************* //
