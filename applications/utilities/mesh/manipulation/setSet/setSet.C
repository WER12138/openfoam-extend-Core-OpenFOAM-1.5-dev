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

Description
    Manipulate a cell/face/point set interactively.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "polyMesh.H"
#include "globalMeshData.H"
#include "topoSetSource.H"
#include "topoSet.H"
#include "IStringStream.H"
#include "topoSet.H"
#include "cellSet.H"
#include "faceSet.H"
#include "OFstream.H"
#include "IFstream.H"
#include "demandDrivenData.H"
#include "writePatch.H"
#include "writePointSet.H"
#include "IOobjectList.H"

#include <stdio.h>


#if READLINE != 0
#include <readline/readline.h>
#include <readline/history.h>
#endif

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#if READLINE != 0
static const char* historyFile = ".setSet";
#endif

Istream& selectStream(Istream* is0Ptr, Istream* is1Ptr)
{
    if (is0Ptr)
    {
        return *is0Ptr;
    }
    else if (is1Ptr)
    {
        return *is1Ptr;
    }
    else
    {
        FatalErrorIn("selectStream(Istream*, Istream*)")
            << "No valid stream opened" << abort(FatalError);

        return *is0Ptr;
    }
}

// Copy set
void backup
(
    const polyMesh& mesh,
    const word& fromName,
    const topoSet& fromSet,
    const word& toName
)
{
    if (fromSet.size() > 0)
    {
        Pout<< "    Backing up " << fromName << " into " << toName << endl;

        topoSet backupSet(mesh, toName, fromSet);

        backupSet.write();
    }
}


// Read and copy set
void backup
(
    const polyMesh& mesh,
    const word& fromName,
    const word& toName
)
{
    topoSet fromSet(mesh, fromName, IOobject::READ_IF_PRESENT);

    backup(mesh, fromName, fromSet, toName);
}


// Write set to VTK readable files
void writeVTK
(
    const polyMesh& mesh,
    const topoSet& currentSet,
    const fileName& vtkName
)
{
    if (typeid(currentSet) == typeid(faceSet))
    {
        // Faces of set with OpenFOAM faceID as value

        faceList setFaces(currentSet.size());
        labelList faceValues(currentSet.size());
        label setFaceI = 0;

        forAllConstIter(topoSet, currentSet, iter)
        {
            setFaces[setFaceI] = mesh.faces()[iter.key()];
            faceValues[setFaceI] = iter.key();
            setFaceI++;
        }

        primitiveFacePatch fp(setFaces, mesh.points());

        writePatch
        (
            true,
            currentSet.name(),
            fp,
            "faceID",
            faceValues,
            mesh.time().path()/vtkName
        );
    }
    else if (typeid(currentSet) == typeid(cellSet))
    {
        // External faces of cellset with OpenFOAM cellID as value

        Map<label> cellFaces(currentSet.size());

        forAllConstIter(cellSet, currentSet, iter)
        {
            label cellI = iter.key();

            const cell& cFaces = mesh.cells()[cellI];

            forAll(cFaces, i)
            {
                label faceI = cFaces[i];

                if (mesh.isInternalFace(faceI))
                {
                    label otherCellI = mesh.faceOwner()[faceI];

                    if (otherCellI == cellI)
                    {
                        otherCellI = mesh.faceNeighbour()[faceI];
                    }

                    if (!currentSet.found(otherCellI))
                    {
                        cellFaces.insert(faceI, cellI);
                    }
                }
                else
                {
                    cellFaces.insert(faceI, cellI);
                }
            }
        }

        faceList setFaces(cellFaces.size());
        labelList faceValues(cellFaces.size());
        label setFaceI = 0;

        forAllConstIter(Map<label>, cellFaces, iter)
        {
            setFaces[setFaceI] = mesh.faces()[iter.key()];
            faceValues[setFaceI] = iter();              // Cell ID
            setFaceI++;
        }

        primitiveFacePatch fp(setFaces, mesh.points());

        writePatch
        (
            true,
            currentSet.name(),
            fp,
            "cellID",
            faceValues,
            mesh.time().path()/vtkName
        );
    }
    else if (typeid(currentSet) == typeid(pointSet))
    {
        writePointSet
        (
            true,
            mesh,
            currentSet,
            mesh.time().path()/vtkName
        );
    }
    else
    {
        WarningIn
        (
            "void writeVTK"
            "(const polyMesh& mesh, const topoSet& currentSet,"
            "const fileName& vtkName)"
        )   << "Don't know how to handle set of type " << currentSet.type()
            << endl;
    }
}


