#ifndef PHYSUTILS_UNBINNEDPOLFW_ANGULARAPARAMETERS_H__
#define PHYSUTILS_UNBINNEDPOLFW_ANGULARAPARAMETERS_H__

#include <array>

/**
 * Class fro parametrizing a polynomial dependence of a parameter, when the support points
 * and the values at these support points are known.
 * Tempalted in the number of used parameters; N - 1 is the degree of the used polynomial.
 *
 * Support points have to be known at construction, values at support points can be changed
 * later on.
 *
 * The maximum degree currently is parabolic (higher won't compile and/or link)
 */
template<size_t N>
class Parametrization {
public:
  /** constructor, need only supports for constructor. */
  Parametrization(const std::array<double, N>& sup) : m_sup(sup) {}

  /** set the values at the support points for a given parametrization. */
  void setVals(const std::array<double, N>& val) { m_val = val; }

  /** evaluate the parametrization at the given point. (Implemented up to N = 3) */
  double eval(const double) const;

  const std::array<double, N>& getVal() const { return m_val; }

  const std::array<double, N>& getSupport() const { return m_sup; }

private:
  const std::array<double, N> m_sup; /**< support points. */
  std::array<double, N> m_val; /**< values at support points. */
};


// eval spezializations
// for an "empty" parametrization return zero always
template<>
double Parametrization<0>::eval(const double) const { return 0; }

template<>
double Parametrization<1>::eval(const double) const { return m_val[0]; }

template<>
double Parametrization<2>::eval(const double x) const
{
  // linear parametrization y = y0 * (x - x1)/(x0 - x1) + y1 * (x - x0)/(x1 - x0)
  // implemented as (equivalent): (y0 * (x1 - x) + y1 * (x - x0)) / (x1 - x0)
  return (m_val[0] * (m_sup[1] - x) + m_val[1] * (x - m_sup[0])) / (m_sup[1] - m_sup[0]);
}

template<>
double Parametrization<3>::eval(const double x) const
{
  // parabolic parametrization, implemented as:
  // y = (y0*(x-x1)*(x-x2)*(x1-x2) + y1*(x-x0)*(x-x2)*(x2-x0) + y2*(x-x0)*(x-x1)*(x0-x1)) / N
  // where N = (x0-x1)*(x0-x2)*(x1-x2)

  const double ds01 = m_sup[0] - m_sup[1];
  const double ds02 = m_sup[0] - m_sup[2];
  const double ds12 = m_sup[1] - m_sup[2];
  const double N = ds01 * ds02 * ds12;
  const double dx0 = x - m_sup[0];
  const double dx1 = x - m_sup[1];
  const double dx2 = x - m_sup[2];

  return (m_val[0] * dx1 * dx2 * ds12 - m_val[1] * dx0 * dx2 * ds02 +
          m_val[2] * dx0 * dx1 * ds01) / N;
}

/** small helper struct. */
struct AngularParameters {
  const double L;
  const double phi;
  const double tp;
};

/**
 * Parametrization of the three angular polarization parameters.
 * Basically a wrapper class to encapsulate three parametrizations at once.
 */
template<size_t N, size_t M, size_t P>
class AngularParametrization {
public:
  /** constructor needs only constant support points. */
  AngularParametrization(const std::array<double, N>& AL,
                         const std::array<double, M>& Aphi,
                         const std::array<double, P>& Atp) :
    m_AL(AL), m_Aphi(Aphi), m_Atp(Atp) {}

  /** set the values at the support points. */
  void setVals(const std::array<double, N>& AL, const std::array<double, M>& Aphi,
               const std::array<double, P>& Atp);

  /** evaluate the parametrization at a given point. */
  AngularParameters eval(const double x) const;

  const std::array<double, N>& getValAL() const { return m_AL.getVal(); }
  const std::array<double, M>& getValAphi() const { return m_Aphi.getVal(); }
  const std::array<double, P>& getValAtp() const { return m_Atp.getVal(); }

  const std::array<double, N>& getSupAL() const { return m_AL.getSupport(); }
  const std::array<double, M>& getSupAphi() const { return m_Aphi.getSupport(); }
  const std::array<double, P>& getSupAtp() const { return m_Atp.getSupport(); }

private:
  Parametrization<N> m_AL;
  Parametrization<M> m_Aphi;
  Parametrization<P> m_Atp;
};

template<size_t N, size_t M, size_t P>
AngularParameters AngularParametrization<N,M,P>::eval(const double x) const
{
  return AngularParameters{m_AL.eval(x), m_Aphi.eval(x), m_Atp.eval(x)};
}

template<size_t N, size_t M, size_t P>
void AngularParametrization<N,M,P>::setVals(const std::array<double, N>& AL,
                                            const std::array<double, M>& Aphi,
                                            const std::array<double, P>& Atp)
{
  m_AL.setVals(AL);
  m_Aphi.setVals(Aphi);
  m_Atp.setVals(Atp);
}

#endif
