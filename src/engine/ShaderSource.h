#pragma once
#include "Resource.h"
#include "render_enums.h"
#include <string>

// some shader capabilities (as bitset)
#define HAS_UNLIT				(1 << 0)
#define HAS_AMBIENT				(1 << 1)
#define HAS_DIRECTIONAL_LIGHT	(1 << 2)
#define HAS_POINT_LIGHT			(1 << 3)
#define HAS_SPOT_LIGHT			(1 << 4)
#define HAS_EMISSION			(1 << 5)
#define HAS_ALPHA_CLIP			(1 << 6)
#define HAS_ALPHA_BLEND			(1 << 7)
#define HAS_SKINNED				(1 << 8)
#define HAS_BILLBOARD			(1 << 9)

class ShaderSource : public Resource {
public:
	ShaderSource(const char* buf);

	// Inherited via Resource
	virtual const char* type() override;

	// debug
	void debugPrint();

	int caps;	// capabilities
	std::string src;	// the source itself

private:
	void queryCaps();
};


struct ShaderKey {

	ShaderKey(const ShaderSource* _src, LightType _lt, OpacityType _ot, GeomType _gt) {
		source = _src;
		light_type = _lt;
		opacity_type = _ot;
		geom_type = _gt;
	}

	size_t computeHash() const;
	bool operator==(const ShaderKey& o) const {
		return computeHash() == o.computeHash();
	}
	
	const ShaderSource* source;
	LightType light_type;
	OpacityType opacity_type;
	GeomType geom_type;
};

namespace std {
	template <>
	struct hash<ShaderKey> {
		size_t operator()(const ShaderKey& k) const {
			return k.computeHash();
		}
	};
}