void printHelp(Ostream& os)
{
    os  << "Please type 'help', 'list', 'quit', 'time ddd'"
        << " or a set command after prompt." << endl
        << "'list' will show all current cell/face/point sets." << endl
        << "'time ddd' will change the current time." << endl
        << endl
        << "A set command should be of the following form" << endl
        << endl
        << "    cellSet|faceSet|pointSet <setName> <action> <source>"
        << endl << endl
        << "The <action> is one of" << endl
        << "    list            - prints the contents of the set" << endl
        << "    clear           - clears the set" << endl
        << "    invert          - inverts the set" << endl
        << "    new <source>    - sets to set to the source set" << endl
        << "    add <source>    - adds all elements from the source set" << endl
        << "    delete <source> - deletes      ,," << endl
        << "    subset <source> - combines current set with the source set"
        << endl
        << endl
        << "The sources come in various forms. Type a wrong source"
        << " to see all the types available." << endl
        << endl
        << "Example: pick up all cells connected by point or face to patch"
        << " movingWall" << endl
        << endl
        << "Pick up all faces of patch:" << endl
        << "    faceSet f0 new patchToFace movingWall" << endl
        << "Add faces 0,1,2:" << endl
        << "    faceSet f0 add labelToFace (0 1 2)" << endl
        << "Pick up all points used by faces in faceSet f0:" << endl
        << "    pointSet p0 new faceToPoint f0 all" << endl
        << "Pick up cell which has any face in f0:" << endl
        << "    cellSet c0 new faceToCell f0 any" << endl
        << "Add cells which have any point in p0:" << endl
        << "    cellSet c0 add pointToCell p0 any" << endl
        << "List set:" << endl
        << "    cellSet c0 list" << endl
        << endl;
}


void printAllSets(const polyMesh& mesh, Ostream& os)
{
    IOobjectList objects
    (
        mesh,
        mesh.pointsInstance(),
        polyMesh::meshSubDir/"sets"
    );
    IOobjectList cellSets(objects.lookupClass(cellSet::typeName));
    if (cellSets.size() > 0)
    {
        os  << "cellSets:" << endl;
        forAllConstIter(IOobjectList, cellSets, iter)
        {
            cellSet set(*iter());
            os  << '\t' << set.name() << "\tsize:" << set.size() << endl;
        }
    }
    IOobjectList faceSets(objects.lookupClass(faceSet::typeName));
    if (faceSets.size() > 0)
    {
        os  << "faceSets:" << endl;
        forAllConstIter(IOobjectList, faceSets, iter)
        {
            faceSet set(*iter());
            os  << '\t' << set.name() << "\tsize:" << set.size() << endl;
        }
    }
    IOobjectList pointSets(objects.lookupClass(pointSet::typeName));
    if (pointSets.size() > 0)
    {
        os  << "pointSets:" << endl;
        forAllConstIter(IOobjectList, pointSets, iter)
        {
            pointSet set(*iter());
            os  << '\t' << set.name() << "\tsize:" << set.size() << endl;
        }
    }
    os  << endl;
}


