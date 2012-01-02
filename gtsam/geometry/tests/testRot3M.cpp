/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testRot3.cpp
 * @brief   Unit tests for Rot3M class
 * @author  Alireza Fathi
 */

#include <CppUnitLite/TestHarness.h>
#include <gtsam/base/Testable.h>
#include <boost/math/constants/constants.hpp>
#include <gtsam/base/numericalDerivative.h>
#include <gtsam/base/lieProxies.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/Rot3.h>

using namespace gtsam;

Rot3M R = Rot3M::rodriguez(0.1, 0.4, 0.2);
Point3 P(0.2, 0.7, -2.0);
double error = 1e-9, epsilon = 0.001;

/* ************************************************************************* */
TEST( Rot3M, constructor)
{
	Rot3M expected(eye(3, 3));
	Vector r1(3), r2(3), r3(3);
	r1(0) = 1;
	r1(1) = 0;
	r1(2) = 0;
	r2(0) = 0;
	r2(1) = 1;
	r2(2) = 0;
	r3(0) = 0;
	r3(1) = 0;
	r3(2) = 1;
	Rot3M actual(r1, r2, r3);
	CHECK(assert_equal(actual,expected));
}

/* ************************************************************************* */
TEST( Rot3M, constructor2)
{
	Matrix R = Matrix_(3, 3, 11., 12., 13., 21., 22., 23., 31., 32., 33.);
	Rot3M actual(R);
	Rot3M expected(11, 12, 13, 21, 22, 23, 31, 32, 33);
	CHECK(assert_equal(actual,expected));
}

/* ************************************************************************* */
TEST( Rot3M, constructor3)
{
	Rot3M expected(1, 2, 3, 4, 5, 6, 7, 8, 9);
	Point3 r1(1, 4, 7), r2(2, 5, 8), r3(3, 6, 9);
	CHECK(assert_equal(Rot3M(r1,r2,r3),expected));
}

/* ************************************************************************* */
TEST( Rot3M, transpose)
{
	Rot3M R(1, 2, 3, 4, 5, 6, 7, 8, 9);
	Point3 r1(1, 2, 3), r2(4, 5, 6), r3(7, 8, 9);
	CHECK(assert_equal(R.inverse(),Rot3M(r1,r2,r3)));
}

/* ************************************************************************* */
TEST( Rot3M, equals)
{
	CHECK(R.equals(R));
	Rot3M zero;
	CHECK(!R.equals(zero));
}

/* ************************************************************************* */
// Notice this uses J^2 whereas fast uses w*w', and has cos(t)*I + ....
Rot3M slow_but_correct_rodriguez(const Vector& w) {
	double t = norm_2(w);
	Matrix J = skewSymmetric(w / t);
	if (t < 1e-5) return Rot3M();
	Matrix R = eye(3) + sin(t) * J + (1.0 - cos(t)) * (J * J);
	return R;
}

/* ************************************************************************* */
TEST( Rot3M, rodriguez)
{
	Rot3M R1 = Rot3M::rodriguez(epsilon, 0, 0);
	Vector w = Vector_(3, epsilon, 0., 0.);
	Rot3M R2 = slow_but_correct_rodriguez(w);
	CHECK(assert_equal(R2,R1));
}

/* ************************************************************************* */
TEST( Rot3M, rodriguez2)
{
	Vector axis = Vector_(3,0.,1.,0.); // rotation around Y
	double angle = 3.14 / 4.0;
	Rot3M actual = Rot3M::rodriguez(axis, angle);
	Rot3M expected(0.707388, 0, 0.706825,
			                 0, 1,        0,
			         -0.706825, 0, 0.707388);
	CHECK(assert_equal(expected,actual,1e-5));
}

/* ************************************************************************* */
TEST( Rot3M, rodriguez3)
{
	Vector w = Vector_(3, 0.1, 0.2, 0.3);
	Rot3M R1 = Rot3M::rodriguez(w / norm_2(w), norm_2(w));
	Rot3M R2 = slow_but_correct_rodriguez(w);
	CHECK(assert_equal(R2,R1));
}

