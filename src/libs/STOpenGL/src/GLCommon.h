/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms. 
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/


#ifndef GLCOMMON_H
#define GLCOMMON_H

#include <QtGui/qopengl.h>

#include <math/GLMath.h>

// Common provides miscellaneous functionality related to the opengl library.
// It defines the a set of common geometrical types each representing one of
// the common rendering types (ie. vertex, colour, texture coordinates, etc.)
// These types are primarily intended to be used in constructing rendering
// array structures (such as vertex arrays, colour arrays, etc.).
namespace GL
{
    // static constants & typedefs
    typedef GLuint GLflag;

    typedef GLuint GLindex;
    static const GLindex INVALID_INDEX = GLindex(-1);

    static const GLfloat EPSILON = GLfloat(1.0e-5);

    enum Initialization
    {
        Uninitialized
    };

    // misc functions
    inline const bool fuzzyEqual(GLfloat s0, GLfloat s1, GLfloat e = EPSILON);
    inline const bool fuzzyNotEqual(GLfloat s0, GLfloat s1, GLfloat e = EPSILON);

    // misc data structures
    
    // simple data pair
    template <typename T, typename U>
    struct GLpair
    {
        T first;
        U second;
    };

    // simple read-only array
    template <typename T>
    struct GLarray
    {
        inline GLarray();
        inline explicit GLarray(GL::Initialization);
        inline GLarray(GLsizei size, const T *data);

        inline const T &operator [](GLint i) const;

        GLsizei size;
        const T *data;
    };

    // geometry data structures

    // simple point
    struct GLpoint
    {
        inline GLpoint();
        inline explicit GLpoint(GL::Initialization);
        inline GLpoint(GLfloat x, GLfloat y);

        union { GLfloat x, width; };
        union { GLfloat y, height; };
    };

    // math function specialization
    inline const GLpoint min(const GLpoint &a, const GLpoint &b);
    inline const GLpoint max(const GLpoint &a, const GLpoint &b);

    inline const bool fuzzyEqual(const GLpoint &p0, const GLpoint &p1, GLfloat e = EPSILON);
    inline const bool fuzzyNotEqual(const GLpoint &p0, const GLpoint &p1, GLfloat e = EPSILON);

    inline const bool operator !=(const GLpoint &lhs, const GLpoint &rhs);
    inline const bool operator ==(const GLpoint &lhs, const GLpoint &rhs);

    inline const GLfloat distance(const GLpoint &lhs, const GLpoint &rhs);

    // simple base data
    template <typename T, int N>
    struct GLdata
    {
        static const GLuint POINTS = GLuint(N);
        T p[N];
    };

    // simple options
    typedef GLfloat GLoption;

    template <int N>
    struct GLoptiondata : public GLdata<GLoption, N> { };

    struct GLlineoption : public GLoptiondata<2>
    {
        typedef GLoptiondata<2> option_type;

        inline explicit GLlineoption(const GLoption option = GLoption());
    };
    struct GLtriangleoption : public GLoptiondata<3>
    {
        typedef GLoptiondata<3> option_type;

        inline explicit GLtriangleoption(const GLoption option = GLoption());
    };
    struct GLrectangleoption : public GLoptiondata<4>
    {
        typedef GLoptiondata<4> option_type;

        inline explicit GLrectangleoption(const GLoption option = GLoption());
    };

    // simple base shape
    template <int N>
    struct GLpointdata : public GLdata<GLpoint, N> { };

    template <int N>
    inline const bool fuzzyEqual(const GLpointdata<N> &p0, const GLpointdata<N> &p1, const GLfloat e = EPSILON);
    template <int N>
    inline const bool fuzzyNotEqual(const GLpointdata<N> &p0, const GLpointdata<N> &p1, const GLfloat e = EPSILON);

    // simple line
    struct GLline : public GLpointdata<2>
    {
        typedef GLpointdata<2> shape_type;

        inline GLline();
        inline explicit GLline(GL::Initialization);
        inline GLline(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1);
        inline GLline(const GLpoint &point0, const GLpoint &point1);
    };

    inline const bool fuzzyEqual(const GLline &l0, const GLline &l1, const GLfloat e = EPSILON);
    inline const bool fuzzyNotEqual(const GLline &l0, const GLline &l1, const GLfloat e = EPSILON);

