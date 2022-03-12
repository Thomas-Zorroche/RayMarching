#pragma once

#include "c3ga/Mvec.hpp"
#include "glm/glm.hpp"

/// \brief build a point from a vector
/// \param x vector component related to e1
/// \param y vector component related to e2
/// \param z vector component related to e3
/// \return a multivector corresponding to a point p = e0 +  x e1 + y e2 + z e3 + 0.5 || (x e1 + y e2 + z e3) ||^2 einf
template<typename T>
c3ga::Mvec<T> point(const T& x, const T& y, const T& z) {

    c3ga::Mvec<T> mv;
    mv[c3ga::E1] = x;
    mv[c3ga::E2] = y;
    mv[c3ga::E3] = z;
    mv[c3ga::Ei] = 0.5 * mv.quadraticNorm();
    mv[c3ga::E0] = 1.0;

    return mv;
}

/// \brief build a point from a vector
/// \param vec is should be a multivector of grade 1 vec = v1 e1 + v2 e2 + v3 e3. If vec has other components, they will be ignored during the execution.
/// \return a multivector corresponding to a point p = e0 + v1 e1 + v2 e2 + v3 e3 + 0.5 || vec ||^2 einf
template<typename T>
c3ga::Mvec<T> point(const c3ga::Mvec<T>& vec) {
    return point(vec[c3ga::E1], vec[c3ga::E2], vec[c3ga::E3]);
}

/// \brief build a dual sphere from a center and a radius
/// \param centerX dual sphere center component related to e1
/// \param centerY dual sphere center component related to e2
/// \param centerZ dual sphere center component related to e3
/// \param radius of the sphere
/// \return a multivector corresponding to a dual sphere s = center - 0.5 radius ei, with center being e0 +  x e1 + y e2 + z e3 + 0.5 || (x e1 + y e2 + z e3) ||^2 einf.
template<typename T>
c3ga::Mvec<T> dualSphere(const T& centerX, const T& centerY, const T& centerZ, const T& radius) {
    c3ga::Mvec<T> dualSphere = point(centerX, centerY, centerZ);
    dualSphere[c3ga::Ei] -= 0.5 * radius;
    return dualSphere;
}

// Get the distance between two multivectors
template<typename T>
float distance(const c2ga::Mvec<T>& p1, const c2ga::Mvec<T>& p2) {
	return (p2 - p1).norm();
}