// Read command and execute. Return true if ok, false otherwise.
bool doCommand
(
    const polyMesh& mesh,
    const word& setType,
    const word& setName,
    const word& actionName,
    const bool writeVTKFile,
    Istream& is
)
{
    // Get some size estimate for set.
    const globalMeshData& parData = mesh.globalData();

    label typSize =
        max
        (
            parData.nTotalCells(),
            max
            (
                parData.nTotalFaces(),
                parData.nTotalPoints()
            )
        )
      / (10*Pstream::nProcs());


    bool ok = true;

    // Set to work on
    autoPtr<topoSet> currentSetPtr(NULL);

    word sourceType;

    try
    {
        topoSetSource::setAction action =
            topoSetSource::toAction(actionName);


        IOobject::readOption r;

        if
        (
            (action == topoSetSource::NEW)
         || (action == topoSetSource::CLEAR)
        )
        {
            r = IOobject::NO_READ;

            backup(mesh, setName, setName + "_old");

            currentSetPtr = topoSet::New(setType, mesh, setName, typSize);
        }
        else
        {
            r = IOobject::MUST_READ;

            currentSetPtr = topoSet::New(setType, mesh, setName, r);

            topoSet& currentSet = currentSetPtr();

            // Presize it according to current mesh data.
            currentSet.resize(max(currentSet.size(), typSize));
        }

        if (!currentSetPtr.valid())
        {
            Pout<< "    Cannot construct/load set "
                << topoSet::localPath(mesh, setName) << endl;

            ok = false;
        }
        else
        {
            topoSet& currentSet = currentSetPtr();

            Pout<< "    Set:" << currentSet.name()
                << "  Size:" << currentSet.size()
                << "  Action:" << actionName
                << endl;

            if ((r == IOobject::MUST_READ) && (action != topoSetSource::LIST))
            {
                // currentSet has been read so can make copy.
                backup(mesh, setName, currentSet, setName + "_old");
            }

            switch (action)
            {
                case topoSetSource::CLEAR:
                {
                    // Already handled above by not reading
                    break;
                }
                case topoSetSource::INVERT:
                {
                    currentSet.invert(currentSet.maxSize(mesh));
                    break;
                }
                case topoSetSource::LIST:
                {
                    currentSet.writeDebug(Pout, mesh, 100);
                    Pout<< endl;
                    break;
                }
                case topoSetSource::SUBSET:
                {
                    if (is >> sourceType)
                    {
                        autoPtr<topoSetSource> setSource
                        (
                            topoSetSource::New
                            (
                                sourceType,
                                mesh,
                                is
                            )
                        );

                        // Backup current set.
                        topoSet oldSet
                        (
                            mesh,
                            currentSet.name() + "_old2",
                            currentSet
                        );

                        currentSet.clear();
                        currentSet.resize(oldSet.size());
                        setSource().applyToSet(topoSetSource::NEW, currentSet);

                        // Combine new value of currentSet with old one.
                        currentSet.subset(oldSet);
                    }
                    break;
                }
                default:
                {
                    if (is >> sourceType)
                    {
                        autoPtr<topoSetSource> setSource
                        (
                            topoSetSource::New
                            (
                                sourceType,
                                mesh,
                                is
                            )
                        );

                        setSource().applyToSet(action, currentSet);
                    }
                }
            }


            if (action != topoSetSource::LIST)
            {
                // Set will have been modified.

                // Synchronize for coupled patches.
                currentSet.sync(mesh);

                // Write
                if (writeVTKFile)
                {
                    mkDir(mesh.time().path()/"VTK"/currentSet.name());

                    fileName vtkName
                    (
                        "VTK"/currentSet.name()/currentSet.name()
                      + "_"
                      + name(mesh.time().timeIndex())
                      + ".vtk"
                    );

                    Pout<< "    Writing " << currentSet.name()
                        << " (size " << currentSet.size() << ") to "
                        << currentSet.instance()/currentSet.local()
                           /currentSet.name()
                        << " and to vtk file " << vtkName << endl << endl;

                    currentSet.write();

                    writeVTK(mesh, currentSet, vtkName);
                }
                else
                {
                    Pout<< "    Writing " << currentSet.name()
                        << " (size " << currentSet.size() << ") to "
                        << currentSet.instance()/currentSet.local()
                           /currentSet.name() << endl << endl;

                    currentSet.write();
                }
            }
        }
    }
    catch (Foam::IOerror& fIOErr)
    {
        ok = false;

        Pout<< fIOErr.message().c_str() << endl;

        if (sourceType.size() != 0)
        {
            Pout<< topoSetSource::usage(sourceType).c_str();
        }
    }
    catch (Foam::error& fErr)
    {
        ok = false;

        Pout<< fErr.message().c_str() << endl;

        if (sourceType.size() != 0)
        {
            Pout<< topoSetSource::usage(sourceType).c_str();
        }
    }

    return ok;
}


// Status returned from parsing the first token of the line
enum commandStatus
{
    QUIT,           // quit program
    INVALID,        // token is not a valid set manipulation command
    VALID           // ,,    is a valid     ,,
};


void printMesh(const Time& runTime, const polyMesh& mesh)
{
    Pout<< "Time:" << runTime.timeName()
        << "  cells:" << mesh.nCells()
        << "  faces:" << mesh.nFaces()
        << "  points:" << mesh.nPoints()
        << "  patches:" << mesh.boundaryMesh().size() << nl;
}



commandStatus parseType
(
    Time& runTime,
    polyMesh& mesh,
    const word& setType,
    IStringStream& is
)
{
    if (setType.size() == 0)
    {
        Pout<< "Type 'help' for usage information" << endl;

        return INVALID;
    }
    else if (setType == "help")
    {
        printHelp(Pout);

        return INVALID;
    }
    else if (setType == "list")
    {
        printAllSets(mesh, Pout);

        return INVALID;
    }
    else if (setType == "time")
    {
        scalar time = readScalar(is);

        instantList Times = runTime.times();

        int nearestIndex = -1;
        scalar nearestDiff = Foam::GREAT;

        forAll(Times, timeIndex)
        {
            if (Times[timeIndex].name() == "constant") continue;

            scalar diff = fabs(Times[timeIndex].value() - time);
            if (diff < nearestDiff)
            {
                nearestDiff = diff;
                nearestIndex = timeIndex;
            }
        }

        Pout<< "Changing time from " << runTime.timeName()
            << " to " << Times[nearestIndex].name()
            << endl;

        runTime.setTime(Times[nearestIndex], nearestIndex);
        polyMesh::readUpdateState stat = mesh.readUpdate();

        switch(stat)
        {
            case polyMesh::UNCHANGED:
            {
                Pout<< "    mesh not changed." << endl;
                break;
            }
            case polyMesh::POINTS_MOVED:
            {
                Pout<< "    points moved; topology unchanged." << endl;
                break;
            }
            case polyMesh::TOPO_CHANGE:
            {
                Pout<< "    topology changed; patches unchanged." << nl
                    << "    ";
                printMesh(runTime, mesh);
                break;
            }
            case polyMesh::TOPO_PATCH_CHANGE:
            {
                Pout<< "    topology changed and patches changed." << nl
                    << "    ";
                printMesh(runTime, mesh);

                break;
            }
            default:
            {
                FatalErrorIn("parseType") << "Illegal mesh update state "
                    << stat  << abort(FatalError);
                break;
            }
        }

        return INVALID;
    }
    else if (setType == "quit")
    {
        Pout<< "Quitting ..." << endl;

        return QUIT;
    }
    else if
    (
        setType == "cellSet"
     || setType == "faceSet"
     || setType == "pointSet"
    )
    {
        return VALID;
    }
    else
    {
        SeriousErrorIn
        (
            "commandStatus parseType(Time&, polyMesh&, const word&"
            ", IStringStream&)"
        )   << "Illegal command " << setType << endl
            << "Should be one of 'help', 'list', 'time' or a set type :"
            << " 'cellSet', 'faceSet', 'pointSet'"
            << endl;

        return INVALID;
    }
}


