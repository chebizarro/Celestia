// cmdparser.cpp
//
// Parse Celestia command files and turn them into CommandSequences.
//
// Copyright (C) 2001 Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "celestia.h"

#include "astro.h"
#include "cmdparser.h"
#include "glcontext.h"
#include <celutil/util.h>
#include <celutil/debug.h>
#include <celmath/mathlib.h>
#include <celengine/astro.h>
#include <celestia/celx.h>
#include <celestia/celx_internal.h>
#include <celengine/render.h>
#include <algorithm>
#include <Eigen/Geometry>

// Older gcc versions used <strstream> instead of <sstream>.
// This has been corrected in GCC 3.2, but name clashing must
// be avoided
#ifdef __GNUC__
#undef min
#undef max
#endif
#include <sstream>

using namespace std;


static uint64_t parseRenderFlags(string /*s*/);
static int parseLabelFlags(string /*s*/);
static int parseOrbitFlags(string /*s*/);
static int parseConstellations(CommandConstellations* cmd, string s, int act);
int parseConstellationColor(CommandConstellationColor* cmd, string s, Eigen::Vector3d *col, int act);

CommandParser::CommandParser(istream& in)
{
    tokenizer = new Tokenizer(&in);
    parser = new Parser(tokenizer);
}

CommandParser::CommandParser(Tokenizer& tok)
{
    tokenizer = &tok;
    parser = new Parser(tokenizer);
}

CommandParser::~CommandParser()
{
    delete parser;
    delete tokenizer;
}


CommandSequence* CommandParser::parse()
{
    CommandSequence* seq = new CommandSequence();

    if (tokenizer->nextToken() != Tokenizer::TokenBeginGroup)
    {
        error("'{' expected at start of script.");
        delete seq;
        return nullptr;
    }

    Tokenizer::TokenType ttype = tokenizer->nextToken();
    while (ttype != Tokenizer::TokenEnd && ttype != Tokenizer::TokenEndGroup)
    {
        tokenizer->pushBack();
        Command* cmd = parseCommand();
        if (cmd == nullptr)
        {
            for_each(seq->begin(), seq->end(), deleteFunc<Command*>());;
            delete seq;
            return nullptr;
        }
        else
        {
            seq->push_back(cmd);
        }

        ttype = tokenizer->nextToken();
    }

    if (ttype != Tokenizer::TokenEndGroup)
    {
        error("Missing '}' at end of script.");
        for_each(seq->begin(), seq->end(), deleteFunc<Command*>());;
        delete seq;
        return nullptr;
    }

    return seq;
}


const vector<string>* CommandParser::getErrors() const
{
    return &errorList;
}


void CommandParser::error(const string errMsg)
{
    errorList.push_back(errMsg);
}


static ObserverFrame::CoordinateSystem parseCoordinateSystem(const string& name)
{
    // 'geographic' is a deprecated name for the bodyfixed coordinate system,
    // maintained here for compatibility with older scripts.
    if (compareIgnoringCase(name, "observer") == 0)
        return ObserverFrame::ObserverLocal;
    else if (compareIgnoringCase(name, "bodyfixed") == 0)
        return ObserverFrame::BodyFixed;
    else if (compareIgnoringCase(name, "geographic") == 0)
        return ObserverFrame::BodyFixed;
    else if (compareIgnoringCase(name, "equatorial") == 0)
        return ObserverFrame::Equatorial;
    else if (compareIgnoringCase(name, "ecliptical") == 0)
        return ObserverFrame::Ecliptical;
    else if (compareIgnoringCase(name, "universal") == 0)
        return ObserverFrame::Universal;
    else if (compareIgnoringCase(name, "lock") == 0)
        return ObserverFrame::PhaseLock;
    else if (compareIgnoringCase(name, "chase") == 0)
        return ObserverFrame::Chase;
    else
        return ObserverFrame::ObserverLocal;
}