/* ************************************************************************* */
TEST( Rot3M, rodriguez4)
{
	Vector axis = Vector_(3,0.,0.,1.); // rotation around Z
	double angle = M_PI_2;
	Rot3M actual = Rot3M::rodriguez(axis, angle);
	double c=cos(angle),s=sin(angle);
	Rot3M expected(c,-s, 0,
			          s, c, 0,
			          0, 0, 1);
	CHECK(assert_equal(expected,actual,1e-5));
	CHECK(assert_equal(slow_but_correct_rodriguez(axis*angle),actual,1e-5));
}

/* ************************************************************************* */
TEST( Rot3M, expmap)
{
	Vector v = zero(3);
	CHECK(assert_equal(R.retract(v), R));
}

/* ************************************************************************* */
TEST(Rot3M, log)
{
	Vector w1 = Vector_(3, 0.1, 0.0, 0.0);
	Rot3M R1 = Rot3M::rodriguez(w1);
	CHECK(assert_equal(w1, Rot3M::Logmap(R1)));

	Vector w2 = Vector_(3, 0.0, 0.1, 0.0);
	Rot3M R2 = Rot3M::rodriguez(w2);
	CHECK(assert_equal(w2, Rot3M::Logmap(R2)));

	Vector w3 = Vector_(3, 0.0, 0.0, 0.1);
	Rot3M R3 = Rot3M::rodriguez(w3);
	CHECK(assert_equal(w3, Rot3M::Logmap(R3)));

	Vector w = Vector_(3, 0.1, 0.4, 0.2);
	Rot3M R = Rot3M::rodriguez(w);
	CHECK(assert_equal(w, Rot3M::Logmap(R)));

	Vector w5 = Vector_(3, 0.0, 0.0, 0.0);
	Rot3M R5 = Rot3M::rodriguez(w5);
	CHECK(assert_equal(w5, Rot3M::Logmap(R5)));

	Vector w6 = Vector_(3, boost::math::constants::pi<double>(), 0.0, 0.0);
	Rot3M R6 = Rot3M::rodriguez(w6);
	CHECK(assert_equal(w6, Rot3M::Logmap(R6)));

	Vector w7 = Vector_(3, 0.0, boost::math::constants::pi<double>(), 0.0);
	Rot3M R7 = Rot3M::rodriguez(w7);
	CHECK(assert_equal(w7, Rot3M::Logmap(R7)));

	Vector w8 = Vector_(3, 0.0, 0.0, boost::math::constants::pi<double>());
	Rot3M R8 = Rot3M::rodriguez(w8);
	CHECK(assert_equal(w8, Rot3M::Logmap(R8)));
}

/* ************************************************************************* */
TEST(Rot3M, manifold)
{
	Rot3M gR1 = Rot3M::rodriguez(0.1, 0.4, 0.2);
	Rot3M gR2 = Rot3M::rodriguez(0.3, 0.1, 0.7);
	Rot3M origin;

	// log behaves correctly
	Vector d12 = gR1.localCoordinates(gR2);
	CHECK(assert_equal(gR2, gR1.retract(d12)));
	CHECK(assert_equal(gR2, gR1*Rot3M::Expmap(d12)));
	Vector d21 = gR2.localCoordinates(gR1);
	CHECK(assert_equal(gR1, gR2.retract(d21)));
	CHECK(assert_equal(gR1, gR2*Rot3M::Expmap(d21)));

	// Check that log(t1,t2)=-log(t2,t1)
	CHECK(assert_equal(d12,-d21));

	// lines in canonical coordinates correspond to Abelian subgroups in SO(3)
	Vector d = Vector_(3, 0.1, 0.2, 0.3);
	// exp(-d)=inverse(exp(d))
	CHECK(assert_equal(Rot3M::Expmap(-d),Rot3M::Expmap(d).inverse()));
	// exp(5d)=exp(2*d+3*d)=exp(2*d)exp(3*d)=exp(3*d)exp(2*d)
	Rot3M R2 = Rot3M::Expmap (2 * d);
	Rot3M R3 = Rot3M::Expmap (3 * d);
	Rot3M R5 = Rot3M::Expmap (5 * d);
	CHECK(assert_equal(R5,R2*R3));
	CHECK(assert_equal(R5,R3*R2));
}

