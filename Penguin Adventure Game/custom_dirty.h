#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

// Old Helper functions made by me when i was first learning C++. 
// I will most likely not be using these functions in my game. 
// I will be cleaning them up and making them more efficient as i go along.

//---math constants---
#ifndef PI
#define PI 3.14159265358979323846l

#endif
//---end---

//--- custom print functions ---
void Print(const std::string& message) {
    std::cout << message << std::endl;
}
void Print(int number) {
    std::cout << number << std::endl;
}
void Print(double number) {
    std::cout << number << std::endl;
}
//---end---

// ---math functions---
double DegToRad(double degrees) {
    // Converts degrees to radians
    //because sin, cos, tan etc. in cmath use radians
    return degrees * (PI / 180.0);
}
double RadToDeg(double radians) {
    // Converts radians to degrees
    return radians * (180.0 / PI);
}
double SinDeg(double degrees) {
    return sin(DegToRad(degrees));
}
double CosDeg(double degrees) {
    return cos(DegToRad(degrees));
}
double TanDeg(double degrees) {
    return tan(DegToRad(degrees));
}
double SecDeg(double degrees) {
    return 1.0 / CosDeg(degrees);
}
double CscDeg(double degrees) {
    return 1.0 / SinDeg(degrees);
}
double CotDeg(double degrees) {
    return 1.0 / TanDeg(degrees);
}
double AbsoluteValue(double value) {
    // Returns the absolute value of a number
    return (value < 0) ? -value : value;
}







