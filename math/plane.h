
#ifndef _MATH_GEOMETRY_PLANE__
#define _MATH_GEOMETRY_PLANE__

#include <vector>

#include "math_common.h"
#include "vecg.h"
#include "line.h"


// A 3D Plane of equation a.x + b.y + c.z + d = 0
template <class FT> 
class GenericPlane3 
{
public:
	typedef vecng<3, FT>		Point ;
	typedef vecng<3, FT>		Vector ;
	typedef vecng<4, FT>		vec4;
	typedef GenericLine<3, FT>	Line ;
	typedef GenericPlane3<FT>	thisclass ;

public:
	GenericPlane3(const Point& p1, const Point& p2, const Point& p3) ;
	GenericPlane3(const Point& p, const Vector& n) ;
	GenericPlane3(double a, double b, double c, double d) : a_(a), b_(b), c_(c), d_(d) {}
	GenericPlane3() {}

	Vector normal() const ;

	FT  value(const Point& p) const { return (a_*p.x + b_*p.y + c_*p.z + d_) ; }

	// return values:
	//   POSITIVE: p is on the positive side
	//   NEGATIVE: p is on the negative side
	//   ZERO:	   p is belonging to the plane
	Sign  orient(const Point& p) const ;

	// the projection of p on this plane
	Point projection(const Point &p) const ;

	FT	  squared_ditance(const Point &p) const ;

	bool  intersection(const Line& line, Point& p) const ;

	// fit plane to points
	bool FitToPoints(const std::vector<Point>& Points);
	bool FitToPoints(const std::vector<vec4>& Points, Point& Basis1, Point& Basis2, FT& NormalEigenvalue);

private:
	void Find_ScatterMatrix(const std::vector<vec4> &Points, const Point &Centroid, FT ScatterMatrix[3][3], int Order[3]) const;
	FT sign(FT a, FT b) const;
	void tred2(FT a[3][3], FT d[3], FT e[3]) const;
	void tqli(FT d[3], FT e[3], FT z[3][3]) const;

private:
	FT a_;
	FT b_;
	FT c_;
	FT d_;
} ;





template <class FT> inline
GenericPlane3<FT>::GenericPlane3(const Point& p1, const Point& p2, const Point& p3) {
	Vector n = cross(p2 - p1, p3 - p1);
	a_ = n.x;
	b_ = n.y;
	c_ = n.z;
	d_ = -(a_*p1.x + b_*p1.y + c_*p1.z);

#ifndef NDEBUG // degenerate case
	if (length(n) < 1e-15) {
		std::cerr << "degenerate plane constructed from 3 points:" << std::endl
			<< "\t(" << p1 << ")" << std::endl
			<< "\t(" << p2 << ")" << std::endl
			<< "\t(" << p3 << ")" << std::endl;
	}
#endif
}

template <class FT> inline
GenericPlane3<FT>::GenericPlane3(const Point& p, const Vector& n) {
	Vector nn = normalize(n);
	a_ = nn.x;
	b_ = nn.y;
	c_ = nn.z;
	d_ = -(a_*p.x + b_*p.y + c_*p.z);

#ifndef NDEBUG // degenerate case
	if (length(n) < 1e-15) {
		std::cerr << "degenerate plane constructed from point ("
			<< p << ") and normal (" << n << ")" << std::endl;
	}
#endif
}


template <class FT> inline
typename GenericPlane3<FT>::Vector GenericPlane3<FT>::normal() const {
	Vector n = normalize(Vector(a_, b_, c_));

#ifndef NDEBUG // degenerate case
	if (length(n) < 1e-15) {
		std::cerr << "degenerate plane with normal: (" << n << ")" << std::endl;
	}
#endif
	return n;
}


// return values:
//   1: p is on the positive side
//  -1: p is on the negative side
//   0: the point p is and 0 if the point is belonging the plane.
template <class FT> inline
Sign GenericPlane3<FT>::orient(const Point& p) const {
	FT v = value(p);
	if (ogf_abs(v) < 1e-15)
		return ZERO;

	return (v > 0.0 ? POSITIVE : NEGATIVE);
}