Command* CommandParser::parseCommand()
{
    if (tokenizer->nextToken() != Tokenizer::TokenName)
    {
        error("Invalid command name");
        return nullptr;
    }

    string commandName = tokenizer->getStringValue();

    Value* paramListValue = parser->readValue();
    if (paramListValue == nullptr || paramListValue->getType() != Value::HashType)
    {
        error("Bad parameter list");
        return nullptr;
    }

    Hash* paramList = paramListValue->getHash();
    Command* cmd = nullptr;

    if (commandName == "wait")
    {
        double duration = 1.0;
        paramList->getNumber("duration", duration);
        cmd = new CommandWait(duration);
    }
    else if (commandName == "set")
    {
        double value = 0.0;
        string name;
        paramList->getString("name", name);
        if (!paramList->getNumber("value", value))
        {
            // Some values may be specified via strings
            string valstr;
            if (paramList->getString("value", valstr))
            {
                if (compareIgnoringCase(valstr, "fuzzypoints") == 0)
                    value = (double) Renderer::FuzzyPointStars;
                else if (compareIgnoringCase(valstr, "points") == 0)
                    value = (double) Renderer::PointStars;
                else if (compareIgnoringCase(valstr, "scaleddiscs") == 0)
                    value = (double) Renderer::ScaledDiscStars;
            }
        }

        cmd = new CommandSet(name, value);
    }
    else if (commandName == "select")
    {
        string object;
        paramList->getString("object", object);
        cmd = new CommandSelect(object);
    }
    else if (commandName == "setframe")
    {
        string refName;
        paramList->getString("ref", refName);
        string targetName;
        paramList->getString("target", targetName);
        string coordSysName;
        ObserverFrame::CoordinateSystem coordSys = ObserverFrame::Universal;
        if (paramList->getString("coordsys", coordSysName))
            coordSys = parseCoordinateSystem(coordSysName);

        cmd = new CommandSetFrame(coordSys, refName, targetName);
    }
    else if (commandName == "setsurface")
    {
        string name;
        paramList->getString("name", name);
        cmd = new CommandSetSurface(name);
    }
    else if (commandName == "goto")
    {
        double t = 1.0;
        paramList->getNumber("time", t);
        double distance = 5.0;
        paramList->getNumber("distance", distance);

        ObserverFrame::CoordinateSystem upFrame = ObserverFrame::ObserverLocal;
        string frameString;
        if (paramList->getString("upframe", frameString))
            upFrame = parseCoordinateSystem(frameString);

#ifdef __CELVEC__
        Vec3d up(0, 1, 0);
#else
        Eigen::Vector3d up(Eigen::Vector3d::UnitY());
#endif
        paramList->getVector("up", up);

        cmd = new CommandGoto(t,
                              distance,
#ifdef __CELVEC__
                              Vec3f((float) up.x, (float) up.y, (float) up.z),
#else
                              up.cast<float>(),
#endif
                              upFrame);
    }
    else if (commandName == "gotolonglat")
    {
        double t = 1.0;
        paramList->getNumber("time", t);
        double distance = 5.0;
        paramList->getNumber("distance", distance);
#ifdef __CELVEC__
        Vec3d up(0, 1, 0);
#else
        Eigen::Vector3d up(Eigen::Vector3d::UnitY());
#endif
        paramList->getVector("up", up);
        double longitude;
        paramList->getNumber("longitude", longitude);
        double latitude;
        paramList->getNumber("latitude", latitude);

        cmd = new CommandGotoLongLat(t,
                                     distance,
                                     (float) degToRad(longitude),
                                     (float) degToRad(latitude),
#ifdef __CELVEC__
                                     Vec3f((float) up.x, (float) up.y, (float) up.z));
#else
                                     up.cast<float>());
#endif
    }
    else if (commandName == "gotoloc")
    {
        double t = 1.0;
        paramList->getNumber("time", t);
#ifdef __CELVEC__
        Vec3d pos(0, 1, 0);
#else
        Eigen::Vector3d pos(Eigen::Vector3d::UnitY());
#endif
        if (paramList->getVector("position", pos))
        {
            pos = pos * astro::kilometersToMicroLightYears(1.0);
            double xrot = 0.0;
            paramList->getNumber("xrot", xrot);
            double yrot = 0.0;
            paramList->getNumber("yrot", yrot);
            double zrot = 0.0;
            paramList->getNumber("zrot", zrot);
            zrot = degToRad(zrot);
#ifdef __CELVEC__
            Quatf rotation = Quatf::xrotation((float) degToRad(xrot)) *
                Quatf::yrotation((float) degToRad(yrot)) *
                Quatf::zrotation((float) degToRad(zrot));
            cmd = new CommandGotoLocation(t, Point3d(0.0, 0.0, 0.0) + pos,
                                          rotation);
#else
            auto rotation = Eigen::Quaterniond(Eigen::AngleAxisd(degToRad(xrot), Eigen::Vector3d::UnitX()) *
                                               Eigen::AngleAxisd(degToRad(yrot), Eigen::Vector3d::UnitY()) *
                                               Eigen::AngleAxisd(degToRad(zrot), Eigen::Vector3d::UnitZ()));
            cmd = new CommandGotoLocation(t, pos, rotation);
#endif
        }
        else
        {
            std::string x, y, z;
            paramList->getString("x", x);
            paramList->getString("y", y);
            paramList->getString("z", z);
            double ow, ox, oy, oz;
            paramList->getNumber("ow", ow);
            paramList->getNumber("ox", ox);
            paramList->getNumber("oy", oy);
            paramList->getNumber("oz", oz);
#ifdef __CELVEC__
            Quatf orientation((float)ow, (float)ox, (float)oy, (float)oz);
            cmd = new CommandGotoLocation(t, Point3d((double)BigFix(x), (double)BigFix(y), (double)BigFix(z)),
                                          orientation);
#else
            Eigen::Quaterniond orientation(ow, ox, oy, oz);
            cmd = new CommandGotoLocation(t, Eigen::Vector3d((double)BigFix(x), (double)BigFix(y), (double)BigFix(z)),
                                          orientation);
#endif
        }
    }
    else if (commandName == "seturl")
    {
        std::string url;
        paramList->getString("url", url);
        cmd = new CommandSetUrl(url);
    }
    else if (commandName == "center")
    {
        double t = 1.0;
        paramList->getNumber("time", t);
        cmd = new CommandCenter(t);
    }
    else if (commandName == "follow")
    {
        cmd = new CommandFollow();
    }
    else if (commandName == "synchronous")
    {
        cmd = new CommandSynchronous();
    }
    else if (commandName == "lock")
    {
        cmd = new CommandLock();
    }
    else if (commandName == "chase")
    {
        cmd = new CommandChase();
    }
    else if (commandName == "track")
    {
        cmd = new CommandTrack();
    }
    else if (commandName == "cancel")
    {
        cmd = new CommandCancel();
    }
    else if (commandName == "exit")
    {
        cmd = new CommandExit();
    }
    else if (commandName == "print")
    {
        string text;
        string origin;
        int horig = -1;
        int vorig = -1;
        double hoff = 0;
        double voff = 0;
        double duration = 1.0e9;
        paramList->getString("text", text);
        paramList->getString("origin", origin);
        paramList->getNumber("duration", duration);
        paramList->getNumber("row", voff);
        paramList->getNumber("column", hoff);
        if (compareIgnoringCase(origin, "left") == 0)
        {
            horig = -1;
            vorig = 0;
        }
        else if (compareIgnoringCase(origin, "right") == 0)
        {
            horig = 1;
            vorig = 0;
        }
        else if (compareIgnoringCase(origin, "center") == 0)
        {
            horig = 0;
            vorig = 0;
        }
        else if (compareIgnoringCase(origin, "left") == 0)
        {
            horig = -1;
            vorig = 0;
        }
        else if (compareIgnoringCase(origin, "top") == 0)
        {
            horig = 0;
            vorig = 1;
        }
        else if (compareIgnoringCase(origin, "bottom") == 0)
        {
            horig = 0;
            vorig = -1;
        }
        else if (compareIgnoringCase(origin, "topright") == 0)
        {
            horig = 1;
            vorig = 1;
        }
        else if (compareIgnoringCase(origin, "topleft") == 0)
        {
            horig = -1;
            vorig = 1;
        }
        else if (compareIgnoringCase(origin, "bottomleft") == 0)
        {
            horig = -1;
            vorig = -1;
        }
        else if (compareIgnoringCase(origin, "bottomright") == 0)
        {
            horig = 1;
            vorig = -1;
        }

        cmd = new CommandPrint(text,
                               horig, vorig, (int) hoff, (int) -voff,
                               duration);
    }
    else if (commandName == "cls")
    {
        cmd = new CommandClearScreen();
    }
    else if (commandName == "time")
    {
        double jd = 2451545;
        if (!paramList->getNumber("jd", jd))
        {
            std::string utc;
            paramList->getString("utc", utc);
            astro::Date date = astro::Date(0.0);
            sscanf(utc.c_str(), "%d-%d-%dT%d:%d:%lf",
                &date.year, &date.month, &date.day,
                &date.hour, &date.minute, &date.seconds);
            jd = (double)date;
        }
        cmd = new CommandSetTime(jd);
    }
    else if (commandName == "timerate")
    {
        double rate = 1.0;
        paramList->getNumber("rate", rate);
        cmd = new CommandSetTimeRate(rate);
    }
    else if (commandName == "changedistance")
    {
        double rate = 0.0;
        double duration = 1.0;
        paramList->getNumber("rate", rate);
        paramList->getNumber("duration", duration);
        cmd = new CommandChangeDistance(duration, rate);
    }
    else if (commandName == "orbit")
    {
        double rate = 0.0;
        double duration = 1.0;
#ifdef __CELVEC__
        Vec3d axis;
#else
        Eigen::Vector3d axis(Eigen::Vector3d::Zero());
#endif
        paramList->getNumber("duration", duration);
        paramList->getNumber("rate", rate);
        paramList->getVector("axis", axis);
        cmd = new CommandOrbit(duration,
#ifdef __CELVEC__
                               Vec3f((float) axis.x, (float) axis.y, (float) axis.z),
#else
                               axis.cast<float>(),
#endif
                               (float) degToRad(rate));
    }
    else if (commandName == "rotate")
    {
        double rate = 0.0;
        double duration = 1.0;
#ifdef __CELVEC__
        Vec3d axis;
#else
        Eigen::Vector3d axis(Eigen::Vector3d::Zero());
#endif
        paramList->getNumber("duration", duration);
        paramList->getNumber("rate", rate);
        paramList->getVector("axis", axis);
        cmd = new CommandRotate(duration,
#ifdef __CELVEC__
                                Vec3f((float) axis.x, (float) axis.y, (float) axis.z),
#else
                                axis.cast<float>(),
#endif
                                (float) degToRad(rate));
    }
    else if (commandName == "move")
    {
#ifdef __CELVEC__
        Vec3d velocity;
#else
        Eigen::Vector3d velocity(Eigen::Vector3d::Zero());
#endif
        double duration;
        paramList->getNumber("duration", duration);
        paramList->getVector("velocity", velocity);
        cmd = new CommandMove(duration, velocity * astro::kilometersToMicroLightYears(1.0));
    }
    else if (commandName == "setposition")
    {
        // Base position in light years, offset in kilometers
        Eigen::Vector3d base, offset;
        if (paramList->getVector("base", base))
        {
            paramList->getVector("offset", offset);
            Eigen::Vector3f basef = base.cast<float>();
            UniversalCoord basePosition = UniversalCoord::CreateLy(basef.cast<double>());
            cmd = new CommandSetPosition(basePosition.offsetKm(offset));
#ifdef CELVEC
            cmd = new CommandSetPosition(astro::universalPosition(Point3d(offset.x, offset.y, offset.z),
                                                                  Point3f((float) base.x, (float) base.y, (float) base.z)));
#endif
        }
        else
        {
            std::string x, y, z;
            paramList->getString("x", x);
            paramList->getString("y", y);
            paramList->getString("z", z);
            cmd = new CommandSetPosition(UniversalCoord(BigFix(x), BigFix(y), BigFix(z)));
        }
    }
    else if (commandName == "setorientation")
    {
#ifdef __CELVEC__
        Vec3d axis;
#else
        Eigen::Vector3d axis(Eigen::Vector3d::Zero());
#endif
        double angle;
        if (paramList->getNumber("angle", angle))
        {
            paramList->getVector("axis", axis);
#ifdef __CELVEC__
            cmd = new CommandSetOrientation(Vec3f((float) axis.x, (float) axis.y, (float) axis.z),
                                            (float) degToRad(angle));
#else
            auto orientation = Eigen::Quaternionf(Eigen::AngleAxisf((float) degToRad(angle),
                                                                    axis.cast<float>().normalized()));
            cmd = new CommandSetOrientation(orientation);
#endif
        }
        else
        {
            double ow, ox, oy, oz;
            paramList->getNumber("ow", ow);
            paramList->getNumber("ox", ox);
            paramList->getNumber("oy", oy);
            paramList->getNumber("oz", oz);
#ifdef __CELVEC__
            Quatf orientation((float)ow, (float)ox, (float)oy, (float)oz);
            Vec3f axis;
            float angle;
            orientation.getAxisAngle(axis, angle);
            cmd = new CommandSetOrientation(axis, angle);
#else
            Eigen::Quaternionf orientation(ow, ox, oy, oz);
            cmd = new CommandSetOrientation(orientation);
#endif
        }
    }
    else if (commandName == "lookback")
    {
        cmd = new CommandLookBack();
    }
    else if (commandName == "renderflags")
    {
        uint64_t setFlags = 0;
        uint64_t clearFlags = 0;
        string s;

        if (paramList->getString("set", s))
            setFlags = parseRenderFlags(s);
        if (paramList->getString("clear", s))
            clearFlags = parseRenderFlags(s);

        cmd = new CommandRenderFlags(setFlags, clearFlags);
    }
    else if (commandName == "labels")
    {
        int setFlags = 0;
        int clearFlags = 0;
        string s;

        if (paramList->getString("set", s))
            setFlags = parseLabelFlags(s);
        if (paramList->getString("clear", s))
            clearFlags = parseLabelFlags(s);

        cmd = new CommandLabels(setFlags, clearFlags);
    }
    else if (commandName == "orbitflags")
    {
        int setFlags = 0;
        int clearFlags = 0;
        string s;

        if (paramList->getString("set", s))
            setFlags = parseOrbitFlags(s);
        if (paramList->getString("clear", s))
            clearFlags = parseOrbitFlags(s);

        cmd = new CommandOrbitFlags(setFlags, clearFlags);
    }
    else if (commandName == "constellations")
    {
        string s;
        CommandConstellations *cmdcons= new CommandConstellations();

        if (paramList->getString("set", s))
            parseConstellations(cmdcons, s, 1);
        if (paramList->getString("clear", s))
            parseConstellations(cmdcons, s, 0);
        cmd = cmdcons;
    }

    else if (commandName == "constellationcolor")
    {
        string s;
        CommandConstellationColor *cmdconcol= new CommandConstellationColor();

        Eigen::Vector3d colorv(Eigen::Vector3d::UnitX());
        paramList->getVector("color", colorv);

        if (paramList->getString("set", s))
            parseConstellationColor(cmdconcol, s, &colorv, 1);
        if (paramList->getString("clear", s))
            parseConstellationColor(cmdconcol, s, &colorv, 0);
        cmd = cmdconcol;
    }
    else if (commandName == "setvisibilitylimit")
    {
        double mag = 6.0;
        paramList->getNumber("magnitude", mag);
        cmd = new CommandSetVisibilityLimit(mag);
    }
    else if (commandName == "setfaintestautomag45deg")
    {
        double mag = 8.5;
        paramList->getNumber("magnitude", mag);
        cmd = new CommandSetFaintestAutoMag45deg(mag);
    }
    else if (commandName == "setambientlight")
    {
        double brightness = 0.0;
        paramList->getNumber("brightness", brightness);
        cmd = new CommandSetAmbientLight((float) brightness);
    }
    else if (commandName == "setgalaxylightgain")
    {
        double gain = 0.0;
        paramList->getNumber("gain", gain);
        cmd = new CommandSetGalaxyLightGain((float) gain);
    }
    else if (commandName == "settextureresolution")
    {
        string textureRes;
        unsigned int res = 1;
        paramList->getString("resolution", textureRes);
        if (compareIgnoringCase(textureRes, "low") == 0)
            res = 0;
        else if (compareIgnoringCase(textureRes, "medium") == 0)
            res = 1;
        else if (compareIgnoringCase(textureRes, "high") == 0)
            res = 2;
        cmd = new CommandSetTextureResolution(res);
    }
    else if (commandName == "preloadtex")
    {
        string object;
        paramList->getString("object", object);
        cmd = new CommandPreloadTextures(object);
    }
    else if (commandName == "mark")
    {
        string object;
        paramList->getString("object", object);
        double size = 10.0f;
        paramList->getNumber("size", size);
        Eigen::Vector3d colorv(Eigen::Vector3d::UnitX());
        paramList->getVector("color", colorv);
        double alpha = 0.9f;
        paramList->getNumber("alpha", alpha);
        Color color((float) colorv.x(), (float) colorv.y(), (float) colorv.z(), (float) alpha);

        MarkerRepresentation rep(MarkerRepresentation::Diamond);
        string symbolString;
        if (paramList->getString("symbol", symbolString))
        {
            if (compareIgnoringCase(symbolString, "diamond") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Diamond);
            else if (compareIgnoringCase(symbolString, "triangle") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Triangle);
            else if (compareIgnoringCase(symbolString, "square") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Square);
            else if (compareIgnoringCase(symbolString, "filledsquare") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::FilledSquare);
            else if (compareIgnoringCase(symbolString, "plus") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Plus);
            else if (compareIgnoringCase(symbolString, "x") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::X);
            else if (compareIgnoringCase(symbolString, "leftarrow") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::LeftArrow);
            else if (compareIgnoringCase(symbolString, "rightarrow") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::RightArrow);
            else if (compareIgnoringCase(symbolString, "uparrow") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::UpArrow);
            else if (compareIgnoringCase(symbolString, "downarrow") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::DownArrow);
            else if (compareIgnoringCase(symbolString, "circle") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Circle);
            else if (compareIgnoringCase(symbolString, "disk") == 0)
                rep = MarkerRepresentation(MarkerRepresentation::Disk);
        }

        string label;
        paramList->getString("label", label);
        rep.setSize((float) size);
        rep.setColor(color);
        rep.setLabel(label);

        bool occludable = true;
        paramList->getBoolean("occludable", occludable);

        cmd = new CommandMark(object, rep, occludable);
    }
    else if (commandName == "unmark")
    {
        string object;
        paramList->getString("object", object);
        cmd = new CommandUnmark(object);
    }
    else if (commandName == "unmarkall")
    {
        cmd = new CommandUnmarkAll();
    }
    else if (commandName == "capture")
    {
        string type, filename;
        paramList->getString("type", type);
        paramList->getString("filename", filename);

        cmd = new CommandCapture(type, filename);
    }
    else if (commandName == "renderpath")
    {
#if 0
        GLContext::GLRenderPath glcpath = GLContext::GLPath_GLSL;
        string path;
        paramList->getString("path", path);

        if (compareIgnoringCase(path, "glsl") == 0)
            glcpath = GLContext::GLPath_GLSL;

        cmd = new CommandRenderPath(glcpath);
#else
        cmd = new CommandRenderPath(GLContext::GLPath_GLSL);
#endif
    }
    else if (commandName == "splitview")
    {
        unsigned int view = 1;
        paramList->getNumber("view", view);
        string splitType;
        paramList->getString("type", splitType);
        double splitPos = 0.5;
        paramList->getNumber("position", splitPos);
        cmd = new CommandSplitView(view, splitType, (float) splitPos);
    }
    else if (commandName == "deleteview")
    {
        unsigned int view = 1;
        paramList->getNumber("view", view);
        cmd = new CommandDeleteView(view);
    }
    else if (commandName == "singleview")
    {
        cmd = new CommandSingleView();
    }
    else if (commandName == "setactiveview")
    {
        unsigned int view = 1;
        paramList->getNumber("view", view);
        cmd = new CommandSetActiveView(view);
    }
    else if (commandName == "setradius")
    {
        string object;
        paramList->getString("object", object);
        double radius = 1.0;
        paramList->getNumber("radius", radius);
        cmd = new CommandSetRadius(object, (float) radius);
    }
    else if (commandName == "setlinecolor")
    {
        string item;
        paramList->getString("item", item);
        Eigen::Vector3d colorv(Eigen::Vector3d::UnitX());
        paramList->getVector("color", colorv);
        Color color((float) colorv.x(), (float) colorv.y(), (float) colorv.z());
        cmd = new CommandSetLineColor(item, color);
    }
    else if (commandName == "setlabelcolor")
    {
        string item;
        paramList->getString("item", item);
        Eigen::Vector3d colorv(Eigen::Vector3d::UnitX());
        paramList->getVector("color", colorv);
        Color color((float) colorv.x(), (float) colorv.y(), (float) colorv.z());
        cmd = new CommandSetLabelColor(item, color);
    }
    else if (commandName == "settextcolor")
    {
        Eigen::Vector3d colorv(Eigen::Vector3d::Ones());
        paramList->getVector("color", colorv);
        Color color((float) colorv.x(), (float) colorv.y(), (float) colorv.z());
        cmd = new CommandSetTextColor(color);
    }
    else if(commandName == "play")
    {
        cmd = new CommandWait(0);
    }
    else if (commandName == "overlay")
    {
        float duration;
        float xoffset;
        float yoffset;
        float alpha;
        string filename;
        bool fitscreen;

        if(!paramList->getNumber("duration", duration))
            duration = 3;
        if(!paramList->getNumber("xoffset", xoffset))
            xoffset = 0.0;
        if(!paramList->getNumber("yoffset", yoffset))
            yoffset = 0.0;
        if(!paramList->getNumber("alpha", alpha))
            alpha = 1;
        if(!paramList->getString("filename", filename))
            filename = "";
        if(!paramList->getBoolean("fitscreen", fitscreen))
        {
          int f;
          if(!paramList->getNumber("fitscreen", f))
             fitscreen = false;
          else
            fitscreen = (bool) f;
        }

        cmd = new CommandScriptImage(duration, xoffset, yoffset, alpha, filename, fitscreen);
    }
    else if (commandName == "verbosity")
    {
        int level;

        if(!paramList->getNumber("level", level))
            level = 2;

        cmd = new CommandVerbosity(level);
    }
    else if (commandName == "setwindowbordersvisible")
    {
        bool visible;
        if (!paramList->getBoolean("visible", visible))
             visible = true;

        cmd = new CommandSetWindowBordersVisible(visible);
    }
    else if (commandName == "setringstexture")
    {
        string object, texture, path;
        paramList->getString("object", object);
        paramList->getString("texture", texture);
        paramList->getString("path", path);
        cmd = new CommandSetRingsTexture(object, texture, path);
    }
    else if (commandName == "loadfragment")
    {
        string type, fragment, dir;
        paramList->getString("type", type);
        paramList->getString("fragment", fragment);
        paramList->getString("dir", dir);
        cmd = new CommandLoadFragment(type, fragment, dir);
    }
    else
    {
        error("Unknown command name '" + commandName + "'");
        cmd = nullptr;
    }

    delete paramListValue;

    return cmd;
}


