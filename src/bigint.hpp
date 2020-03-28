#ifndef BIGINT_HPP
#define BIGINT_HPP
#include <complex>
#include <cstring>
#include <iostream>
#include <random>
#include <utility>
namespace calc {

// IntT should be unsigned int,
// and twice of length should be representable by basic type.
// To avoid waste of time,
// self-modifying functions should ensure the length is reset.
// A shorter limb may cause faster add/sub,
// but will harm the maximum length that a fast multiplication can handle with.
template <typename IntT = uint16_t>
class BigInt {
   public:
    bool is_signed_ = true;
    // constructors
    explicit BigInt(int value = 0);
    // copy constructor
    BigInt(const BigInt& rhs);
    // move constructor
    BigInt(BigInt&& rhs) noexcept;
    // construct by \0 terminated c-style string, base=0 is auto-detect
    explicit BigInt(const char* str, size_t base = 0);
    explicit BigInt(const std::string& str, size_t base = 0)
        : BigInt(str.c_str(), base) {}

    // destructor
    virtual ~BigInt();

    // assignment & type conversion operators
    // copy assignment, *only copy the value*
    BigInt& operator=(const BigInt& rhs);
    // move assignment, with possible memory reuse
    BigInt& operator=(BigInt&& rhs) noexcept;
    BigInt& operator=(const char* str);
    BigInt& operator=(const std::string& str);
    explicit operator bool() const;
    explicit operator std::string() const;
    std::string ToString(size_t base, bool uppercase = false,
                         int showbase = 0) const;

    // basic operations
    bool Sign() const;
    // shrink the size allocated to *a power of 2*
    // @return: capacity after shrink
    size_t Shrink();
    // the sign will be preserved, so len_ may be seg_len+1 if bit_len%LIMB==0
    BigInt& CutLen(size_t seg_len, size_t bit_len = 0);
    BigInt& CutBit(size_t bitlen);
    // assign non-negative random value
    // @param: length=0 means length=this->len_
    // @param: Unless 0, ensure the bitlen of the highest segment is fixed%LIMB
    //         so fixed==LIMB will always generate a negative value
    BigInt& GenRandom(size_t length = 0, size_t fixed = 0);
    const IntT* Data() const;
    size_t Length() const;

    // bit arithmetic
    BigInt& ToBitInv();  // modifying version of ~a
    BigInt operator~() const;
    BigInt& operator&=(const BigInt& rhs);
    BigInt& operator|=(const BigInt& rhs);
    BigInt& operator^=(const BigInt& rhs);
    BigInt& operator<<=(size_t rhs);
    BigInt& operator>>=(size_t rhs);

    // comparison
#ifdef __cpp_impl_three_way_comparison
    std::weak_ordering operator<=>(const BigInt& rhs) const;
#else
    int Compare(const BigInt& rhs) const;
#endif

    // operator +=,-=,++,--
    BigInt& operator+=(IntT rhs);
    BigInt& operator+=(const BigInt& rhs);
    BigInt& operator-=(const BigInt& rhs);
    BigInt& operator++();  // ++i
    BigInt& operator--();
    BigInt operator++(int);  // i++
    BigInt operator--(int);
    BigInt& ToOpposite();  // modifying version of -a
    BigInt operator-() const;
    BigInt& ToAbsolute();

    // operator *=,/=,%=
    BigInt& operator*=(IntT rhs);
    BigInt& operator*=(const BigInt& rhs);
    BigInt& operator/=(IntT rhs);
    BigInt& operator/=(const BigInt& rhs);
    BigInt& operator%=(IntT rhs);
    BigInt& operator%=(const BigInt& rhs);
    BigInt& BasicDivEq(IntT rhs, IntT* mod = nullptr);
    BigInt& PlainMulEq(const BigInt& rhs);
    BigInt& FFTMulEq(const BigInt& rhs);
    BigInt& PlainDivEq(const BigInt& rhs, BigInt* mod = nullptr);
    BigInt& DivEqAlgA(const BigInt& rhs, BigInt* mod = nullptr);
    BigInt& DivEqAlgB(const BigInt& rhs, BigInt* mod = nullptr);

    // non-modifying
    static BigInt PlainMul(BigInt lhs, const BigInt& rhs);
    static BigInt FFTMul(BigInt lhs, const BigInt& rhs);
    static BigInt BasicDiv(BigInt lhs, IntT rhs, IntT* mod = nullptr);
    static BigInt PlainDiv(BigInt lhs, const BigInt& rhs,
                           BigInt* mod = nullptr);
    static BigInt DivAlgA(BigInt lhs, const BigInt& rhs, BigInt* mod = nullptr);
    static BigInt DivAlgB(BigInt lhs, const BigInt& rhs, BigInt* mod = nullptr);