double Clamp(double value, double min, double max) {
    // Clamps a value between a minimum and maximum
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
double LinearInterpolate(double Input1, double Input2, double Time) {
    // Linearly interpolates between Input1 and Input2 by Time (0.0 to 1.0)
    return Input1 + (Input2 - Input1) * Time;
}
double SmoothInterpolate(double Input1, double Input2, double Time) {
    // Smoothly interpolates between Input1 and Input2 by Time (0.0 to 1.0)
    Time = Clamp(Time, 0.0, 1.0);
    Time = Time * Time * (3 - 2 * Time); // Smoothstep function
    return LinearInterpolate(Input1, Input2, Time);
}
double SquishToUnit(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}
double SquishToRange(double x, double min, double max) {
    return min + (max - min) * SquishToUnit(x);
}
// ---end---

// ---vector functions---

// 2D Vector structure
struct Vector2 {
    double x;
    double y;
    // ---normal operations---
    Vector2 operator+(const Vector2& other) const {
        // Adds two Vector2 objects
        //Mathematically, (1,2) + (3,4) = (1+3, 2+4) = (4,6)
        return { x + other.x, y + other.y };
    }
    Vector2 operator-(const Vector2& other) const {
        // Subtracts two Vector2 objects
        //Mathematically, (1,2) - (3,4) = (1-3, 2-4) = (-2,-2)
        return { x - other.x, y - other.y };
    }
    Vector2 operator*(double scalar) const {
        // Multiplies a Vector2 by a scalar
        //Mathematically, (1,2) * 3 = (1*3, 2*3) = (3,6)
        return { x * scalar, y * scalar };
    }
    Vector2 operator/(double scalar) const {
        // Divides a Vector2 by a scalar
        //Mathematically, (1,2) / 2 = (1/2, 2/2) = (0.5,1)
        return { x / scalar, y / scalar };
    }
    Vector2& operator+=(const Vector2& other) {
        // Adds another Vector2 to this one
        x += other.x;
        y += other.y;
        return *this;
    }
    Vector2& operator-=(const Vector2& other) {
        // Subtracts another Vector2 from this one
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Vector2& operator*=(double scalar) {
        // Multiplies this Vector2 by a scalar
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Vector2& operator/=(double scalar) {
        // Divides this Vector2 by a scalar
        x /= scalar;
        y /= scalar;
        return *this;
    }
    // ---end---

    // ---advanced operations---
    double Magnitude() const {
        // Returns the magnitude (length) of the vector
        //Mathematically, |(x,y)| = sqrt(x^2 + y^2)
        return std::sqrt(x * x + y * y);
    }
    Vector2 Normalize() const {
        // Returns a normalized (unit length) version of the vector
        //Mathematically, (x,y) normalized = (x/mag, y/mag)
        double mag = Magnitude();
        if (mag == 0) return { 0, 0 }; // Avoid division by zero
        return { x / mag, y / mag };
    }
    double Dot(const Vector2& other) const {
        // Returns the dot product of this vector and another
        //Mathematically, (x1,y1) · (x2,y2) = x1*x2 + y1*y2
        return x * other.x + y * other.y;
    }
    double Cross(const Vector2& other) const {
        // Returns the 2D cross product (scalar) of this vector and another
        //Mathematically, (x1,y1) x (x2,y2) = x1*y2 - y1*x2
        return x * other.y - y * other.x;
    }
    Vector2 Rotate(double angleDegrees) const {
        // Rotates the vector by a given angle in degrees
        //Mathematically, (x,y) rotated by A = (x*cosA - y*sinA, x*sinA + y*cosA)
        // Where A is the angle in radians
        double cosA = CosDeg(angleDegrees);
        double sinA = SinDeg(angleDegrees);
        return { x * cosA - y * sinA, x * sinA + y * cosA };
    }
    Vector2 Vector2LinearInterpolate(const Vector2& other, double time) const {
        // Linearly interpolates between this vector and another by time (0.0 to 1.0)
        return {
            LinearInterpolate(x, other.x, time),
            LinearInterpolate(y, other.y, time)
        };
    }
    Vector2 Vector2SmoothInterpolate(const Vector2& other, double time) const {
        // Smoothly interpolates between this vector and another by time (0.0 to 1.0)
        return {
            SmoothInterpolate(x, other.x, time),
            SmoothInterpolate(y, other.y, time)
        };
    }
    Vector2 SquishToRange(const Vector2& min, const Vector2& max) const {
        // Squishes each component of the vector to a specified range
        return {
            ::SquishToRange(x, min.x, max.x),
            ::SquishToRange(y, min.y, max.y)
        };
    }


};


// 3D Vector structure
struct Vector3 {
    double x;
    double y;
    double z;

    // ---normal operations---
    Vector3 operator+(const Vector3& other) const {
        // Adds two Vector3 objects
        //Mathematically, (1,2,3) + (4,5,6) = (1+4, 2+5, 3+6) = (5,7,9)
        return { x + other.x, y + other.y, z + other.z };
    }
    Vector3 operator-(const Vector3& other) const {
        // Subtracts two Vector3 objects
        //Mathematically, (1,2,3) - (4,5,6) = (1-4, 2-5, 3-6) = (-3,-3,-3)
        return { x - other.x, y - other.y, z - other.z };
    }
    Vector3 operator*(double scalar) const {
        // Multiplies a Vector3 by a scalar
        //Mathematically, (1,2,3) * 2 = (1*2, 2*2, 3*2) = (2,4,6)
        return { x * scalar, y * scalar, z * scalar };
    }
    Vector3 operator/(double scalar) const {
        // Divides a Vector3 by a scalar
        //Mathematically, (1,2,3) / 2 = (1/2, 2/2, 3/2) = (0.5,1,1.5)
        return { x / scalar, y / scalar, z / scalar };
    }
    Vector3& operator+=(const Vector3& other) {
        // Adds another Vector3 to this one
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vector3& operator-=(const Vector3& other) {
        // Subtracts another Vector3 from this one
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    Vector3& operator*=(double scalar) {
        // Multiplies this Vector3 by a scalar
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vector3& operator/=(double scalar) {
        // Divides this Vector3 by a scalar
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
    // ---end---

    // ---advanced operations---
    double Magnitude() const {
        // Returns the magnitude (length) of the vector
        //Mathematically, |(x,y,z)| = sqrt(x^2 + y^2 + z^2)
        return std::sqrt(x * x + y * y + z * z);
    }
    Vector3 Normalize() const {
        // Returns a normalized (unit length) version of the vector
        //Mathematically, (x,y,z) normalized = (x/mag, y/mag, z/mag)
        double mag = Magnitude();
        if (mag == 0) return { 0, 0, 0 }; // Avoid division by zero
        return { x / mag, y / mag, z / mag };
    }
    double Dot(const Vector3& other) const {
        // Returns the dot product of this vector and another
        //Mathematically, (x1,y1,z1) · (x2,y2,z2) = x1*x2 + y1*y2 + z1*z2
        return x * other.x + y * other.y + z * other.z;
    }
    Vector3 Cross(const Vector3& other) const {
        // Returns the cross product of this vector and another
        //Mathematically, (x1,y1,z1) x (x2,y2,z2) = (y1*z2 - z1*y2, z1*x2 - x1*z2, x1*y2 - y1*x2)
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }
    Vector3 Vector3LinearInterpolate(const Vector3& other, double time) const {
        // Linearly interpolates between this vector and another by time (0.0 to 1.0)
        return {
            LinearInterpolate(x, other.x, time),
            LinearInterpolate(y, other.y, time),
            LinearInterpolate(z, other.z, time)
        };
    }
    Vector3 Vector3SmoothInterpolate(const Vector3& other, double time) const {
        // Smoothly interpolates between this vector and another by time (0.0 to 1.0)
        return {
            SmoothInterpolate(x, other.x, time),
            SmoothInterpolate(y, other.y, time),
            SmoothInterpolate(z, other.z, time)
        };
    }
    Vector3 SquishToRange(const Vector3& min, const Vector3& max) const {
        // Squishes each component of the vector to a specified range
        return {
            ::SquishToRange(x, min.x, max.x),
            ::SquishToRange(y, min.y, max.y),
            ::SquishToRange(z, min.z, max.z)
        };
    }
};

// ---end---

// --- Matrix functions---
// (For simplicity, only 2D matrices are implemented here)
struct Matrix2x2 {
    // a mtrix looks like this:
    //--------
    // | a b |
    // | c d |
    //--------


    double m[2][2];

    // ---normal operations---
    // --- ---addition---
    Matrix2x2 operator+(const Matrix2x2& other) const {
        // Adds two 2x2 matrices
        //Mathematically Looks like this:
        //--------
        // | a b |   | e f |   | a+e b+f |
        // | c d | + | g h | = | c+g d+h |
        //--------
        //Numerical Example:
        //--------
        // | 1 2 |   | 5 6 |   | 1+5 2+6 |   | 6  8 |
        // | 3 4 | + | 7 8 | = | 3+7 4+8 | = | 10 12|
        //--------

        return { {
            {m[0][0] + other.m[0][0], m[0][1] + other.m[0][1]},
            {m[1][0] + other.m[1][0], m[1][1] + other.m[1][1]}
        } };
    }
    // --- ---subtraction---
    Matrix2x2 operator-(const Matrix2x2& other) const {
        // Subtracts two 2x2 matrices
        //Mathematically Looks like this:
        //--------
        // | a b |   | e f |   | a-e b-f |
        // | c d | - | g h | = | c-g d-h |
        //--------
        //Numerical Example:
        //--------
        // | 5 6 |   | 1 2 |   | 5-1 6-2 |   | 4 4 |
        // | 7 8 | - | 3 4 | = | 7-3 8-4 | = | 4 4 |
        //--------

        return { {
            {m[0][0] - other.m[0][0], m[0][1] - other.m[0][1]},
            {m[1][0] - other.m[1][0], m[1][1] - other.m[1][1]}
        } };
    }
    // --- ---multiplication of 2 Maticies---
    Matrix2x2 operator*(const Matrix2x2& other) const {
        // Multiplies two 2x2 matrices
        //Mathematically Looks like this:
        //--------
        // | a b |   | e f |   | a*e+b*g a*f+b*h |
        // | c d | * | g h | = | c*e+d*g c*f+d*h |
        //--------
        //Numerical Example:
        //--------
        // | 1 2 |   | 5 6 |   | 1*5+2*7 1*6+2*8 |   | 19 22 |
        // | 3 4 | * | 7 8 | = | 3*5+4*7 3*6+4*8 | = | 43 50 |
        //--------

        return { {
            {
                m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0],
                m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1]
            },
            {
                m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0],
                m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1]
            }
        } };
    }
    // --- --- division of 2 Maticies---
    Matrix2x2 operator/(const Matrix2x2& other) const {
        // Divides two 2x2 matrices (multiplication by the inverse of the other matrix)
        //Mathematically Looks like this:
        //--------
        // | a b |   | e f |   | a b | * |  d -b |   1
        // | c d | / | g h | = | c d | * | -g  e | * ---
        //                        |       |       | e*d-b*g
        //--------
        //Numerical Example:
        //--------
        // | 19 22 |   | 5 6 |   | 19 22 | * |  8 -6 |   1
        // | 43 50 | / | 7 8 | = | 43 50 | * | -7  5 | * ---
        //                            |       |       | 5*8-6*7
        //--------


        double det = other.m[0][0] * other.m[1][1] - other.m[0][1] * other.m[1][0];
        if (det == 0) throw std::runtime_error("Matrix is singular and cannot be divided.");

        Matrix2x2 inverse = { {
            {other.m[1][1] / det, -other.m[0][1] / det},
            {-other.m[1][0] / det, other.m[0][0] / det}
        } };

        return (*this) * inverse;
    }

};

// ---end---