uint64_t parseRenderFlags(string s)
{
    istringstream in(s);

    Tokenizer tokenizer(&in);
    uint64_t flags = 0;

    Tokenizer::TokenType ttype = tokenizer.nextToken();
    while (ttype != Tokenizer::TokenEnd)
    {
        if (ttype == Tokenizer::TokenName)
        {
            string name = tokenizer.getNameValue();

            if (CelxLua::RenderFlagMap.count(name) == 0)
                cerr << "Unknown render flag: " << name << "\n";
            else
                flags |= CelxLua::RenderFlagMap[name];

            ttype = tokenizer.nextToken();
            if (ttype == Tokenizer::TokenBar)
                ttype = tokenizer.nextToken();
        }
    }

    return flags;
}


int parseLabelFlags(string s)
{
    istringstream in(s);

    Tokenizer tokenizer(&in);
    int flags = 0;

    Tokenizer::TokenType ttype = tokenizer.nextToken();
    while (ttype != Tokenizer::TokenEnd)
    {
        if (ttype == Tokenizer::TokenName)
        {
            string name = tokenizer.getNameValue();

            if (CelxLua::LabelFlagMap.count(name) == 0)
                cerr << "Unknown label flag: " << name << "\n";
            else
                flags |= CelxLua::LabelFlagMap[name];

            ttype = tokenizer.nextToken();
            if (ttype == Tokenizer::TokenBar)
                ttype = tokenizer.nextToken();
        }
    }

    return flags;
}