template <class FT> inline
typename GenericPlane3<FT>::Point GenericPlane3<FT>::projection(const Point& p) const {
	// the equation of the plane is Ax+By+Cz+D=0
	// the normal direction is (A,B,C)
	// the projected point is p-lambda(A,B,C) where
	// A(x-lambda*A) + B(y-lambda*B) + C(z-lambda*C) + D = 0

	FT num = a_*p.x + b_*p.y + c_*p.z + d_;
	FT den = a_*a_ + b_*b_ + c_*c_;
	FT lambda = num / den;

	FT x = p.x - lambda * a_;
	FT y = p.y - lambda * b_;
	FT z = p.z - lambda * c_;
	return Point(x, y, z);
}


template <class FT> inline
FT GenericPlane3<FT>::squared_ditance(const Point& p) const {
	FT v = value(p);
	return (v * v) / (a_ * a_ + b_ * b_ + c_ * c_);
}


template <class FT> inline
bool GenericPlane3<FT>::intersection(const Line& line, Point& p) const {
	Vector dir = line.direction();
	FT c = dot(dir, normal());
	if (ogf_abs(c) < 1e-15)
		return false;

	Point p0 = line.point();
	// p = p0 + dir * t
	// equation: p is in the plane (so we first compute t)
	FT t = -(a_ * p0.x + b_ * p0.y + c_ * p0.z + d_) / (a_ * dir.x + b_ * dir.y + c_ * dir.z);
	p = p0 + dir * t;
	return true;
}

template <class FT> inline
bool GenericPlane3<FT>::FitToPoints(const std::vector<Point>& Points){
	Point Basis1, Basis2;
	std::vector<vec4> WeightedPoints(Points.size());
	for (unsigned int i = 0; i < Points.size(); i++)
	{
		WeightedPoints[i] = vec4(Points[i][0], Points[i][1], Points[i][2], 1.0f);
	}
	FT NormalEigenvalue;
	return FitToPoints(WeightedPoints, Basis1, Basis2, NormalEigenvalue);
}