    // simple triangle
    struct GLtriangle : public GLpointdata<3>
    {
        typedef GLpointdata<3> shape_type;

        inline GLtriangle();
        inline explicit GLtriangle(GL::Initialization);
        inline GLtriangle(GLfloat x, GLfloat y, GLfloat size);
        inline GLtriangle(const GLpoint &point, GLfloat size);
    };

    inline const bool fuzzyEqual(const GLtriangle &t0, const GLtriangle &t1, const GLfloat e = EPSILON);
    inline const bool fuzzyNotEqual(const GLtriangle &t0, const GLtriangle &t1, const GLfloat e = EPSILON);

    // simple rectangle
    struct GLrectangle : public GLpointdata<4>
    {
        typedef GLpointdata<4> shape_type;

        inline GLrectangle();
        inline explicit GLrectangle(GL::Initialization);
        inline GLrectangle(GLfloat x, GLfloat y, GLfloat size);
        inline GLrectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height);
        inline GLrectangle(const GLpoint &point, GLfloat size);
        inline GLrectangle(const GLpoint &point, GLfloat width, GLfloat height);
        inline GLrectangle(const GLpoint &p0, const GLpoint &p1, const GLpoint &p2, const GLpoint &p3);
        
        inline static const GLrectangle fromCorners(const GLpoint &topLeft, const GLpoint &bottomRight);
        static const GLrectangle fromLine(const GLpoint &from, const GLpoint &to, GLfloat lineSize = 1.0f);
    };

    // generic circle, template argument represents resolution
    template <int N>
    struct GLcircle : public GLpointdata<N>
    {
        typedef GLpointdata<N> shape_type;

        inline GLcircle();
        inline explicit GLcircle(GL::Initialization);
        inline GLcircle(GLfloat x, GLfloat y, GLfloat radius);
        inline GLcircle(const GLpoint &point, GLfloat radius);
    };

    // texture data structures

    // simple triangle
    struct GLtriangletexture : public GLpointdata<3>
    {
        typedef GLpointdata<3> shape_type;

        inline GLtriangletexture();
        inline explicit GLtriangletexture(GL::Initialization);
        inline GLtriangletexture(const GLtriangle &triangle);
        inline GLtriangletexture(const GLpoint &point0, const GLpoint &point1, const GLpoint &point2);
    };

    // simple rectangle
    struct GLrectangletexture : public GLpointdata<4>
    {
        typedef GLpointdata<4> shape_type;

        inline GLrectangletexture();
        inline explicit GLrectangletexture(GL::Initialization);
        inline GLrectangletexture(const GLrectangle &rectangle);
        inline GLrectangletexture(const GLpoint &point0, const GLpoint &point1, const GLpoint &point2, const GLpoint &point3);
    };

    // index data structures

    template <int N>
    struct GLindexdata
    {
        static const GLuint INDICIES = N;

        inline GLindexdata();
        inline explicit GLindexdata(GL::Initialization);
        inline explicit GLindexdata(const GLindex first);

        GLindex i[N];
    };
    typedef GLindexdata<2> GLlineindex;
    typedef GLindexdata<3> GLtriangleindex;
    typedef GLindexdata<4> GLrectangleindex;
    #define GLcircleindex GLindexdata
    // typedef GLindexdata<N> GLcircleindex<N>

} // namespace GL //


/****************************************** DEFINITION ******************************************/

namespace GL
{

    // misc functions
    inline const bool fuzzyEqual(GLfloat s0, GLfloat s1, GLfloat e)
    {
        return fabs(s1 - s0) <= e;
    }
    inline const bool fuzzyNotEqual(GLfloat s0, GLfloat s1, GLfloat e)
    {
        return fabs(s1 - s0) >= e;
    }

    // simple read-only array
    template <typename T>
    inline GLarray<T>::GLarray() : size(0), data(0) { }
    template <typename T>
    inline GLarray<T>::GLarray(GL::Initialization) { }
    template <typename T>
    inline GLarray<T>::GLarray(GLsizei size, const T *data)
        : size(size), data(data)
    { }
    template <typename T>
    inline const T &GLarray<T>::operator [](GLint i) const
    {
        return data[i];
    }
    
    // vertex data structures 
    