    // I/O
    // currently accept 2<=base<=36, other value will be 10
    //@param: showbase: 0) don't show, 1) 0xabc, 0XA0B, 1a_12, 2) abc_16
    void Print(size_t base = 10, bool uppercase = false, int showbase = 0,
               std::FILE* f = stdout) const;
    template <typename _IntT>
    friend std::ostream& operator<<(std::ostream& out,
                                    const BigInt<_IntT>& rhs);

   private:
    // construct from raw data
    explicit BigInt(const IntT* data, size_t length);

    // data
    static constexpr size_t LIMB = sizeof(IntT) << 3;
    static constexpr size_t MAX_CAP = size_t(1) << 63;
    size_t cap_ = 1;  // capacity, currently must be *a power of 2*
    size_t len_ = 1;  // actual used length
    IntT* val_ = nullptr;

    // random device
    // NOLINTNEXTLINE: c++17 ok
    inline static std::random_device rand_dev_;
    // NOLINTNEXTLINE: c++17 ok
    inline static auto rand_gen_ = static_cast<std::mt19937>(rand_dev_());
    // usage: rand_(rand_gen_)
    // NOLINTNEXTLINE: c++17 ok
    inline static std::uniform_int_distribution<IntT> rand_;

    // private functions
    // used to align the length if needed
    // accept cut and expand
    void SetLen(size_t new_len, bool preserve_sign);
    // shrink the len_,
    // must ensure target len is definitely not longer than current len.
    // preserve sign
    // @return: len after shrink
    void ShrinkLen();
    void Resize(size_t new_cap);
    void AutoExpandSize(size_t target_len);
    // Shrink size if certain condition is met
    void AutoShrinkSize();