template <class FT> inline
bool GenericPlane3<FT>::FitToPoints(const std::vector<vec4>& Points, Point& Basis1, Point& Basis2, FT& NormalEigenvalue){
	Point Centroid, Normal;

	FT ScatterMatrix[3][3];
	int  Order[3];
	FT DiagonalMatrix[3];
	FT OffDiagonalMatrix[3];

	// Find centroid
	Centroid = Point(0.0, 0.0, 0.0);
	FT TotalWeight = 0.0;
	for (unsigned int i = 0; i < Points.size(); i++)
	{
		TotalWeight += Points[i][3];
		Centroid += Point(Points[i][0], Points[i][1], Points[i][2]) * Points[i][3];
	}
	Centroid /= TotalWeight;

	// Compute scatter matrix
	Find_ScatterMatrix(Points, Centroid, ScatterMatrix, Order);

	tred2(ScatterMatrix, DiagonalMatrix, OffDiagonalMatrix);
	tqli(DiagonalMatrix, OffDiagonalMatrix, ScatterMatrix);

	/*
	**    Find the smallest eigenvalue first.
	*/
	FT Min = DiagonalMatrix[0];
	FT Max = DiagonalMatrix[0];
	unsigned int MinIndex = 0;
	unsigned int MiddleIndex = 0;
	unsigned int MaxIndex = 0;
	for (unsigned int i = 1; i < 3; i++)
	{
		if (DiagonalMatrix[i] < Min)
		{
			Min = DiagonalMatrix[i];
			MinIndex = i;
		}
		if (DiagonalMatrix[i] > Max)
		{
			Max = DiagonalMatrix[i];
			MaxIndex = i;
		}
	}
	for (unsigned int i = 0; i < 3; i++)
	{
		if (MinIndex != i && MaxIndex != i)
		{
			MiddleIndex = i;
		}
	}
	/*
	**    The normal of the plane is the smallest eigenvector.
	*/
	for (unsigned int i = 0; i < 3; i++)
	{
		Normal[Order[i]] = ScatterMatrix[i][MinIndex];
		Basis1[Order[i]] = ScatterMatrix[i][MiddleIndex];
		Basis2[Order[i]] = ScatterMatrix[i][MaxIndex];
	}
	NormalEigenvalue = ogf_abs(DiagonalMatrix[MinIndex]);

	*this = GenericPlane3(Centroid, Normal);

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class FT> inline
void GenericPlane3<FT>::Find_ScatterMatrix(const std::vector<vec4> &Points, const Point &Centroid, FT ScatterMatrix[3][3], int Order[3]) const{
	int TempI;
	FT TempD;

	/*    To compute the correct scatter matrix, the centroid must be
	**    subtracted from all points.  If the plane is to be forced to pass
	**    through the origin (0,0,0), then the Centroid was earlier set
	**    equal to (0,0,0).  This, of course, is NOT the true Centroid of
	**    the set of points!  Since the matrix is symmetrical about its
	**    diagonal, one-third of it is redundant and is simply found at
	**    the end.
	*/
	for (unsigned int i = 0; i<3; i++){
		ScatterMatrix[i][0] = ScatterMatrix[i][1] = ScatterMatrix[i][2] = 0;
	}

	for (unsigned int i = 0; i < Points.size(); i++)
	{
		const vec4 &P = Points[i];
		Point d = Point(P[0], P[1], P[2]) - Centroid;
		float Weight = P[3];
		ScatterMatrix[0][0] += d[0]*d[0]*Weight;
		ScatterMatrix[0][1] += d[0]*d[1]*Weight;
		ScatterMatrix[0][2] += d[0]*d[2]*Weight;
		ScatterMatrix[1][1] += d[0]*d[1]*Weight;
		ScatterMatrix[1][2] += d[1]*d[2]*Weight;
		ScatterMatrix[2][2] += d[2]*d[2]*Weight;
	}
	ScatterMatrix[1][0] = ScatterMatrix[0][1];
	ScatterMatrix[2][0] = ScatterMatrix[0][2];
	ScatterMatrix[2][1] = ScatterMatrix[1][2];

	/*    Now, perform a sort of "Matrix-sort", whereby all the larger elements
	**    in the matrix are relocated towards the lower-right portion of the
	**    matrix.  This is done as a requisite of the tred2 and tqli algorithms,
	**    for which the scatter matrix is being computed as an input.
	**    "Order" is a 3 element array that will keep track of the xyz order
	**    in the ScatterMatrix.
	*/
	Order[0] = 0;        /* Beginning order is x-y-z, as found above */
	Order[1] = 1;
	Order[2] = 2;
	if (ScatterMatrix[0][0] > ScatterMatrix[1][1]){
		TempD = ScatterMatrix[0][0];
		ScatterMatrix[0][0] = ScatterMatrix[1][1];
		ScatterMatrix[1][1] = TempD;
		TempD = ScatterMatrix[0][2];
		ScatterMatrix[0][2] = ScatterMatrix[2][0] = ScatterMatrix[1][2];
		ScatterMatrix[1][2] = ScatterMatrix[2][1] = TempD;
		TempI = Order[0];
		Order[0] = Order[1];
		Order[1] = TempI;
	}
	if (ScatterMatrix[1][1] > ScatterMatrix[2][2]){
		TempD = ScatterMatrix[1][1];
		ScatterMatrix[1][1] = ScatterMatrix[2][2];
		ScatterMatrix[2][2] = TempD;
		TempD = ScatterMatrix[0][1];
		ScatterMatrix[0][1] = ScatterMatrix[1][0] = ScatterMatrix[0][2];
		ScatterMatrix[0][2] = ScatterMatrix[2][0] = TempD;
		TempI = Order[1];
		Order[1] = Order[2];
		Order[2] = TempI;
	}
	if (ScatterMatrix[0][0] > ScatterMatrix[1][1]){
		TempD = ScatterMatrix[0][0];
		ScatterMatrix[0][0] = ScatterMatrix[1][1];
		ScatterMatrix[1][1] = TempD;
		TempD = ScatterMatrix[0][2];
		ScatterMatrix[0][2] = ScatterMatrix[2][0] = ScatterMatrix[1][2];
		ScatterMatrix[1][2] = ScatterMatrix[2][1] = TempD;
		TempI = Order[0];
		Order[0] = Order[1];
		Order[1] = TempI;
	}
}

/*
**    This code is taken from ``Numerical Recipes in C'', 2nd
**    and 3rd editions, by Press, Teukolsky, Vetterling and
**    Flannery, Cambridge University Press, 1992, 1994.
**
*/

/*
**    tred2 Householder reduction of a float, symmetric matrix a[1..n][1..n].
**    On output, a is replaced by the orthogonal matrix q effecting the
**    transformation. d[1..n] returns the diagonal elements of the
**    tridiagonal matrix, and e[1..n] the off-diagonal elements, with
**    e[1]=0.
**
**    For my problem, I only need to handle a 3x3 symmetric matrix,
**    so it can be simplified.
**    Therefore n=3.
**
**    Attention: in the book, the index for array starts from 1,
**    but in C, index should start from zero. so I need to modify it.
**    I think it is very simple to modify, just substract 1 from all the
**    index.
*/

template <class FT> inline
FT GenericPlane3<FT>::sign(FT a, FT b) const {
	if (b < 0)
		return -ogf_abs(a);
	else
		return ogf_abs(a);
}

template <class FT> inline
void GenericPlane3<FT>::tred2(FT a[3][3], FT d[3], FT e[3]) const {
	int l, k, i, j;
	FT scale, hh, h, g, f;

	for (i = 3; i >= 2; i--)
	{
		l = i - 1;
		h = scale = 0.0;
		if (l>1)
		{
			for (k = 1; k <= l; k++)
				scale += ogf_abs(a[i - 1][k - 1]);
			if (scale == 0.0)        /* skip transformation */
				e[i - 1] = a[i - 1][l - 1];
			else
			{
				for (k = 1; k <= l; k++)
				{
					a[i - 1][k - 1] /= scale;    /* use scaled a's for transformation. */
					h += a[i - 1][k - 1] * a[i - 1][k - 1];    /* form sigma in h. */
				}
				f = a[i - 1][l - 1];
				g = f>0 ? -std::sqrt(h) : std::sqrt(h);
				e[i - 1] = scale*g;
				h -= f*g;    /* now h is equation (11.2.4) */
				a[i - 1][l - 1] = f - g;    /* store u in the ith row of a. */
				f = 0.0;
				for (j = 1; j <= l; j++)
				{
					a[j - 1][i - 1] = a[i - 1][j - 1] / h; /* store u/H in ith column of a. */
					g = 0.0;    /* form an element of A.u in g */
					for (k = 1; k <= j; k++)
						g += a[j - 1][k - 1] * a[i - 1][k - 1];
					for (k = j + 1; k <= l; k++)
						g += a[k - 1][j - 1] * a[i - 1][k - 1];
					e[j - 1] = g / h; /* form element of p in temorarliy unused element of e. */
					f += e[j - 1] * a[i - 1][j - 1];
				}
				hh = f / (h + h);    /* form K, equation (11.2.11) */
				for (j = 1; j <= l; j++) /* form q and store in e overwriting p. */
				{
					f = a[i - 1][j - 1]; /* Note that e[l]=e[i-1] survives */
					e[j - 1] = g = e[j - 1] - hh*f;
					for (k = 1; k <= j; k++) /* reduce a, equation (11.2.13) */
						a[j - 1][k - 1] -= (f*e[k - 1] + g*a[i - 1][k - 1]);
				}
			}
		}
		else
			e[i - 1] = a[i - 1][l - 1];
		d[i - 1] = h;
	}


	/*
	**    For computing eigenvector.
	*/
	d[0] = 0.0;
	e[0] = 0.0;

	for (i = 1; i <= 3; i++)/* begin accumualting of transfomation matrices */
	{
		l = i - 1;
		if (d[i - 1]) /* this block skipped when i=1 */
		{
			for (j = 1; j <= l; j++)
			{
				g = 0.0;
				for (k = 1; k <= l; k++) /* use u and u/H stored in a to form P.Q */
					g += a[i - 1][k - 1] * a[k - 1][j - 1];
				for (k = 1; k <= l; k++)
					a[k - 1][j - 1] -= g*a[k - 1][i - 1];
			}
		}
		d[i - 1] = a[i - 1][i - 1];
		a[i - 1][i - 1] = 1.0; /* reset row and column of a to identity matrix for next iteration */
		for (j = 1; j <= l; j++)
			a[j - 1][i - 1] = a[i - 1][j - 1] = 0.0;
	}
}



/*
**    QL algo with implicit shift, to determine the eigenvalues and
**    eigenvectors of a float,symmetric  tridiagonal matrix, or of a float,
**    symmetric matrix previously reduced by algo tred2.
**    On input , d[1..n] contains the diagonal elements of the tridiagonal
**    matrix. On output, it returns the eigenvalues. The vector e[1..n]
**    inputs the subdiagonal elements of the tridiagonal matrix, with e[1]
**    arbitrary. On output e is destroyed. If the eigenvectors of a
**    tridiagonal matrix are desired, the matrix z[1..n][1..n] is input
**    as the identity matrix. If the eigenvectors of a matrix that has
**    been reduced by tred2 are required, then z is input as the matrix
**    output by tred2. In either case, the kth column of z returns the
**    normalized eigenvector corresponding to d[k].
**
*/

template <class FT> inline
void GenericPlane3<FT>::tqli(FT d[3], FT e[3], FT z[3][3]) const {
	int        m, l, iter, i, k;
	FT    s, r, p, g, f, dd, c, b;

	for (i = 2; i <= 3; i++)
		e[i - 2] = e[i - 1];    /* convenient to renumber the elements of e */
	e[2] = 0.0;
	for (l = 1; l <= 3; l++)
	{
		iter = 0;
		do
		{
			for (m = l; m <= 2; m++)
			{
				/*
				**    Look for a single small subdiagonal element
				**    to split the matrix.
				*/
				dd = ogf_abs(d[m - 1]) + ogf_abs(d[m]);
				if (ogf_abs(e[m - 1]) + dd == dd)
					break;
			}
			if (m != l)
			{
				if (iter++ == 30)
				{
					printf("\nToo many iterations in TQLI");
				}
				g = (d[l] - d[l - 1]) / (2.0f*e[l - 1]); /* form shift */
				r = std::sqrt((g*g) + 1.0f);
				g = d[m - 1] - d[l - 1] + e[l - 1] / (g + sign(r, g)); /* this is dm-ks */
				s = c = 1.0;
				p = 0.0;
				for (i = m - 1; i >= l; i--)
				{
					/*
					**    A plane rotation as in the original
					**    QL, followed by Givens rotations to
					**    restore tridiagonal form.
					*/
					f = s*e[i - 1];
					b = c*e[i - 1];
					if (ogf_abs(f) >= ogf_abs(g))
					{
						c = g / f;
						r = std::sqrt((c*c) + 1.0f);
						e[i] = f*r;
						c *= (s = 1.0f / r);
					}
					else
					{
						s = f / g;
						r = std::sqrt((s*s) + 1.0f);
						e[i] = g*r;
						s *= (c = 1.0f / r);
					}
					g = d[i] - p;
					r = (d[i - 1] - g)*s + 2.0f*c*b;
					p = s*r;
					d[i] = g + p;
					g = c*r - b;
					for (k = 1; k <= 3; k++)
					{
						/*
						**    Form eigenvectors
						*/
						f = z[k - 1][i];
						z[k - 1][i] = s*z[k - 1][i - 1] + c*f;
						z[k - 1][i - 1] = c*z[k - 1][i - 1] - s*f;
					}
				}
				d[l - 1] = d[l - 1] - p;
				e[l - 1] = g;
				e[m - 1] = 0.0f;
			}
		} while (m != l);
	}
}

#endif
