/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "Voronoy.h"
#define Pi  (3.141592653589793238462643383279502884)

namespace Cicm
{
    // Voronoy Circle
    Voronoy::VoronoyCircle::VoronoyCircle(double _x, double _y, double _r)
    {
        x = _x;
        y = _y;
        r = _r;
    };
    
    Voronoy::VoronoyCircle::~VoronoyCircle()
    {
        ;
    };
    
    // Voronoy Point
    Voronoy::VoronoyPoint::VoronoyPoint(double _x, double _y){
        x = _x;
        y = _y;
    };
    
    Voronoy::VoronoyPoint::~VoronoyPoint()
    {
        circles.clear();
    };
    
    // Voronoy
	Voronoy::Voronoy(Mode _mode)
    {
        mode = _mode;
    };

	Voronoy::~Voronoy()
	{
		clear();
	};

	void Voronoy::clear()
	{
		points.clear();
        points_bottom.clear();
	};

	void Voronoy::addPointCartesian(double _abscissa, double _ordinate, double _heiht)
	{
        if(mode == Plane)
            points.push_back(VoronoyPoint(_abscissa, _ordinate));
        else if(mode == Circle)
        {
            double _azimuth;
            if(_abscissa == 0 && _ordinate == 0)
                _azimuth = 0;
            else
                _azimuth = atan2(_ordinate, _abscissa);
            points.push_back(VoronoyPoint(cos(_azimuth), sin(_azimuth)));
        }
        else
        {
            double _azimuth, _elevation;
            if(_abscissa == 0 && _ordinate == 0)
                _azimuth = 0;
            else
                _azimuth = atan2(_ordinate, _abscissa);
            if(_abscissa == 0 && _ordinate == 0 && _heiht == 0)
                _elevation = 0;
            else
            {
                _elevation = acos(_heiht / sqrt(_abscissa * _abscissa + _ordinate * _ordinate + _heiht * _heiht));
            }
            addPointPolar(1., _azimuth, _elevation);
        }
	}
    
    void Voronoy::addPointPolar(double _radius, double _azimuth, double _elevation)
    {
        if(mode == Plane)
            points.push_back(VoronoyPoint(_radius * cos(_azimuth), _radius * sin(_azimuth)));
        else if(mode == Circle)
             points.push_back(VoronoyPoint(cos(_azimuth), sin(_azimuth)));
        else
        {
            if(_elevation >= 0)
            {
                points.push_back(VoronoyPoint(cos(_azimuth) * fabs(2. * _elevation / Pi), sin(_azimuth) * fabs(2. * _elevation / Pi)));
                points_bottom.push_back(VoronoyPoint(cos(_azimuth) * (2. - fabs(2. * _elevation / Pi)), sin(_azimuth) * (2. - fabs(2. * _elevation / Pi))));
                
            }
            else
            {
                points_bottom.push_back(VoronoyPoint(cos(_azimuth) * fabs(2. * _elevation / Pi), sin(_azimuth) * fabs(2. * _elevation / Pi)));
                points.push_back(VoronoyPoint(cos(_azimuth) * (2. - fabs(2. * _elevation / Pi)), sin(_azimuth) * (2. - fabs(2. * _elevation / Pi))));
            }
        }
    }
			
	void Voronoy::perform()
	{
        if(mode == Sphere)
        {
            int size = points.size();
            for(int i = 0; i < size - 2; i++)
            {
                for(int j = i+1; j < size - 1; j++)
                {
                    for(int k = j+1; k < size; k++)
                    {
                        evaluateTriangle(i, j, k);
                    }
                }
            }
            for(int i = 0; i < points.size(); i++)
            {
                if(sqrt(points[i].x * points[i].x + points[i].y * points[i].y) > 1)
                {
                    points.erase(points.begin()+i);
                    i--;
                }
            }
            for(int i = 0; i < size; i++)
            {
                for(int j = 0; j < points[i].circles.size(); j++)
                    points[i].circles[j].pt = &points[i];
                std::sort(points[i].circles.begin(), points[i].circles.end(), VoronoyCircle::compare);
            }
            
            size = points_bottom.size();
            for(int i = 0; i < size - 2; i++)
            {
                for(int j = i+1; j < size - 1; j++)
                {
                    for(int k = j+1; k < size; k++)
                    {
                        evaluateTriangle(i, j, k, 0);
                    }
                }
            }
            for(int i = 0; i < points_bottom.size(); i++)
            {
                if(sqrt(points_bottom[i].x * points_bottom[i].x + points_bottom[i].y * points_bottom[i].y) > 1)
                {
                    points_bottom.erase(points_bottom.begin()+i);
                    i--;
                }
            }
            for(int i = 0; i < size; i++)
            {
                for(int j = 0; j < points_bottom[i].circles.size(); j++)
                    points_bottom[i].circles[j].pt = &points_bottom[i];
                std::sort(points_bottom[i].circles.begin(), points_bottom[i].circles.end(), VoronoyCircle::compare);
            }
            points.insert(points.begin(), points_bottom.begin(), points_bottom.end());
            points_bottom.clear();
        }
        else
        {
            int size = points.size();
            for(int i = 0; i < size - 2; i++)
            {
                for(int j = i+1; j < size - 1; j++)
                {
                    for(int k = j+1; k < size; k++)
                    {
                        evaluateTriangle(i, j, k);
                    }
                }
            }
            
            for(int i = 0; i < size; i++)
            {
                for(int j = 0; j < points[i].circles.size(); j++)
                    points[i].circles[j].pt = &points[i];
                std::sort(points[i].circles.begin(), points[i].circles.end(), VoronoyCircle::compare);
            }
        }
	};