/* ************************************************************************* */
class AngularVelocity: public Point3 {
public:
	AngularVelocity(const Point3& p) :
		Point3(p) {
	}
	AngularVelocity(double wx, double wy, double wz) :
		Point3(wx, wy, wz) {
	}
};

AngularVelocity bracket(const AngularVelocity& X, const AngularVelocity& Y) {
	return X.cross(Y);
}

/* ************************************************************************* */
TEST(Rot3M, BCH)
{
	// Approximate exmap by BCH formula
	AngularVelocity w1(0.2, -0.1, 0.1);
	AngularVelocity w2(0.01, 0.02, -0.03);
	Rot3M R1 = Rot3M::Expmap (w1.vector()), R2 = Rot3M::Expmap (w2.vector());
	Rot3M R3 = R1 * R2;
	Vector expected = Rot3M::Logmap(R3);
	Vector actual = BCH(w1, w2).vector();
	CHECK(assert_equal(expected, actual,1e-5));
}

/* ************************************************************************* */
TEST( Rot3M, rotate_derivatives)
{
	Matrix actualDrotate1a, actualDrotate1b, actualDrotate2;
	R.rotate(P, actualDrotate1a, actualDrotate2);
	R.inverse().rotate(P, actualDrotate1b, boost::none);
	Matrix numerical1 = numericalDerivative21(testing::rotate<Rot3M,Point3>, R, P);
	Matrix numerical2 = numericalDerivative21(testing::rotate<Rot3M,Point3>, R.inverse(), P);
	Matrix numerical3 = numericalDerivative22(testing::rotate<Rot3M,Point3>, R, P);
	EXPECT(assert_equal(numerical1,actualDrotate1a,error));
	EXPECT(assert_equal(numerical2,actualDrotate1b,error));
	EXPECT(assert_equal(numerical3,actualDrotate2, error));
}

/* ************************************************************************* */
TEST( Rot3M, unrotate)
{
	Point3 w = R * P;
	Matrix H1,H2;
	Point3 actual = R.unrotate(w,H1,H2);
	CHECK(assert_equal(P,actual));

	Matrix numerical1 = numericalDerivative21(testing::unrotate<Rot3M,Point3>, R, w);
	CHECK(assert_equal(numerical1,H1,error));

	Matrix numerical2 = numericalDerivative22(testing::unrotate<Rot3M,Point3>, R, w);
	CHECK(assert_equal(numerical2,H2,error));
}

/* ************************************************************************* */
TEST( Rot3M, compose )
{
	Rot3M R1 = Rot3M::rodriguez(0.1, 0.2, 0.3);
	Rot3M R2 = Rot3M::rodriguez(0.2, 0.3, 0.5);

	Rot3M expected = R1 * R2;
	Matrix actualH1, actualH2;
	Rot3M actual = R1.compose(R2, actualH1, actualH2);
	CHECK(assert_equal(expected,actual));

	Matrix numericalH1 = numericalDerivative21(testing::compose<Rot3M>, R1,
			R2, 1e-2);
	CHECK(assert_equal(numericalH1,actualH1));

	Matrix numericalH2 = numericalDerivative22(testing::compose<Rot3M>, R1,
			R2, 1e-2);
	CHECK(assert_equal(numericalH2,actualH2));
}

/* ************************************************************************* */
TEST( Rot3M, inverse )
{
	Rot3M R = Rot3M::rodriguez(0.1, 0.2, 0.3);

	Rot3M I;
	Matrix actualH;
	CHECK(assert_equal(I,R*R.inverse(actualH)));
	CHECK(assert_equal(I,R.inverse()*R));

	Matrix numericalH = numericalDerivative11(testing::inverse<Rot3M>, R);
	CHECK(assert_equal(numericalH,actualH));
}

