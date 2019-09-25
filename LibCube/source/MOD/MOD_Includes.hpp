#pragma once

#include <oishii/reader/binary_reader.hxx>
#include <vector>
#include <glm/glm.hpp>
#include <common.hpp>

namespace libcube {

static inline void skipChunk(oishii::BinaryReader& bReader, u32 offset)
{
	bReader.seek<oishii::Whence::Current>(offset);
}

namespace pikmin1 {

static inline void skipPadding(oishii::BinaryReader& bReader)
{
	const u32 currentPos = bReader.tell();
	const u32 toRead = (~(0x20 - 1) & (currentPos + 0x20 - 1)) - currentPos;
	skipChunk(bReader, toRead);
}

//! @brief  Vector3, contains x, y & z
struct Vector3
{
	constexpr static const char name[] = "Vector3";
	glm::vec3 m_vec;

	static void onRead(oishii::BinaryReader& bReader, Vector3& context)
	{
		context.m_vec.x = bReader.read<float>();
		context.m_vec.y = bReader.read<float>();
		context.m_vec.z = bReader.read<float>();
	}
};

//! @brief Vector2, contains x & y
struct Vector2
{
	constexpr static const char name[] = "Vector2";
	glm::vec2 m_vec;

	static void onRead(oishii::BinaryReader& bReader, Vector2& context)
	{
		context.m_vec.x = bReader.read<float>();
		context.m_vec.y = bReader.read<float>();
	}
};

struct BoundBox
{
	constexpr static const char name[] = "Bounding Box";
	Vector3 m_minBounds;
	Vector3 m_maxBounds;

	static void onRead(oishii::BinaryReader& bReader, BoundBox& context)
	{
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_minBounds);
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_maxBounds);
	}
};

//! @brief Unsure of purpose, used in BaseCollTriInfo
struct Plane
{
	constexpr static const char name[] = "Plane";
	Vector3 m_unk1;
	f32 m_unk2;

	static void onRead(oishii::BinaryReader& bReader, Plane& context)
	{
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_unk1);
		context.m_unk2 = bReader.read<f32>();
	}
};

//! @brief Base Collision Triangle Information
struct BaseCollTriInfo
{
	constexpr static const char name[] = "Base Collision Triangle Information";
	u32 m_unk1;
	u32 m_unk2;
	u32 m_unk3;
	u32 m_unk4;

	u16 m_unk5;
	u16 m_unk6;
	u16 m_unk7;
	u16 m_unk8;

	Plane m_unk9;

	static void onRead(oishii::BinaryReader& bReader, BaseCollTriInfo& context)
	{
		context.m_unk1 = bReader.read<u32>();
		context.m_unk2 = bReader.read<u32>();
		context.m_unk3 = bReader.read<u32>();
		context.m_unk4 = bReader.read<u32>();

		context.m_unk5 = bReader.read<u16>();
		context.m_unk6 = bReader.read<u16>();
		context.m_unk7 = bReader.read<u16>();
		context.m_unk8 = bReader.read<u16>();

		bReader.dispatch<Plane, oishii::Direct, false>(context.m_unk9);
	}
};

//! @brief Unsure of purpose, used in BCTI chunk (0x)
struct BaseRoomInfo
{
	constexpr static const char name[] = "Base Room Information";
	u32 m_unk1;

	static void onRead(oishii::BinaryReader& bReader, BaseRoomInfo& context)
	{
		context.m_unk1 = bReader.read<u32>();
	}
};

struct String {
	constexpr static const char name[] = "Pikmin 1 String";

	std::string m_str;

	static void onRead(oishii::BinaryReader& bReader, String& context)
	{
		const u32 nameLength = bReader.read<u32>();
		std::string nameString(nameLength, 0);

		for (u32 j = 0; j < nameLength; ++j)
			nameString[j] = bReader.read<s8>();

		context.m_str = nameString;
	}
};

struct Colour
{
	u8 m_R, m_G, m_B, m_A;
	Colour() { m_R = m_G = m_B = m_A = 0; }
	Colour(u8 _r, u8 _g, u8 _b, u8 _a) : m_R(_r), m_G(_g), m_B(_b), m_A(_a) {}

	void read(oishii::BinaryReader& bReader)
	{
		m_R = bReader.read<u8>();
		m_G = bReader.read<u8>();
		m_B = bReader.read<u8>();
		m_A = bReader.read<u8>();
	}
};

//! @brief Material
struct Material
{
	u32 m_hasPE; // pixel engine
	u32 m_unk2;
	Colour m_unkColour1;

	// if (hasPE & 1)
	u32 m_unk3;

	void read(oishii::BinaryReader& bReader)
	{
		m_hasPE = bReader.read<u32>();
		m_unk2 = bReader.read<u32>();
		m_unkColour1.read(bReader);

		if (m_hasPE & 1)
		{
			m_unk3 = bReader.read<u32>();
			// PVWPolygonColourInfo
			// PVWLightingInfo
			// PVWPeInfo
			// PVWTextureInfo
		}
	}
};

//! @brief DispList, contains u8 vector full of face data
struct DispList
{
	constexpr static const char name[] = "Display List";

	u32 m_unk1 = 0;
	u32 m_unk2 = 0;

	std::vector<u8> m_dispData;

	static void onRead(oishii::BinaryReader& bReader, DispList& context)
	{
		context.m_unk1 = bReader.read<u32>();
		context.m_unk2 = bReader.read<u32>();
		context.m_dispData.resize(bReader.read<u32>());

		skipPadding(bReader);
		// Read the displist data
		for (auto& dispData : context.m_dispData)
			dispData = bReader.read<u8>();
	}
};