	void Voronoy::evaluateTriangle(int i, int j, int k, bool top)
	{
        if(top)
        {
            int size = points.size();
            double abs, ord, circle_abscissa, circle_ordinate, circle_radius;
            double ikx = (points[i].x - points[k].x), jky = (points[j].y - points[k].y), jkx = (points[j].x - points[k].x), iky = (points[i].y - points[k].y), ik_x = (points[i].x + points[k].x), ik_y = (points[i].y + points[k].y), jk_x = (points[j].x + points[k].x), jk_y = (points[j].y + points[k].y);
            double D = ikx * jky - jkx * iky;
            if(!D)
                return;
            
            circle_abscissa = ((ikx * ik_x + iky * ik_y) * 0.5 * jky - (jkx * jk_x + jky * jk_y) * 0.5 * iky) / D;
            circle_ordinate = ((jkx * jk_x + jky * jk_y) * 0.5 * ikx - (ikx * ik_x + iky * ik_y) * 0.5 * jkx) / D;
            abs = circle_abscissa - points[i].x;
            ord = circle_ordinate - points[i].y;
            circle_radius = sqrt(abs * abs + ord * ord);
            
            // If one point is inside the circle, the circle is exclude.
            for(int l = 0; l < size; l++)
            {
                if(l != i && l != j && l != k)
                {
                    abs = circle_abscissa - points[l].x;
                    ord = circle_ordinate - points[l].y;
                    if(sqrt(abs * abs + ord * ord) < circle_radius)
                    {
                        return;
                    }
                }
            }
            
            // Else, the points save the circle and the circle save the the points.
            VoronoyCircle circle = VoronoyCircle(circle_abscissa, circle_ordinate, circle_radius);
            points[i].circles.push_back(circle);
            points[j].circles.push_back(circle);
            points[k].circles.push_back(circle);
        }
        else
        {
            int size = points_bottom.size();
            double abs, ord, circle_abscissa, circle_ordinate, circle_radius;
            double ikx = (points_bottom[i].x - points_bottom[k].x), jky = (points_bottom[j].y - points_bottom[k].y), jkx = (points_bottom[j].x - points_bottom[k].x), iky = (points_bottom[i].y - points_bottom[k].y), ik_x = (points_bottom[i].x + points_bottom[k].x), ik_y = (points_bottom[i].y + points_bottom[k].y), jk_x = (points_bottom[j].x + points_bottom[k].x), jk_y = (points_bottom[j].y + points_bottom[k].y);
            double D = ikx * jky - jkx * iky;
            if(!D)
                return;
            
            circle_abscissa = ((ikx * ik_x + iky * ik_y) * 0.5 * jky - (jkx * jk_x + jky * jk_y) * 0.5 * iky) / D;
            circle_ordinate = ((jkx * jk_x + jky * jk_y) * 0.5 * ikx - (ikx * ik_x + iky * ik_y) * 0.5 * jkx) / D;
            abs = circle_abscissa - points_bottom[i].x;
            ord = circle_ordinate - points_bottom[i].y;
            circle_radius = sqrt(abs * abs + ord * ord);
            
            // If one point is inside the circle, the circle is exclude.
            for(int l = 0; l < size; l++)
            {
                if(l != i && l != j && l != k)
                {
                    abs = circle_abscissa - points_bottom[l].x;
                    ord = circle_ordinate - points_bottom[l].y;
                    if(sqrt(abs * abs + ord * ord) < circle_radius)
                    {
                        return;
                    }
                }
            }
            
            // Else, the points save the circle and the circle save the the points.
            VoronoyCircle circle = VoronoyCircle(circle_abscissa, circle_ordinate, circle_radius);
            points_bottom[i].circles.push_back(circle);
            points_bottom[j].circles.push_back(circle);
            points_bottom[k].circles.push_back(circle);
        }
	};
}