int parseOrbitFlags(string s)
{
    istringstream in(s);

    Tokenizer tokenizer(&in);
    int flags = 0;

    Tokenizer::TokenType ttype = tokenizer.nextToken();
    while (ttype != Tokenizer::TokenEnd)
    {
        if (ttype == Tokenizer::TokenName)
        {
            string name = tokenizer.getNameValue();
            name[0] = toupper(name[0]);

            if (CelxLua::BodyTypeMap.count(name) == 0)
                cerr << "Unknown orbit flag: " << name << "\n";
            else
                flags |= CelxLua::BodyTypeMap[name];

            ttype = tokenizer.nextToken();
            if (ttype == Tokenizer::TokenBar)
                ttype = tokenizer.nextToken();
        }
    }

    return flags;
}


int parseConstellations(CommandConstellations* cmd, string s, int act)
{
    istringstream in(s);

    Tokenizer tokenizer(&in);
    int flags = 0;

    Tokenizer::TokenType ttype = tokenizer.nextToken();
    while (ttype != Tokenizer::TokenEnd)
    {
        if (ttype == Tokenizer::TokenName)
        {
            string name = tokenizer.getNameValue();
            if (compareIgnoringCase(name, "all") == 0 && act==1)
                cmd->flags.all = true;
            else if (compareIgnoringCase(name, "all") == 0 && act==0)
                cmd->flags.none = true;
            else
                cmd->setValues(name, act);

            ttype = tokenizer.nextToken();
            if (ttype == Tokenizer::TokenBar)
                ttype = tokenizer.nextToken();
        }
        else
        {
            DPRINTF(0, "Command Parser: error parsing render flags\n");
            return 0;
        }
    }

    return flags;
}


int parseConstellationColor(CommandConstellationColor* cmd, string s, Eigen::Vector3d *col, int act)
{
    istringstream in(s);

    Tokenizer tokenizer(&in);
    int flags = 0;

    if(!act)
        cmd->unsetColor();
    else
        cmd->setColor((float)col->x(), (float)col->y(), (float)col->z());

    Tokenizer::TokenType ttype = tokenizer.nextToken();
    while (ttype != Tokenizer::TokenEnd)
    {
        if (ttype == Tokenizer::TokenName)
        {
            string name = tokenizer.getNameValue();
            if (compareIgnoringCase(name, "all") == 0 && act==1)
                cmd->flags.all = true;
            else if (compareIgnoringCase(name, "all") == 0 && act==0)
                cmd->flags.none = true;
            else
                cmd->setConstellations(name);

            ttype = tokenizer.nextToken();
            if (ttype == Tokenizer::TokenBar)
                ttype = tokenizer.nextToken();
        }
        else
        {
            DPRINTF(0, "Command Parser: error parsing render flags\n");
            return 0;
        }
    }

    return flags;
}
