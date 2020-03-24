#include "bigint.hpp"
namespace calc {

// bit arithmetic
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::ToBitInv() {
    for (size_t i = 0; i < len_; ++i) val_[i] = ~val_[i];
    return *this;
}
template <typename IntT>
BigInt<IntT> BigInt<IntT>::operator~() const {
    BigInt<IntT> obj = *this;
    return obj.ToBitInv();
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator&=(const BigInt& rhs) {
    if (len_ <= rhs.len_) {
        if (len_ < rhs.len_) SetLen(rhs.len_, false);
        for (size_t i = 0; i < len_; ++i) val_[i] &= rhs.val_[i];
    } else {
        for (size_t i = 0; i < rhs.len_; ++i) val_[i] &= rhs.val_[i];
        // implicit alignment of len_
        if (!rhs.Sign()) {
            std::fill(val_ + rhs.len_, val_ + len_, IntT(0));
            len_ = rhs.len_;
        }
    }
    ShrinkLen();
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator|=(const BigInt& rhs) {
    if (len_ <= rhs.len_) {
        if (len_ < rhs.len_) SetLen(rhs.len_, false);
        for (size_t i = 0; i < len_; ++i) val_[i] |= rhs.val_[i];
    } else {
        for (size_t i = 0; i < rhs.len_; ++i) val_[i] |= rhs.val_[i];
        // implicit alignment of len_
        if (rhs.Sign()) {
            val_[rhs.len_] = IntT(-1);
            if (rhs.len_ + 1 < len_)
                std::fill(val_ + rhs.len_ + 1, val_ + len_, IntT(0));
            len_ = rhs.len_ + 1;
        }
    }
    ShrinkLen();
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator^=(const BigInt& rhs) {
    if (len_ <= rhs.len_) {
        if (len_ < rhs.len_) SetLen(rhs.len_, false);
        for (size_t i = 0; i < len_; ++i) val_[i] ^= rhs.val_[i];
    } else {
        for (size_t i = 0; i < rhs.len_; ++i) val_[i] ^= rhs.val_[i];
        // implicit alignment of len_
        if (rhs.Sign())
            for (size_t i = rhs.len_; i < len_; ++i) val_[i] = ~val_[i];
    }
    ShrinkLen();
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator<<=(size_t rhs) {
    size_t q = rhs / LIMB, r = rhs % LIMB;
    size_t new_len = len_ + q + (r > 0);
    if (new_len < len_) new_len = MAX_CAP;  // overflow!
    if (new_len > cap_) AutoExpandSize(new_len);
    if (q >= new_len) {
        std::fill(val_, val_ + len_, IntT(0));
        len_ = 1;
        AutoShrinkSize();
    } else {
        if (r != 0) {
            for (size_t i = new_len - 1; i > q; --i) {
                val_[i] = static_cast<IntT>((val_[i - q] << r) +
                                            (val_[i - q - 1] >> (LIMB - r)));
            }
            val_[q] = static_cast<IntT>(val_[0] << r);
        } else {
            for (size_t i = new_len - 1; i > q; --i) val_[i] = val_[i - q];
        }
        if (q != 0) std::fill(val_, val_ + q, IntT(0));
        len_ = new_len;
        if (len_ > 1 && val_[len_ - 1] == 0) --len_;
    }
    return *this;
}
template <typename IntT>
BigInt<IntT>& BigInt<IntT>::operator>>=(size_t rhs) {
    size_t q = rhs / LIMB, r = rhs % LIMB;
    size_t new_len = len_ - q;
    if (new_len > len_ || new_len == 0) {
        // overflow!
        std::fill(val_, val_ + len_, IntT(0));
        len_ = 1;
    } else {
        if (r != 0) {
            for (size_t i = 0; i < new_len - 1; ++i) {
                val_[i] = static_cast<IntT>((val_[i + q] >> r) +
                                            (val_[i + q + 1] << (LIMB - r)));
            }
            val_[new_len - 1] = static_cast<IntT>(val_[len_ - 1] >> r);
        } else {
            for (size_t i = 0; i < new_len; ++i) val_[i] = val_[i + q];
        }
        if (q != 0) std::fill(val_ + new_len, val_ + len_, IntT(0));
        len_ = new_len;
        if (len_ > 1 && val_[len_ - 1] == 0) --len_;
    }
    AutoShrinkSize();
    return *this;
}

// non-modifying binary operators
template <typename IntT>
BigInt<IntT> operator&(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs &= rhs;
}
template <typename IntT>
BigInt<IntT> operator|(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs |= rhs;
}
template <typename IntT>
BigInt<IntT> operator^(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs ^= rhs;
}
template <typename IntT>
BigInt<IntT> operator+(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs += rhs;
}
template <typename IntT>
BigInt<IntT> operator-(BigInt<IntT> lhs, const BigInt<IntT>& rhs) {
    return lhs -= rhs;
}
}  // namespace calc