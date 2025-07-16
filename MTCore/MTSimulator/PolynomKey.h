#include "robin_hood.h"
#include "INDICATORSTRUCTRES.H"

#pragma once

struct PolynomrKey {
	PolynomrKey(IndicatorType t = IndicatorType::NONE, IndicatorLen l = IndicatorLen::NONE, IndicatorValueType vt = IndicatorValueType::Value)
		: type(t), len(l), valueType(vt) {
	}
	IndicatorType type;
	IndicatorLen len;
	IndicatorValueType valueType;

	bool operator==(const PolynomrKey& other) const {
		return type == other.type && len == other.len && valueType == other.valueType;
	}
};

namespace std {
	template<>
	struct hash<PolynomrKey> {
		std::size_t operator()(const PolynomrKey& k) const {
			return (static_cast<std::size_t>(k.type) << 16) ^
				(static_cast<std::size_t>(k.len) << 8) ^
				static_cast<std::size_t>(k.valueType);
		}
	};
}