//! @brief Matrix Group, contains DispList
struct MtxGroup
{
	constexpr static const char name[] = "Matrix Group";

	std::vector<u16> m_unk1Array;
	std::vector<DispList> m_dispLists;

	static void onRead(oishii::BinaryReader& bReader, MtxGroup& context)
	{
		// Unknown purpose of the array, store it anyways
		context.m_unk1Array.resize(bReader.read<u32>());
		for (auto& unkArr : context.m_unk1Array)
			unkArr = bReader.read<u16>();

		context.m_dispLists.resize(bReader.read<u32>());

		for (auto& dLists : context.m_dispLists)
		{
			bReader.dispatch<DispList, oishii::Direct, false>(dLists);
		}
	}
};

//! @brief Batch, contains Matrix Group
struct Batch
{
	constexpr static const char name[] = "Mesh Batch";

	u32 m_unk1 = 0;
	u32 m_vcd = 0;	//	Vertex Descriptor

	std::vector<MtxGroup> m_mtxGroups;

	static void onRead(oishii::BinaryReader& bReader, Batch& context)
	{
		// Read the batch variables
		context.m_unk1 = bReader.read<u32>();
		context.m_vcd = bReader.read<u32>(); // Vertex descriptor

		context.m_mtxGroups.resize(bReader.read<u32>());
		for (auto& mGroup : context.m_mtxGroups)
		{
			bReader.dispatch<MtxGroup, oishii::Direct, false>(mGroup);
		}
	}
};

//! @brief Joint, contains MatPoly
struct Joint
{
	constexpr static const char name[] = "Mesh Joint";

	u32 m_unk1;
	bool m_unk2;
	bool m_unk3;
	Vector3 m_boundsMin;
	Vector3 m_boundsMax;
	f32 m_boundsSphereRadius;
	Vector3 m_scale;
	Vector3 m_rotation;
	Vector3 m_translation;
	struct MatPoly
	{
		u16 m_index; // I think this is an index of sorts, not sure what though. :/
		u16 m_unk2;
	};
	std::vector<MatPoly> m_matpolys;


	static void onRead(oishii::BinaryReader& bReader, Joint& context)
	{
		context.m_unk1 = bReader.read<u32>();

		const u16 unk1 = static_cast<u16>(bReader.read<u32>());
		context.m_unk2 = (unk1 & 1) != 0;
		context.m_unk3 = (unk1 & 0x4000) != 0;

		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_boundsMin);
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_boundsMax);
		context.m_boundsSphereRadius = bReader.read<f32>();
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_scale);
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_rotation);
		bReader.dispatch<Vector3, oishii::Direct, false>(context.m_translation);

		context.m_matpolys.resize(bReader.read<u32>());
		for (auto& matpoly : context.m_matpolys)
		{
			matpoly.m_index = bReader.read<u16>();
			matpoly.m_unk2 = bReader.read<u16>();
		}
	}
};

struct VtxMatrix
{
	constexpr static const char name[] = "VtxMatrix";

	bool m_partiallyWeighted;
	u16 m_index;

	static void onRead(oishii::BinaryReader& bReader, VtxMatrix& context)
	{
		const s16 idx = bReader.read<s16>();
		context.m_index = (context.m_partiallyWeighted = idx >= 0) ? idx : -idx - 1;
	}
};

struct NBT
{
	Vector3 m_normals;
	Vector3 m_binormals;
	Vector3 m_tangents;
};

struct CollGroup
{
	constexpr static const char name[] = "Collision Group";

	BoundBox m_collBounds;

	float m_unk1;
	u32 m_unkCount1;
	u32 m_unkCount2;
	u32 m_blockSize;

	static void onRead(oishii::BinaryReader& bReader, CollGroup& context)
	{
		bReader.dispatch<BoundBox, oishii::Direct, false>(context.m_collBounds);
		context.m_unk1 = bReader.read<f32>();
		context.m_unkCount1 = bReader.read<u32>();
		context.m_unkCount2 = bReader.read<u32>();
		context.m_blockSize = bReader.read<u32>();

		u32 maxCollTris = 0;
		for (u32 i = 0; i < context.m_blockSize; i++)
		{
			const u16 unkCount1 = bReader.read<u16>();
			const u16 collTriCount = bReader.read<u16>();

			if (collTriCount > maxCollTris)
				maxCollTris = collTriCount;

			for (u32 j = 0; j < collTriCount; j++)
				bReader.read<u32>();

			if (unkCount1)
				for (u32 k = 0; k < unkCount1; k++)
					bReader.read<u8>();
		}

		DebugReport("Max collision triangles within a block: %u\n", maxCollTris);

		for (u32 i = 0; i < context.m_unkCount2; i++)
		{
			for (u32 j = 0; j < context.m_unkCount1; j++)
			{
				const u32 unk1 = bReader.read<u32>();
			}
		}
	}
};

struct Envelope
{
	constexpr static const char name[] = "Collision Group";

	std::vector<u16> m_indexes;
	std::vector<f32> m_weights;

	static void onRead(oishii::BinaryReader& bReader, Envelope& context)
	{
		context.m_indexes.resize(bReader.read<u16>());
		context.m_weights.resize(context.m_indexes.size());

		for (u32 i = 0; i < context.m_indexes.size(); i++)
		{
			context.m_indexes[i] = bReader.read<u16>();
			context.m_weights[i] = bReader.read<f32>();
		}
	}
};

}

}