    // FFT
    template <typename T>
    static void BitRevSort(T* a, size_t n);
    static void FFT(std::complex<double>* dest, size_t n, bool inv);
};

// specialization
// LIMB>21 can't use AlgB, redirect to AlgA
template <>
BigInt<uint32_t>& BigInt<uint32_t>::DivEqAlgB(const BigInt& rhs, BigInt* mod);

// stream operators
template <typename IntT>
std::istream& operator>>(std::istream& in, BigInt<IntT>& rhs);
template <typename IntT>
std::ostream& operator<<(std::ostream& out, const BigInt<IntT>& rhs);

// non-modifying binary operators
// Google: prefer to define non-modifying binary operators as non-member func
template <typename IntT>
BigInt<IntT> operator&(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator|(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator^(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator<<(BigInt<IntT> lhs, size_t rhs);
template <typename IntT>
BigInt<IntT> operator>>(BigInt<IntT> lhs, size_t rhs);
template <typename IntT>
BigInt<IntT> operator+(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator-(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator*(BigInt<IntT> lhs, IntT rhs);
template <typename IntT>
BigInt<IntT> operator*(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator/(BigInt<IntT> lhs, IntT rhs);
template <typename IntT>
BigInt<IntT> operator/(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
template <typename IntT>
BigInt<IntT> operator%(BigInt<IntT> lhs, IntT rhs);
template <typename IntT>
BigInt<IntT> operator%(BigInt<IntT> lhs, const BigInt<IntT>& rhs);
#ifndef __cpp_impl_three_way_comparison
template <typename IntT>
bool operator<(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
template <typename IntT>
bool operator>(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
template <typename IntT>
bool operator<=(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
template <typename IntT>
bool operator>=(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
template <typename IntT>
bool operator==(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
template <typename IntT>
bool operator!=(const BigInt<IntT>& lhs, const BigInt<IntT>& rhs);
#endif

// explicit instanization
extern template class BigInt<uint8_t>;
extern template class BigInt<uint16_t>;
extern template class BigInt<uint32_t>;

// explicit instanizaiton of operators
extern template BigInt<uint8_t> operator&(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator|(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator^(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator<<(BigInt<uint8_t> lhs, size_t rhs);
extern template BigInt<uint8_t> operator>>(BigInt<uint8_t> lhs, size_t rhs);
extern template std::istream& operator>>(std::istream& in,
                                         BigInt<uint8_t>& rhs);
extern template std::ostream& operator<<(std::ostream& out,
                                         const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator+(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator-(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator*(BigInt<uint8_t> lhs, uint8_t rhs);
extern template BigInt<uint8_t> operator*(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator/(BigInt<uint8_t> lhs, uint8_t rhs);
extern template BigInt<uint8_t> operator/(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint8_t> operator%(BigInt<uint8_t> lhs, uint8_t rhs);
extern template BigInt<uint8_t> operator%(BigInt<uint8_t> lhs,
                                          const BigInt<uint8_t>& rhs);
extern template BigInt<uint16_t> operator&(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator|(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator^(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator<<(BigInt<uint16_t> lhs, size_t rhs);
extern template BigInt<uint16_t> operator>>(BigInt<uint16_t> lhs, size_t rhs);
extern template std::istream& operator>>(std::istream& in,
                                         BigInt<uint16_t>& rhs);
extern template std::ostream& operator<<(std::ostream& out,
                                         const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator+(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator-(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator*(BigInt<uint16_t> lhs, uint16_t rhs);
extern template BigInt<uint16_t> operator*(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator/(BigInt<uint16_t> lhs, uint16_t rhs);
extern template BigInt<uint16_t> operator/(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint16_t> operator%(BigInt<uint16_t> lhs, uint16_t rhs);
extern template BigInt<uint16_t> operator%(BigInt<uint16_t> lhs,
                                           const BigInt<uint16_t>& rhs);
extern template BigInt<uint32_t> operator&(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator|(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator^(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator<<(BigInt<uint32_t> lhs, size_t rhs);
extern template BigInt<uint32_t> operator>>(BigInt<uint32_t> lhs, size_t rhs);
extern template std::istream& operator>>(std::istream& in,
                                         BigInt<uint32_t>& rhs);
extern template std::ostream& operator<<(std::ostream& out,
                                         const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator+(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator-(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator*(BigInt<uint32_t> lhs, uint32_t rhs);
extern template BigInt<uint32_t> operator*(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator/(BigInt<uint32_t> lhs, uint32_t rhs);
extern template BigInt<uint32_t> operator/(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
extern template BigInt<uint32_t> operator%(BigInt<uint32_t> lhs, uint32_t rhs);
extern template BigInt<uint32_t> operator%(BigInt<uint32_t> lhs,
                                           const BigInt<uint32_t>& rhs);
#ifndef __cpp_impl_three_way_comparison
extern template bool operator<(const BigInt<uint8_t>& lhs,
                               const BigInt<uint8_t>& rhs);
extern template bool operator>(const BigInt<uint8_t>& lhs,
                               const BigInt<uint8_t>& rhs);
extern template bool operator<=(const BigInt<uint8_t>& lhs,
                                const BigInt<uint8_t>& rhs);
extern template bool operator>=(const BigInt<uint8_t>& lhs,
                                const BigInt<uint8_t>& rhs);
extern template bool operator==(const BigInt<uint8_t>& lhs,
                                const BigInt<uint8_t>& rhs);
extern template bool operator!=(const BigInt<uint8_t>& lhs,
                                const BigInt<uint8_t>& rhs);
extern template bool operator<(const BigInt<uint16_t>& lhs,
                               const BigInt<uint16_t>& rhs);
extern template bool operator>(const BigInt<uint16_t>& lhs,
                               const BigInt<uint16_t>& rhs);
extern template bool operator<=(const BigInt<uint16_t>& lhs,
                                const BigInt<uint16_t>& rhs);
extern template bool operator>=(const BigInt<uint16_t>& lhs,
                                const BigInt<uint16_t>& rhs);
extern template bool operator==(const BigInt<uint16_t>& lhs,
                                const BigInt<uint16_t>& rhs);
extern template bool operator!=(const BigInt<uint16_t>& lhs,
                                const BigInt<uint16_t>& rhs);
extern template bool operator<(const BigInt<uint32_t>& lhs,
                               const BigInt<uint32_t>& rhs);
extern template bool operator>(const BigInt<uint32_t>& lhs,
                               const BigInt<uint32_t>& rhs);
extern template bool operator<=(const BigInt<uint32_t>& lhs,
                                const BigInt<uint32_t>& rhs);
extern template bool operator>=(const BigInt<uint32_t>& lhs,
                                const BigInt<uint32_t>& rhs);
extern template bool operator==(const BigInt<uint32_t>& lhs,
                                const BigInt<uint32_t>& rhs);
extern template bool operator!=(const BigInt<uint32_t>& lhs,
                                const BigInt<uint32_t>& rhs);
#endif
}  // namespace calc
#endif