/* ************************************************************************* */
TEST( Rot3M, between )
{
	Rot3M R = Rot3M::rodriguez(0.1, 0.4, 0.2);
	Rot3M origin;
	CHECK(assert_equal(R, origin.between(R)));
	CHECK(assert_equal(R.inverse(), R.between(origin)));

	Rot3M R1 = Rot3M::rodriguez(0.1, 0.2, 0.3);
	Rot3M R2 = Rot3M::rodriguez(0.2, 0.3, 0.5);

	Rot3M expected = R1.inverse() * R2;
	Matrix actualH1, actualH2;
	Rot3M actual = R1.between(R2, actualH1, actualH2);
	CHECK(assert_equal(expected,actual));

	Matrix numericalH1 = numericalDerivative21(testing::between<Rot3M> , R1, R2);
	CHECK(assert_equal(numericalH1,actualH1));

	Matrix numericalH2 = numericalDerivative22(testing::between<Rot3M> , R1, R2);
	CHECK(assert_equal(numericalH2,actualH2));
}

/* ************************************************************************* */
TEST( Rot3M, xyz )
{
	double t = 0.1, st = sin(t), ct = cos(t);

	// Make sure all counterclockwise
	// Diagrams below are all from from unchanging axis

	// z
	// |   * Y=(ct,st)
	// x----y
	Rot3M expected1(1, 0, 0, 0, ct, -st, 0, st, ct);
	CHECK(assert_equal(expected1,Rot3M::Rx(t)));

	// x
	// |   * Z=(ct,st)
	// y----z
	Rot3M expected2(ct, 0, st, 0, 1, 0, -st, 0, ct);
	CHECK(assert_equal(expected2,Rot3M::Ry(t)));

	// y
	// |   X=* (ct,st)
	// z----x
	Rot3M expected3(ct, -st, 0, st, ct, 0, 0, 0, 1);
	CHECK(assert_equal(expected3,Rot3M::Rz(t)));

	// Check compound rotation
	Rot3M expected = Rot3M::Rz(0.3) * Rot3M::Ry(0.2) * Rot3M::Rx(0.1);
	CHECK(assert_equal(expected,Rot3M::RzRyRx(0.1,0.2,0.3)));
}

/* ************************************************************************* */
TEST( Rot3M, yaw_pitch_roll )
{
	double t = 0.1;

	// yaw is around z axis
	CHECK(assert_equal(Rot3M::Rz(t),Rot3M::yaw(t)));

	// pitch is around y axis
	CHECK(assert_equal(Rot3M::Ry(t),Rot3M::pitch(t)));

	// roll is around x axis
	CHECK(assert_equal(Rot3M::Rx(t),Rot3M::roll(t)));

	// Check compound rotation
	Rot3M expected = Rot3M::yaw(0.1) * Rot3M::pitch(0.2) * Rot3M::roll(0.3);
	CHECK(assert_equal(expected,Rot3M::ypr(0.1,0.2,0.3)));
}

/* ************************************************************************* */
TEST( Rot3M, RQ)
{
	// Try RQ on a pure rotation
	Matrix actualK;
	Vector actual;
	boost::tie(actualK, actual) = RQ(R.matrix());
	Vector expected = Vector_(3, 0.14715, 0.385821, 0.231671);
	CHECK(assert_equal(eye(3),actualK));
	CHECK(assert_equal(expected,actual,1e-6));

	// Try using xyz call, asserting that Rot3M::RzRyRx(x,y,z).xyz()==[x;y;z]
	CHECK(assert_equal(expected,R.xyz(),1e-6));
	CHECK(assert_equal(Vector_(3,0.1,0.2,0.3),Rot3M::RzRyRx(0.1,0.2,0.3).xyz()));

	// Try using ypr call, asserting that Rot3M::ypr(y,p,r).ypr()==[y;p;r]
	CHECK(assert_equal(Vector_(3,0.1,0.2,0.3),Rot3M::ypr(0.1,0.2,0.3).ypr()));
	CHECK(assert_equal(Vector_(3,0.3,0.2,0.1),Rot3M::ypr(0.1,0.2,0.3).rpy()));

	// Try ypr for pure yaw-pitch-roll matrices
	CHECK(assert_equal(Vector_(3,0.1,0.0,0.0),Rot3M::yaw (0.1).ypr()));
	CHECK(assert_equal(Vector_(3,0.0,0.1,0.0),Rot3M::pitch(0.1).ypr()));
	CHECK(assert_equal(Vector_(3,0.0,0.0,0.1),Rot3M::roll (0.1).ypr()));

	// Try RQ to recover calibration from 3*3 sub-block of projection matrix
	Matrix K = Matrix_(3, 3, 500.0, 0.0, 320.0, 0.0, 500.0, 240.0, 0.0, 0.0, 1.0);
	Matrix A = K * R.matrix();
	boost::tie(actualK, actual) = RQ(A);
	CHECK(assert_equal(K,actualK));
	CHECK(assert_equal(expected,actual,1e-6));
}

