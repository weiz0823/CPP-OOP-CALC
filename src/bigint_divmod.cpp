#include <cassert>
#include "bigint.hpp"
namespace calc {
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator/=(IntT rhs) {
    BasicDivEq(rhs);
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator%=(IntT rhs) {
    if (rhs == 0) return *this;
    const uint64_t limb_mod = (1l << LIMB) % rhs;
    uint64_t cur_mod = 1;
    uint64_t tot = 0;
    bool sign = Sign();
    if (sign) ToOpposite();
    if (limb_mod == 0) {
        tot = val_[0] % rhs;
    } else {
        for (size_t i = 0; i < len_; ++i) {
            tot = (tot + cur_mod * val_[i]) % rhs;
            cur_mod = cur_mod * limb_mod % rhs;
        }
    }
    if (len_ > 1) std::fill(val_ + 1, val_ + len_, IntT(0));
    len_ = 2;
    val_[0] = IntT(tot);
    if (sign) ToOpposite();
    ShrinkLen();
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator/=(const BigInt& rhs) {
    if (len_ <= 64 / LIMB && rhs.len_ <= 64 / LIMB) {
        PlainDivEq(rhs, nullptr);
    } else if (rhs.len_ == 1) {
        BasicDivEq(rhs.val_[0], nullptr);
    } else {
        if constexpr (LIMB > 21)
            DivEqAlgA(rhs, nullptr);
        else
            DivEqAlgB(rhs, nullptr);
    }
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator%=(const BigInt& rhs) {
    BigInt<IntT> mod;
    if (len_ <= 64 / LIMB && rhs.len_ <= 64 / LIMB) {
        PlainDivEq(rhs, &mod);
    } else {
        if constexpr (LIMB > 21)
            DivEqAlgA(rhs, &mod);
        else
            DivEqAlgB(rhs, &mod);
    }
    *this = std::move(mod);
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::BasicDivEq(IntT rhs, IntT* mod) {
    if (rhs == 0 || rhs == 1) return *this;
    uint64_t t = 0;
    bool sign = Sign();
    if (sign) ToOpposite();
    IntT tmp_rhs = rhs, log_rhs = 0;
    while (!(tmp_rhs & 1)) {
        ++log_rhs;
        tmp_rhs >>= 1;
    }

    if (len_ == 1) {
        t = val_[0] % rhs;
        val_[0] /= rhs;
    } else if (tmp_rhs == 1) {
        *this >>= log_rhs;
    } else {
        for (size_t i = len_ - 1; i != size_t(-1); --i) {
            t = ((t % rhs) << LIMB) | val_[i];
            val_[i] = IntT(t / rhs);
        }
    }

    if (sign) ToOpposite();
    ShrinkLen();
    if (mod) {
        if (sign && t)
            *mod = IntT(rhs - t % rhs);
        else
            *mod = IntT(t % rhs);
    }
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::PlainDivEq(const BigInt& rhs, BigInt* mod) {
    if (rhs == BigInt<IntT>(0)) return *this;
    // must ensure len_*LIMB<=64
    bool sign = Sign();
    if (sign) ToOpposite();
    uint64_t x = 0, y = 0, z = 0, w = 0;
    size_t i;
    for (i = len_ - 1; i != size_t(-1); --i) x = (x << LIMB) | val_[i];
    for (i = rhs.len_ - 1; i != size_t(-1); --i) y = (y << LIMB) | rhs.val_[i];
    if (rhs.Sign()) y = 0 - y;
    z = x / y;
    w = x - z * y;
    i = 0;
    while (z) {
        val_[i++] = IntT(z);
        z >>= LIMB;
    }
    std::fill(val_ + i, val_ + len_, IntT(0));
    len_ = i ? i : 1;
    if (Sign()) SetLen(len_ + 1, false);
    if (sign != rhs.Sign()) ToOpposite();
    if (mod) {
        mod->SetLen(64 / LIMB, false);
        i = 0;
        while (w) {
            mod->val_[i++] = IntT(w);
            w >>= LIMB;
        }
        std::fill(mod->val_ + i, mod->val_ + mod->len_, IntT(0));
        mod->len_ = i ? i : 1;
        if (sign) mod->ToOpposite();
    }
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::DivEqAlgA(const BigInt& rhs, BigInt* mod) {
    if (rhs == BigInt<IntT>(0)) return *this;
    bool sign = Sign();
    if (sign) ToOpposite();
    size_t mov = 0;
    uint64_t q, r, u1, u2, v1, v2;
    constexpr uint64_t b = (1l << LIMB);
    if (rhs.Sign()) {
        BigInt<IntT> tmp_obj = -rhs;
        IntT test_tmp = tmp_obj.val_[tmp_obj.len_ - 1];
        while (test_tmp < (IntT(1) << (LIMB - 1))) {
            test_tmp <<= 1;
            ++mov;
        }
        if (*this < tmp_obj) {
            if (mod) *mod = *this;
            std::fill(val_, val_ + len_, IntT(0));
            len_ = 1;
        } else if (tmp_obj.len_ <= 64 / LIMB && len_ <= 64 / LIMB) {
            PlainDivEq(tmp_obj, mod);
        } else if (tmp_obj.len_ == 1) {
            // ensure non-negative
            if (mod) {
                BasicDivEq(tmp_obj.val_[0], mod->val_);
                mod->SetLen(1, false);
            } else {
                BasicDivEq(tmp_obj.val_[0], nullptr);
            }
        } else {
            BigInt<IntT> result;
            result.SetLen(len_ - tmp_obj.len_ + 2, false);
            if (mov) {
                v1 = IntT(tmp_obj.val_[tmp_obj.len_ - 1] << mov) |
                     (tmp_obj.val_[tmp_obj.len_ - 2] >> (LIMB - mov));
                v2 = IntT(tmp_obj.val_[tmp_obj.len_ - 2] << mov) |
                     (tmp_obj.val_[tmp_obj.len_ - 3] >> (LIMB - mov));
            } else {
                v1 = tmp_obj.val_[tmp_obj.len_ - 1];
                v2 = tmp_obj.val_[tmp_obj.len_ - 2];
            }
            u1 = val_[len_ - 1];
            for (size_t i = len_ - tmp_obj.len_; i != size_t(-1); --i) {
                if (mov) {
                    u1 = (u1 << mov) |
                         (val_[i + tmp_obj.len_ - 2] >> (LIMB - mov));
                    u2 = IntT(val_[i + tmp_obj.len_ - 2] << mov) |
                         (val_[i + tmp_obj.len_ - 3] >> (LIMB - mov));
                } else {
                    u2 = val_[i + tmp_obj.len_ - 2];
                }
                q = u1 / v1;
                if (q >= b) q = b - 1;
                r = u1 - q * v1;
                if (q * v2 > b * r + u2) {
                    --q;
                    r += v1;
                }
                *this -= (tmp_obj * IntT(q)) << (i * LIMB);
                if (Sign()) {
                    --q;
                    *this += tmp_obj << (i * LIMB);
                }
                result.val_[i] = q;

                u1 = (uint64_t(val_[i + tmp_obj.len_ - 1]) << LIMB) |
                     val_[i + tmp_obj.len_ - 2];
            }
            if (mod) *mod = *this;
            if (sign != rhs.Sign())
                *this = -result;
            else
                *this = result;
        }
    } else {
        IntT test_tmp = rhs.val_[rhs.len_ - 1];
        while (test_tmp < (IntT(1) << (LIMB - 1))) {
            test_tmp <<= 1;
            ++mov;
        }
        if (*this < rhs) {
            if (mod) *mod = *this;
            std::fill(val_, val_ + len_, IntT(0));
            len_ = 1;
        } else if (rhs.len_ <= 64 / LIMB && len_ <= 64 / LIMB) {
            PlainDivEq(rhs, mod);
        } else if (rhs.len_ == 1) {
            // ensure non-negative
            if (mod) {
                BasicDivEq(rhs.val_[0], mod->val_);
                mod->SetLen(1, false);
            } else {
                BasicDivEq(rhs.val_[0], nullptr);
            }
        } else {
            BigInt<IntT> result;
            result.SetLen(len_ - rhs.len_ + 2, false);
            if (mov) {
                v1 = IntT(rhs.val_[rhs.len_ - 1] << mov) |
                     (rhs.val_[rhs.len_ - 2] >> (LIMB - mov));
                v2 = IntT(rhs.val_[rhs.len_ - 2] << mov) |
                     (rhs.val_[rhs.len_ - 3] >> (LIMB - mov));
            } else {
                v1 = rhs.val_[rhs.len_ - 1];
                v2 = rhs.val_[rhs.len_ - 2];
            }
            u1 = val_[len_ - 1];
            for (size_t i = len_ - rhs.len_; i != size_t(-1); --i) {
                if (mov) {
                    u1 = (u1 << mov) | (val_[i + rhs.len_ - 2] >> (LIMB - mov));
                    u2 = IntT(val_[i + rhs.len_ - 2] << mov) |
                         (val_[i + rhs.len_ - 3] >> (LIMB - mov));
                } else {
                    u2 = val_[i + rhs.len_ - 2];
                }
                q = u1 / v1;
                if (q >= b) q = b - 1;
                r = u1 - q * v1;
                if (q * v2 > b * r + u2) {
                    --q;
                    r += v1;
                }
                *this -= (rhs * IntT(q)) << (i * LIMB);
                if (Sign()) {
                    --q;
                    *this += rhs << (i * LIMB);
                }
                result.val_[i] = q;

                u1 = (uint64_t(val_[i + rhs.len_ - 1]) << LIMB) |
                     val_[i + rhs.len_ - 2];
            }
            if (mod) *mod = *this;
            if (sign != rhs.Sign())
                *this = -result;
            else
                *this = result;
        }
    }
    if (mod && sign) mod->ToOpposite();
    if (mod) mod->ShrinkLen();
    ShrinkLen();
    return *this;
}
template <>
BigInt<uint32_t>& BigInt<uint32_t>::DivEqAlgB(const BigInt& rhs, BigInt* mod) {
    return DivEqAlgA(rhs, mod);
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::DivEqAlgB(const BigInt& rhs, BigInt* mod) {
    if (rhs == BigInt<IntT>(0)) return *this;
    if constexpr (LIMB > 21) return DivEqAlgA(rhs, mod);
    bool sign = Sign();
    if (sign) ToOpposite();
    uint64_t q, u, v;
    constexpr uint64_t b = (1l << LIMB);
    if (rhs.Sign()) {
        BigInt<IntT> tmp_obj = -rhs;
        if (*this < tmp_obj) {
            if (mod) *mod = *this;
            std::fill(val_, val_ + len_, IntT(0));
            len_ = 1;
        } else if (tmp_obj.len_ <= 64 / LIMB && len_ <= 64 / LIMB) {
            PlainDivEq(tmp_obj, mod);
        } else if (tmp_obj.len_ == 1) {
            // ensure non-negative
            if (mod) {
                BasicDivEq(tmp_obj.val_[0], mod->val_);
                mod->SetLen(1, false);
            } else {
                BasicDivEq(tmp_obj.val_[0], nullptr);
            }
        } else {
            BigInt<IntT> result;
            result.SetLen(len_ - tmp_obj.len_ + 2, false);
            v = (uint64_t(tmp_obj.val_[tmp_obj.len_ - 1]) << LIMB) |
                tmp_obj.val_[tmp_obj.len_ - 2];
            u = (uint64_t(val_[len_ - 1]) << LIMB) | val_[len_ - 2];
            for (size_t i = len_ - tmp_obj.len_; i != size_t(-1); --i) {
                q = u / v;
                if (q >= b) q = b - 1;
                *this -= (tmp_obj * IntT(q)) << (i * LIMB);
                if (Sign()) {
                    --q;
                    *this += tmp_obj << (i * LIMB);
                }
                result.val_[i] = q;
                u = (uint64_t(val_[i + tmp_obj.len_ - 1]) << (2 * LIMB)) |
                    (uint64_t(val_[i + tmp_obj.len_ - 2]) << LIMB) |
                    val_[i + tmp_obj.len_ - 3];
            }
            if (mod) *mod = *this;
            if (sign != rhs.Sign())
                *this = -result;
            else
                *this = result;
        }
    } else {
        if (*this < rhs) {
            if (mod) *mod = *this;
            std::fill(val_, val_ + len_, IntT(0));
            len_ = 1;
        } else if (rhs.len_ <= 64 / LIMB && len_ <= 64 / LIMB) {
            PlainDivEq(rhs, mod);
        } else if (rhs.len_ == 1) {
            // ensure non-negative
            if (mod) {
                BasicDivEq(rhs.val_[0], mod->val_);
                mod->SetLen(1, false);
            } else {
                BasicDivEq(rhs.val_[0], nullptr);
            }
        } else {
            BigInt<IntT> result;
            result.SetLen(len_ - rhs.len_ + 2, false);
            v = (uint64_t(rhs.val_[rhs.len_ - 1]) << LIMB) |
                rhs.val_[rhs.len_ - 2];
            u = (uint64_t(val_[len_ - 1]) << LIMB) | val_[len_ - 2];
            for (size_t i = len_ - rhs.len_; i != size_t(-1); --i) {
                q = u / v;
                if (q >= b) q = b - 1;
                *this -= (rhs * IntT(q)) << (i * LIMB);
                if (Sign()) {
                    --q;
                    *this += rhs << (i * LIMB);
                }
                result.val_[i] = q;
                u = (uint64_t(val_[i + rhs.len_ - 1]) << (2 * LIMB)) |
                    (uint64_t(val_[i + rhs.len_ - 2]) << LIMB) |
                    val_[i + rhs.len_ - 3];
            }
            if (mod) *mod = *this;
            if (sign != rhs.Sign())
                *this = -result;
            else
                *this = result;
        }
    }
    if (mod && sign) mod->ToOpposite();
    if (mod) mod->ShrinkLen();
    ShrinkLen();
    return *this;
}

// non-modifying
template <typename IntT>
BigInt<IntT> BigInt<IntT>::BasicDiv(BigInt lhs, IntT rhs, IntT* mod) {
    return lhs.BasicDivEq(rhs, mod);
}
template <typename IntT>
BigInt<IntT> BigInt<IntT>::PlainDiv(BigInt lhs, const BigInt& rhs,
                                    BigInt* mod) {
    return lhs.PlainDivEq(rhs, mod);
}
template <typename IntT>
BigInt<IntT> BigInt<IntT>::DivAlgA(BigInt lhs, const BigInt& rhs, BigInt* mod) {
    return lhs.DivEqAlgA(rhs, mod);
}
template <typename IntT>
BigInt<IntT> BigInt<IntT>::DivAlgB(BigInt lhs, const BigInt& rhs, BigInt* mod) {
    return lhs.DivEqAlgB(rhs, mod);
}

// non-modifying binary operators
template <typename IntT>
BigInt<IntT> operator/(BigInt<IntT> lhs, IntT rhs) {
    return lhs /= rhs;
}
template <typename IntT>
BigInt<IntT> operator/(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs /= rhs;
}
template <typename IntT>
BigInt<IntT> operator%(BigInt<IntT> lhs, IntT rhs) {
    return lhs %= rhs;
}
template <typename IntT>
BigInt<IntT> operator%(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs %= rhs;
}
}  // namespace calc