commandStatus parseAction(const word& actionName)
{
    commandStatus stat = INVALID;

    if (actionName.size() != 0)
    {
        try
        {
            (void)topoSetSource::toAction(actionName);

            stat = VALID;
        }
        catch (Foam::IOerror& fIOErr)
        {
            stat = INVALID;
        }
        catch (Foam::error& fErr)
        {
            stat = INVALID;
        }
    }
    return stat;
}


// Main program:

int main(int argc, char *argv[])
{
    argList::validOptions.insert("noVTK", "");
    argList::validOptions.insert("batch", "file");
#   include "addTimeOptions.H"

#   include "setRootCase.H"
#   include "createTime.H"

    bool writeVTK = !args.options().found("noVTK");

    // Get times list
    instantList Times = runTime.times();

#   include "checkTimeOptions.H"

    runTime.setTime(Times[startTime], startTime);

#   include "createPolyMesh.H"

    // Print some mesh info
    printMesh(runTime, mesh);


    std::ifstream* fileStreamPtr(NULL);

    if (args.options().found("batch"))
    {
        fileName batchFile(args.options()["batch"]);

        Pout<< "Reading commands from file " << batchFile << endl;

        if (!exists(batchFile))
        {
            FatalErrorIn(args.executable())
                << "Cannot open file " << batchFile << exit(FatalError);
        }

        fileStreamPtr = new std::ifstream(batchFile.c_str());
    }
#if READLINE != 0
    else if (!read_history(historyFile))
    {
        Info<< "Successfully read history from " << historyFile << endl;
    }
#endif


    Pout<< "Please type 'help', 'quit' or a set command after prompt." << endl;

    bool ok = true;

    FatalError.throwExceptions();
    FatalIOError.throwExceptions();

    do
    {
        string rawLine;

        // Type: cellSet, faceSet, pointSet
        word setType;
        // Name of destination set.
        word setName;
        // Action (new, invert etc.)
        word actionName;

        commandStatus stat = INVALID;

        if (fileStreamPtr)
        {
            if (!fileStreamPtr->good())
            {
                Pout<< "End of batch file" << endl;
                break;
            }

            std::getline(*fileStreamPtr, rawLine);

            if (rawLine.size() > 0)
            {
                Pout<< "Doing:" << rawLine << endl;
            }
        }
        else
        {
#           if READLINE != 0
            {
                char* linePtr = readline("readline>");

                rawLine = string(linePtr);

                if (*linePtr)
                {
                    add_history(linePtr);
                    write_history(historyFile);
                }

                free(linePtr);   // readline uses malloc, not new.
            }
#           else
            {
                Pout<< "Command>" << flush;
                std::getline(std::cin, rawLine);
            }
#           endif
        }

        if (rawLine.size() == 0 || rawLine[0] == '#')
        {
            continue;
        }

        IStringStream is(rawLine + ' ');

        // Type: cellSet, faceSet, pointSet
        is >> setType;

        stat = parseType(runTime, mesh, setType, is);

        if (stat == VALID)
        {
            if (is >> setName)
            {
                if (is >> actionName)
                {
                    stat = parseAction(actionName);
                }
            }
        }

        ok = true;

        if (stat == QUIT)
        {
            break;
        }
        else if (stat == VALID)
        {
            ok = doCommand(mesh, setType, setName, actionName, writeVTK, is);
        }

    } while (ok);


    if (fileStreamPtr)
    {
        delete fileStreamPtr;
    }

    Pout << nl << "End" << endl;

    return 0;
}


// ************************************************************************* //