/* ************************************************************************* */
TEST( Rot3M, expmapStability ) {
  Vector w = Vector_(3, 78e-9, 5e-8, 97e-7);
  double theta = w.norm();
  double theta2 = theta*theta;
  Rot3M actualR = Rot3M::Expmap(w);
  Matrix W = Matrix_(3,3, 0.0, -w(2), w(1),
                          w(2), 0.0, -w(0),
                          -w(1), w(0), 0.0 );
  Matrix W2 = W*W;
  Matrix Rmat = eye(3) + (1.0-theta2/6.0 + theta2*theta2/120.0
      - theta2*theta2*theta2/5040.0)*W + (0.5 - theta2/24.0 + theta2*theta2/720.0)*W2 ;
  Rot3M expectedR( Rmat );
  CHECK(assert_equal(expectedR, actualR, 1e-10));
}

/* ************************************************************************* */
TEST( Rot3M, logmapStability ) {
  Vector w = Vector_(3, 1e-8, 0.0, 0.0);
  Rot3M R = Rot3M::Expmap(w);
//  double tr = R.r1().x()+R.r2().y()+R.r3().z();
//  std::cout.precision(5000);
//  std::cout << "theta: " << w.norm() << std::endl;
//  std::cout << "trace: " << tr << std::endl;
//  R.print("R = ");
  Vector actualw = Rot3M::Logmap(R);
  CHECK(assert_equal(w, actualw, 1e-15));
}

/* ************************************************************************* */
TEST(Rot3M, quaternion) {
  // NOTE: This is also verifying the ability to convert Vector to Quaternion
  Quaternion q1(0.710997408193224, 0.360544029310185, 0.594459869568306, 0.105395217842782);
  Rot3M R1 = Rot3M(Matrix_(3,3,
      0.271018623057411,   0.278786459830371,   0.921318086098018,
      0.578529366719085,   0.717799701969298,  -0.387385285854279,
     -0.769319620053772,   0.637998195662053,   0.033250932803219));

  Quaternion q2(0.263360579192421, 0.571813128030932, 0.494678363680335, 0.599136268678053);
  Rot3M R2 = Rot3M(Matrix_(3,3,
      -0.207341903877828,   0.250149415542075,   0.945745528564780,
       0.881304914479026,  -0.371869043667957,   0.291573424846290,
       0.424630407073532,   0.893945571198514,  -0.143353873763946));

  // Check creating Rot3M from quaternion
  EXPECT(assert_equal(R1, Rot3M(q1)));
  EXPECT(assert_equal(R1, Rot3M::quaternion(q1.w(), q1.x(), q1.y(), q1.z())));
  EXPECT(assert_equal(R2, Rot3M(q2)));
  EXPECT(assert_equal(R2, Rot3M::quaternion(q2.w(), q2.x(), q2.y(), q2.z())));

  // Check converting Rot3M to quaterion
  EXPECT(assert_equal(Vector(R1.toQuaternion().coeffs()), Vector(q1.coeffs())));
  EXPECT(assert_equal(Vector(R2.toQuaternion().coeffs()), Vector(q2.coeffs())));

  // Check that quaternion and Rot3M represent the same rotation
  Point3 p1(1.0, 2.0, 3.0);
  Point3 p2(8.0, 7.0, 9.0);

  Point3 expected1 = R1*p1;
  Point3 expected2 = R2*p2;

  Point3 actual1 = Point3(q1*p1.vector());
  Point3 actual2 = Point3(q2*p2.vector());

  EXPECT(assert_equal(expected1, actual1));
  EXPECT(assert_equal(expected2, actual2));
}

/* ************************************************************************* */
int main() {
	TestResult tr;
	return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */

