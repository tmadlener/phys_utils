#ifndef PHYSUTILS_UNBINNEDPOLFW_RANGE_H__
#define PHYSUTILS_UNBINNEDPOLFW_RANGE_H__

/**
 * enum tag specifying the type of range:
 * - TwoSided: upper and lower limit
 * - Upper: only upper limit
 * - Lower: only lower limit
 * - Empty: empty range corresponds to no limits
 */
enum class RangeType {
  TwoSided,
  Upper,
  Lower,
  Empty
};

/**
 * Range class providing a contains function to check if a value is in a given range.
 *
 * Different range types can be specified by providing the appropriate RangeType parameter.
 * Boundaries cannot be changed after construction.
 */
template<RangeType RT>
class Range {
public:

  Range(double min, double max) : m_bounds({min, max})
  {
    static_assert(RT == RangeType::TwoSided, "Only TwoSided ranges have two boundaries");
  }

  Range(double val) : m_bounds({val})
  {
    static_assert(RT == RangeType::Upper || RT == RangeType::Lower,
                  "Only Upper and Lower ranges have one boundary");
  }

  Range() : m_bounds({})
  {
    static_assert(RT == RangeType::Empty, "Can only construct Empty ranges with no boundaries");
  }

  bool contains(double v) const;

private:
  const std::array<double, 2> m_bounds;
};

template<>
bool Range<RangeType::Empty>::contains(double) const
{
  return true;
}

template<>
bool Range<RangeType::Upper>::contains(double v) const
{
  return v < m_bounds[0];
}

template<>
bool Range<RangeType::Lower>::contains(double v) const
{
  return v > m_bounds[0];
}

template<>
bool Range<RangeType::TwoSided>::contains(double v) const
{
  return (v > m_bounds[0] && v < m_bounds[1]);
}

using EmptyRange = Range<RangeType::Empty>;

#endif
