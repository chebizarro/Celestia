// selection.h
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _CELENGINE_SELECTION_H_
#define _CELENGINE_SELECTION_H_

#include <string>
#include <celengine/univcoord.h>
#include <Eigen/Core>

class CatEntry;
class Star;
class Body;
class Location;
class DeepSkyObject;

class Selection
{
 public:
    enum Type {
        Type_Nil,
        Type_Star,
        Type_Body,
        Type_DeepSky,
        Type_Location,
        Type_Generic
    };

public:
    Selection() : type(Type_Nil), obj(nullptr) {};
    Selection(CatEntry *cat) : type(Type_Generic), obj(cat) { checkNull(); };
    Selection(Star* star) : type(Type_Star), obj(star) { checkNull(); };
    Selection(Body* body) : type(Type_Body), obj(body) { checkNull(); };
    Selection(DeepSkyObject* deepsky) : type(Type_DeepSky), obj(deepsky) {checkNull(); };
    Selection(Location* location) : type(Type_Location), obj(location) { checkNull(); };
    Selection(const Selection& sel) : type(sel.type), obj(sel.obj) {};
    ~Selection() {};

    bool empty() const { return type == Type_Nil; }
    double radius() const;
    UniversalCoord getPosition(double t) const;
    Eigen::Vector3d getVelocity(double t) const;
    std::string getName(bool i18n = false) const;
    Selection parent() const;

    bool isVisible() const;

    Star* star() const
    {
        return type == Type_Star ? static_cast<Star*>(obj) : nullptr;
    }

    Body* body() const
    {
        return type == Type_Body ? static_cast<Body*>(obj) : nullptr;
    }

    DeepSkyObject* deepsky() const
    {
        return type == Type_DeepSky ? static_cast<DeepSkyObject*>(obj) : nullptr;
    }

    Location* location() const
    {
        return type == Type_Location ? static_cast<Location*>(obj) : nullptr;
    }

    CatEntry *catEntry() const
    {
        return type != Type_Nil ? static_cast<CatEntry*>(obj) : nullptr;
    }

    Type getType() const { return type; }

    // private:
    Type type;
    void* obj;

    void checkNull() { if (obj == nullptr) type = Type_Nil; }
};


inline bool operator==(const Selection& s0, const Selection& s1)
{
    return s0.type == s1.type && s0.obj == s1.obj;
}

inline bool operator!=(const Selection& s0, const Selection& s1)
{
    return s0.type != s1.type || s0.obj != s1.obj;
}

/*inline bool operator<(const Selection& s0, const Selection& s1)
{
    return s0.type < s1.type || s0.obj < s1.obj;
}*/

#endif // _CELENGINE_SELECTION_H_
