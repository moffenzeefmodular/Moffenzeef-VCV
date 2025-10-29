#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace Mapping
{

namespace Impl
{

template<typename T, std::size_t LEN, typename rangeClass, typename F>
struct EmptyArray {
	constexpr EmptyArray(const F &func)
		: data() {
		constexpr auto min = rangeClass::min;
		constexpr auto max = rangeClass::max;

		for (std::size_t i = 0; i < LEN; i++) {
			auto x = min + i * (max - min) / float(LEN - 1);
			data[i] = func(x);
		}
	}
	std::array<T, LEN> data;
};

} // namespace Impl

template<std::size_t LEN, typename T>
class LookupTable_t {
public:
	using Base_t = std::array<T, LEN>;

public:
	constexpr LookupTable_t(const T min_, const T max_, const Base_t &input)
		: min(min_)
		, max(max_) {
		static_assert(LEN >= 2);
		std::copy_n(input.begin(), LEN, points.begin());
	}

	constexpr T lookup(T val) const {
		float idx = ((val - min) / (max - min)) * (LEN - 1);

		if (idx <= 0.f)
			return points.front();
		else if (idx >= (LEN - 1))
			return points.back();
		else {
			auto lower_idx = (uint32_t)idx;
			float phase = idx - lower_idx;
			auto lower = points[lower_idx];
			auto upper = points[lower_idx + 1];
			return lower + phase * (upper - lower);
		}
	}

	constexpr T operator()(T val) const {
		return lookup(val);
	}

private:
	Base_t points;
	const T min;
	const T max;

public:
	template<typename rangeClass, typename F>
	static constexpr LookupTable_t generate_const(const F func) {
		constexpr Impl::EmptyArray<T, LEN, rangeClass, F> dataArray(func);
		return LookupTable_t(rangeClass::min, rangeClass::max, dataArray.data);
	}

	template<typename rangeClass, typename F>
	static LookupTable_t generate(const F func) {
		Impl::EmptyArray<T, LEN, rangeClass, F> dataArray(func);
		return LookupTable_t(rangeClass::min, rangeClass::max, dataArray.data);
	}
};

} // namespace Mapping
