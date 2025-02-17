/*
 * Software License Agreement (BSD License)
 *  Copyright (c) 2015-2019, CNRS - LAAS INRIA
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <cmath>
#include <limits>
#include <hpp/fcl/math/transform.h>
#include <hpp/fcl/shape/geometric_shapes.h>
#include <../src/distance_func_matrix.h>

#define CLAMP(value, l, u) fmax(fmin(value, u), l)

// Note that partial specialization of template functions is not allowed.
// Therefore, two implementations with the default narrow phase solvers are
// provided. If another narrow phase solver were to be used, the default
// template implementation would be called, unless the function is also
// specialized for this new type.
//
// One solution would be to make narrow phase solvers derive from an abstract
// class and specialize the template for this abstract class.
namespace hpp
{
namespace fcl {
  class GJKSolver;


  // Compute the distance between C1 and C2 by computing the distance
  // between the two segments supporting the capsules.
  // Match algorithm of Real-Time Collision Detection, Christer Ericson - Closest Point of Two Line Segments
  template <>
  FCL_REAL ShapeShapeDistance <Capsule, Capsule> (const CollisionGeometry* o1, const Transform3f& tf1,
   const CollisionGeometry* o2, const Transform3f& tf2,
   const GJKSolver*, const DistanceRequest& request,
   DistanceResult& result)
  {
    const Capsule* capsule1 = static_cast <const Capsule*> (o1);
    const Capsule* capsule2 = static_cast <const Capsule*> (o2);

    FCL_REAL EPSILON = std::numeric_limits<FCL_REAL>::epsilon () * 100;

    // We assume that capsules are centered at the origin.
    const fcl::Vec3f& c1 = tf1.getTranslation ();
    const fcl::Vec3f& c2 = tf2.getTranslation ();
    // We assume that capsules are oriented along z-axis.
    FCL_REAL halfLength1 = capsule1->halfLength;
    FCL_REAL halfLength2 = capsule2->halfLength;
    FCL_REAL radius1 = capsule1->radius;
    FCL_REAL radius2 = capsule2->radius;
    // direction of capsules
    // ||d1|| = 2 * halfLength1
    const fcl::Vec3f d1 = 2 * halfLength1 * tf1.getRotation().col(2);
    const fcl::Vec3f d2 = 2 * halfLength2 * tf2.getRotation().col(2);

    // Starting point of the segments
    // p1 + d1 is the end point of the segment
    const fcl::Vec3f p1 = c1 - d1 / 2;
    const fcl::Vec3f p2 = c2 - d2 / 2;
    const fcl::Vec3f r = p1-p2;
    FCL_REAL a = d1.dot(d1);
    FCL_REAL b = d1.dot(d2);
    FCL_REAL c = d1.dot(r);
    FCL_REAL e = d2.dot(d2);
    FCL_REAL f = d2.dot(r);
    // S1 is parametrized by the equation p1 + s * d1
    // S2 is parametrized by the equation p2 + t * d2
    FCL_REAL s = 0.0;
    FCL_REAL t = 0.0;

    if (a <= EPSILON && e <= EPSILON)
    {
      // Check if the segments degenerate into points
      s = t = 0.0;
    }
    else if (a <= EPSILON)
    {
      // First segment is degenerated
      s = 0.0;
      t = CLAMP(f / e, 0.0, 1.0);
    }
    else if (e <= EPSILON)
    {
      // Second segment is degenerated
      s = CLAMP(-c / a, 0.0, 1.0);
      t = 0.0;
    }
    else
    {
      // Always non-negative, equal 0 if the segments are colinear
      FCL_REAL denom = fmax(a*e-b*b, 0);

      if (denom > EPSILON)
      {
        s = CLAMP((b*f-c*e) / denom, 0.0, 1.0);
      }
      else
      {
        s = 0.0;
      }

      t = (b*s + f) / e;
      if (t < 0.0)
      {
        t = 0.0;
        s = CLAMP(-c / a, 0.0, 1.0);
      }
      else if (t > 1.0)
      {
        t = 1.0;
        s = CLAMP((b - c)/a, 0.0, 1.0);
      }
    }

    // witness points achieving the distance between the two segments
    const Vec3f w1 = p1 + s * d1;
    const Vec3f w2 = p2 + t * d2;
    FCL_REAL distance = (w1 - w2).norm();
    Vec3f normal = (w1 - w2) / distance;
    result.normal = normal;

    // capsule spcecific distance computation
    distance = distance - (radius1 + radius2);
    result.min_distance = distance;
    // witness points for the capsules
    if (request.enable_nearest_points)
    {
      result.nearest_points[0] = w1 - radius1 * normal;
      result.nearest_points[1] = w2 + radius2 * normal;
    }
    return distance;
  }

  template <>
  std::size_t ShapeShapeCollide <Capsule, Capsule>
  (const CollisionGeometry* o1, const Transform3f& tf1,
   const CollisionGeometry* o2, const Transform3f& tf2,
   const GJKSolver*, const CollisionRequest& request,
   CollisionResult& result)
  {
    GJKSolver* unused = 0x0;
    DistanceResult distanceResult;
    DistanceRequest distanceRequest (request.enable_contact);

    FCL_REAL distance = ShapeShapeDistance <Capsule, Capsule>
      (o1, tf1, o2, tf2, unused, distanceRequest, distanceResult);

    if (distance > 0)
    {
      return 0;
    }
    else
    {
      Contact contact (o1, o2, -1, -1);
      const Vec3f& p1 = distanceResult.nearest_points [0];
      const Vec3f& p2 = distanceResult.nearest_points [1];
      contact.pos = 0.5 * (p1+p2);
      contact.normal = distanceResult.normal;
      result.addContact(contact);
      return 1;
    }
  }

} // namespace fcl

} // namespace hpp
