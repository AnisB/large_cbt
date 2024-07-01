#ifndef DOUBLE_MATH_HLSL
#define DOUBLE_MATH_HLSL

double invsqrt_double(double number)
{
    double y = number;
    double x2 = y * 0.5;
    uint low, high;
    asuint(number, low, high);
    int64_t i = (int64_t(high) << 32ull) | int64_t(low);
    // The magic number is for doubles is from https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
    i = 0x5fe6eb50c7b537a9 - (i >> 1);
    y = asdouble(uint(i & 0xffffffffull), uint((i >> 32ull) & 0xffffffffull));
    y = y * (1.5 - (x2 * y * y));   // 1st iteration
    y = y * (1.5 - (x2 * y * y));   // 2nd iteration
    y = y * (1.5 - (x2 * y * y));   // 3nd iteration
    return y;
}

double sqrt_double(double number)
{
    return number < 1e-32 ? 0.0 : 1.0 / invsqrt_double(number);
}

double length_double(double3 vec)
{
    return sqrt_double(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

double floor_double(double v)
{
    double r = double(int64_t(v));
    return v < 0 ? r - 1 : r;
}

double2 floor_double2(double2 v)
{
    v.x = floor_double(v.x);
    v.y = floor_double(v.y);
    return v;
}

double3 floor_double3(double3 v)
{
    v.x = floor_double(v.x);
    v.y = floor_double(v.y);
    v.z = floor_double(v.z);
    return v;
}

double frac_double(double v)
{
    return v - floor_double(v);
}

double2 frac_double2(double2 v)
{
    return v - floor_double2(v);
}

double3 frac_double3(double3 v)
{
    return v - floor_double3(v);
}

double dot_double(double2 v0, double2 v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

double dot_double(double3 v0, double3 v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

double mod_double(double v0, double v1)
{
    return v0 - int(v0 / v1) * v1;
}

double2 mod_double2(double2 v0, double2 v1)
{
    v0.x = mod_double(v0.x, v1.x);
    v0.y = mod_double(v0.y, v1.y);
    return v0;
}

double3 mod_double3(double3 v0, double3 v1)
{
    v0.x = mod_double(v0.x, v1.x);
    v0.y = mod_double(v0.y, v1.y);
    v0.z = mod_double(v0.z, v1.z);
    return v0;
}

double acos_double(double x)
{
    double y = abs(clamp(x, -1.0, 1.0));
    double sqrtY = y != 1.0 ? sqrt_double(1.0 - y) : 0.0;
    double z = (-0.168577 * y + 1.56723) * sqrtY;
    return x > 0.0 ? z : 0.5 * PI;
}
double asin_double(double x)
{
  double negate = double(x < 0);
  x = abs(x);
  double ret = -0.0187293;
  ret *= x;
  ret += 0.0742610;
  ret *= x;
  ret -= 0.2121144;
  ret *= x;
  ret += 1.5707288;
  ret = 3.14159265358979*0.5 - sqrt_double(1.0 - x)*ret;
  return ret - 2 * negate * ret;
}

static const double atan_tbl[] = {
    -3.333333333333333333333333333303396520128e-1,
     1.999999117496509842004185053319506031014e-1,
    -1.428514132711481940637283859690014415584e-1,
     1.110012236849539584126568416131750076191e-1,
    -8.993611617787817334566922323958104463948e-2,
     7.212338962134411520637759523226823838487e-2,
    -5.205055255952184339031830383744136009889e-2,
     2.938542391751121307313459297120064977888e-2,
    -1.079891788348568421355096111489189625479e-2,
     1.858552116405489677124095112269935093498e-3
};

// https://outerra.blogspot.com/2014/05/double-precision-approximations-for-map.html
double atan2_double(double y, double x)
{
#if 0
    //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
    double ONEQTR_PI = PI / 4.0;
    double THRQTR_PI = 3.0 * PI / 4.0;
    double r, angle;
    double abs_y = abs(y) + 1e-10f;      // kludge to prevent 0/0 condition
    if ( x < 0.0f )
    {
        r = (x + abs_y) / (abs_y - x);
        angle = THRQTR_PI;
    }
    else
    {
        r = (x - abs_y) / (x + abs_y);
        angle = ONEQTR_PI;
    }
    angle += (0.1963f * r * r - 0.9817f) * r;
    if ( y < 0.0f )
        return( -angle );     // negate if in quad III or IV
    else
        return( angle );
#else
    double ax = abs(x);
    double ay = abs(y);
    double t0 = max(ax, ay);
    double t1 = min(ax, ay);
    double a = (1 / t0 ) * t1;
    double s = a * a;
    double p = atan_tbl[9];

    p = fma( fma( fma( fma( fma( fma( fma( fma( fma( fma(p, s,
        atan_tbl[8]), s,
        atan_tbl[7]), s, 
        atan_tbl[6]), s,
        atan_tbl[5]), s,
        atan_tbl[4]), s,
        atan_tbl[3]), s,
        atan_tbl[2]), s,
        atan_tbl[1]), s,
        atan_tbl[0]), s*a, a);

    double r = ay > ax ? (HALF_PI - p) : p;
    r = x < 0 ?  PI - r : r;
    r = y < 0 ? -r : r;
    return r;
#endif
}

// https://outerra.blogspot.com/2017/06/fp64-approximations-for-sincos-for.html
double sin_double(double x)
{
    //minimax coefs for sin for 0..pi/2 range
    const double a3 = -1.666665709650470145824129400050267289858e-1;
    const double a5 =  8.333017291562218127986291618761571373087e-3;
    const double a7 = -1.980661520135080504411629636078917643846e-4;
    const double a9 =  2.600054767890361277123254766503271638682e-6;

    const double m_2_pi = 0.636619772367581343076;
    const double m_pi_2 = 1.57079632679489661923;

    double y = abs(x * m_2_pi);
    double q = floor_double(y);
    int quadrant = int(q);

    double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
    t *= m_pi_2;
    double t2 = t * t;
    double r = fma(fma(fma(fma(a9, t2, a7), t2, a5), t2, a3), t2*t, t);
    r = x < 0 ? -r : r;
    return (quadrant & 2) != 0 ? -r : r;
}

double cos_double(double x)
{
	//sin(x + PI/2) = cos(x)
    return sin_double(x + 1.57079632679489661923);
}

double2 sincos_double(double t)
{
#if 0
    double2 f = abs(frac_double2(t-double2(0.25,0.0))-0.5);
    double h = abs(frac_double(t*4.0)-0.5);
    return (-1.0 + f*f*(24.0-32.0*f)) * (1.028519 - 0.0570379*h); 
#else
	return double2(sin_double(t), cos_double(t));
#endif
}

#endif // DOUBLE_MATH_HLSL