    // simple point
    inline GLpoint::GLpoint() : x(0.0f), y(0.0f) { }
    inline GLpoint::GLpoint(GL::Initialization) { }
    inline GLpoint::GLpoint(GLfloat x, GLfloat y) : x(x), y(y) { }

    inline const GLpoint min(const GLpoint &a, const GLpoint &b)
    {
        return GLpoint(GL::min<GLfloat>(a.x, b.x), GL::min<GLfloat>(a.y, b.y));
    }
    inline const GLpoint max(const GLpoint &a, const GLpoint &b)
    {
        return GLpoint(GL::max<GLfloat>(a.x, b.x), GL::max<GLfloat>(a.y, b.y));
    }

    inline const bool fuzzyEqual(const GLpoint &p0, const GLpoint &p1, GLfloat e)
    {
        return fuzzyEqual(p0.x, p1.x, e) && fuzzyEqual(p0.y, p1.y, e);
    }
    inline const bool fuzzyNotEqual(const GLpoint &p0, const GLpoint &p1, GLfloat e)
    {
        return fuzzyNotEqual(p0.x, p1.x, e) || fuzzyNotEqual(p0.y, p1.y, e);
    }

    inline const bool operator !=(const GLpoint &lhs, const GLpoint &rhs)
    {
        return (lhs.x != rhs.x) || (lhs.y != rhs.y);
    }
    inline const bool operator ==(const GLpoint &lhs, const GLpoint &rhs)
    {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y);
    }

    // simple options
    inline GLlineoption::GLlineoption(const GLoption option)
    {
        p[0] = p[1] = option;
    }
    inline GLtriangleoption::GLtriangleoption(const GLoption option)
    {
        p[0] = p[1] = p[2] = option;
    }
    inline GLrectangleoption::GLrectangleoption(const GLoption option)
    {
        p[0] = p[1] = p[2] = p[3] = option;
    }

    // simple base shape
    template <int N>
    inline const bool fuzzyEqual(const GLpointdata<N> &p0, const GLpointdata<N> &p1, const GLfloat e)
    {
        for (int i = 1; i < N; ++i)
        {
            if ( !fuzzyEqual(p0.p[i], p1.p[i], e) )
                return false;
        }
        return true;
    }
    template <int N>
    inline const bool fuzzyNotEqual(const GLpointdata<N> &p0, const GLpointdata<N> &p1, const GLfloat e)
    {
        for (int i = 1; i < N; ++i)
        {
            if ( fuzzyNotEqual(p0.p[i], p1.p[i], e) )
                return true;
        }
        return false;
    }

    inline GLline::GLline()
    {
        shape_type::p[0] = GLpoint();
        shape_type::p[1] = GLpoint();
    }
    inline GLline::GLline(GL::Initialization) { }
    inline GLline::GLline(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1)
    {
        shape_type::p[0] = GLpoint(x0, y0);
        shape_type::p[1] = GLpoint(x1, y1);
    }
    inline GLline::GLline(const GLpoint &point0, const GLpoint &point1)
    {
        shape_type::p[0] = point0;
        shape_type::p[1] = point1;
    }

    inline const bool fuzzyEqual(const GLline &l0, const GLline &l1, const GLfloat e)
    {
        return fuzzyEqual(l0.p[0], l1.p[0], e) && fuzzyEqual(l0.p[1], l1.p[1], e);
    }
    inline const bool fuzzyNotEqual(const GLline &l0, const GLline &l1, const GLfloat e)
    {
        return fuzzyNotEqual(l0.p[0], l1.p[0], e) || fuzzyNotEqual(l0.p[1], l1.p[1], e);
    }

    // siple triangle
    inline GLtriangle::GLtriangle()
    {
        shape_type::p[0] = GLpoint();
        shape_type::p[1] = GLpoint();
        shape_type::p[2] = GLpoint();
    }
    inline GLtriangle::GLtriangle(GL::Initialization) { }
    inline GLtriangle::GLtriangle(GLfloat x, GLfloat y, GLfloat size)
    {
        shape_type::p[0] = GLpoint(x, y + size);
        shape_type::p[1] = GLpoint(
            x + (size * ( 0.5 * M_SQRT3)), y + (size * (-0.5))
        );
        shape_type::p[2] = GLpoint(
            x + (size * (-0.5 * M_SQRT3)), y + (size * (-0.5))
        );
    }
    inline GLtriangle::GLtriangle(const GLpoint &point, GLfloat size)
    {
        shape_type::p[0] = GLpoint(
            point.x,
            point.y + size
        );
        shape_type::p[1] = GLpoint(
            point.x + (size * ( 0.5 * M_SQRT3)), point.y + (size * (-0.5))
        );
        shape_type::p[2] = GLpoint(
            point.x + (size * (-0.5 * M_SQRT3)), point.y + (size * (-0.5))
        );
    }

    inline const bool fuzzyEqual(const GLtriangle &t0, const GLtriangle &t1, const GLfloat e)
    {
        return fuzzyEqual(t0.p[0], t1.p[0], e)
            && fuzzyEqual(t0.p[1], t1.p[1], e)
            && fuzzyEqual(t0.p[2], t1.p[2], e);
    }
    inline const bool fuzzyNotEqual(const GLtriangle &t0, const GLtriangle &t1, const GLfloat e)
    {
        return fuzzyNotEqual(t0.p[0], t1.p[0], e)
            || fuzzyNotEqual(t0.p[1], t1.p[1], e)
            || fuzzyNotEqual(t0.p[2], t1.p[2], e);
    }

    // simple rectangle
    inline GLrectangle::GLrectangle()
    {
        shape_type::p[0] = GLpoint();
        shape_type::p[1] = GLpoint();
        shape_type::p[2] = GLpoint();
        shape_type::p[3] = GLpoint();
    }
    inline GLrectangle::GLrectangle(GL::Initialization) { }
    inline GLrectangle::GLrectangle(GLfloat x, GLfloat y, GLfloat size)
    {
        shape_type::p[0] = GLpoint(x - size / 2.0f, y - size / 2.0f);
        shape_type::p[1] = GLpoint(x + size / 2.0f, y - size / 2.0f);
        shape_type::p[2] = GLpoint(x + size / 2.0f, y + size / 2.0f);
        shape_type::p[3] = GLpoint(x - size / 2.0f, y + size / 2.0f);
    }
    inline GLrectangle::GLrectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height)
    {
        shape_type::p[0] = GLpoint(x - width / 2.0f, y - height / 2.0f);
        shape_type::p[1] = GLpoint(x + width / 2.0f, y - height / 2.0f);
        shape_type::p[2] = GLpoint(x + width / 2.0f, y + height / 2.0f);
        shape_type::p[3] = GLpoint(x - width / 2.0f, y + height / 2.0f);
    }
    inline GLrectangle::GLrectangle(const GLpoint &point, GLfloat size)
    {
        shape_type::p[0] = GLpoint(point.x - size / 2.0f, point.y - size / 2.0f);
        shape_type::p[1] = GLpoint(point.x + size / 2.0f, point.y - size / 2.0f);
        shape_type::p[2] = GLpoint(point.x + size / 2.0f, point.y + size / 2.0f);
        shape_type::p[3] = GLpoint(point.x - size / 2.0f, point.y + size / 2.0f);
    }
    inline GLrectangle::GLrectangle(const GLpoint &point, GLfloat width, GLfloat height)
    {
        shape_type::p[0] = GLpoint(point.x - width / 2.0f, point.y - height / 2.0f);
        shape_type::p[1] = GLpoint(point.x + width / 2.0f, point.y - height / 2.0f);
        shape_type::p[2] = GLpoint(point.x + width / 2.0f, point.y + height / 2.0f);
        shape_type::p[3] = GLpoint(point.x - width / 2.0f, point.y + height / 2.0f);
    }
    inline GLrectangle::GLrectangle(const GLpoint &p0, const GLpoint &p1, const GLpoint &p2, const GLpoint &p3)
    {
        shape_type::p[0] = p0;
        shape_type::p[1] = p1;
        shape_type::p[2] = p2;
        shape_type::p[3] = p3;
    }

    inline const GLrectangle GLrectangle::fromCorners(const GLpoint &topLeft, const GLpoint &bottomRight)
    {
        GLrectangle rectangle(GL::Uninitialized);
        rectangle.p[0] = topLeft;
        rectangle.p[1] = GLpoint(bottomRight.x, topLeft.y);
        rectangle.p[2] = bottomRight;
        rectangle.p[3] = GLpoint(topLeft.x, bottomRight.y);
        return rectangle;
    }
    
    // generic circle
    template <int N>
    inline GLcircle<N>::GLcircle()
    {
        for (int i=0; i<N; ++i)
        {
            shape_type::p[i] = GLpoint();
        }
    }
    template <int N>
    inline GLcircle<N>::GLcircle(GL::Initialization) { }
    template <int N>
    inline GLcircle<N>::GLcircle(GLfloat x, GLfloat y, GLfloat radius)
    {
        static const GLfloat POINTS_INV = 1.0f / (GLfloat) (shape_type::POINTS - 1);
        
        shape_type::p[0] = GLpoint(x, y);
        for (int i = 1; i < N; ++i)
        {
            shape_type::p[i] = GLpoint(
                x + (radius * qFastCos(POINTS_INV * i * (2.0 * M_PI))),
                y + (radius * qFastSin(POINTS_INV * i * (2.0 * M_PI)))
            );
        }
    }
    template <int N>
    inline GLcircle<N>::GLcircle(const GLpoint &p, GLfloat radius)
    {
        static const GLfloat POINTS_INV = 1.0f / (GLfloat) (shape_type::POINTS - 1);
        
        shape_type::p[0] = p;
        for (int i = 1; i < N; ++i)
        {
            shape_type::p[i] = GLpoint(
                p.x + (radius * qFastCos(POINTS_INV * i * (2.0 * M_PI))),
                p.y + (radius * qFastSin(POINTS_INV * i * (2.0 * M_PI)))
            );
        }
    }

    // texture data structures

    // simple triangle
    inline GLtriangletexture::GLtriangletexture()
    {                                           // 0 ___ 1
        shape_type::p[0] = GLpoint(0.0f, 0.0f); //  |  /
        shape_type::p[1] = GLpoint(1.0f, 0.0f); //  | /
        shape_type::p[2] = GLpoint(0.0f, 1.0f); //  |/
    }                                           //  2
    inline GLtriangletexture::GLtriangletexture(GL::Initialization) { }
    inline GLtriangletexture::GLtriangletexture(const GLtriangle &triangle)
    {
        shape_type::p[0] = triangle.p[0];
        shape_type::p[1] = triangle.p[1];
        shape_type::p[2] = triangle.p[2];
    }
    inline GLtriangletexture::GLtriangletexture(const GLpoint &point0, const GLpoint &point1, const GLpoint &point2)
    {
        shape_type::p[0] = point0;
        shape_type::p[1] = point1;
        shape_type::p[2] = point2;
    }

    // simple rectangle
    inline GLrectangletexture::GLrectangletexture()
    {
        shape_type::p[0] = GLpoint(0.0f, 0.0f); // 0 ___ 1
        shape_type::p[1] = GLpoint(1.0f, 0.0f); //  |   |
        shape_type::p[2] = GLpoint(1.0f, 1.0f); //  |___|
        shape_type::p[3] = GLpoint(0.0f, 1.0f); // 3     4
    }
    inline GLrectangletexture::GLrectangletexture(GL::Initialization) { }
    inline GLrectangletexture::GLrectangletexture(const GLrectangle &rectangle)
    {
        shape_type::p[0] = rectangle.p[0];
        shape_type::p[1] = rectangle.p[1];
        shape_type::p[2] = rectangle.p[2];
        shape_type::p[3] = rectangle.p[3];
    }
    inline GLrectangletexture::GLrectangletexture(const GLpoint &point0, const GLpoint &point1, 
                                                  const GLpoint &point2, const GLpoint &point3)
    {
        shape_type::p[0] = point0;
        shape_type::p[1] = point1;
        shape_type::p[2] = point2;
        shape_type::p[3] = point3;
    }
    
    // index data structures 

    template <int N>
    inline GLindexdata<N>::GLindexdata()
    {
        for (int i=0; i<N; ++i)
        {
            GLindexdata<N>::i[i] = GL::INVALID_INDEX;
        }
    }
    template <int N>
    inline GLindexdata<N>::GLindexdata(GL::Initialization) { }
    template <int N>
    inline GLindexdata<N>::GLindexdata(const GLindex first)
    {
        for (GLuint i=0; i<GLindexdata<N>::INDICIES; ++i)
        {
            GLindexdata<N>::i[i] = (first + i);
        }
    }
    
} // namespace GL //

#endif // GLCOMMON_H //
