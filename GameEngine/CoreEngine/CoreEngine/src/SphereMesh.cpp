#include "CoreMeshes.h"

#include "MeshData.h"
#include "ObjParser.h"

namespace CoreMeshes
{
	const MeshData& Sphere()
	{
		static MeshData mesh;
		static bool initialized = false;

		if (initialized)
			return mesh;

		initialized = true;

		ObjParser data;

		data.Vertices = {
			Vector3(  0.000000f, -1.000000f,  0.000000f ),
			Vector3(  0.723607f, -0.447220f,  0.525725f ),
			Vector3( -0.276388f, -0.447220f,  0.850649f ),
			Vector3( -0.894426f, -0.447216f,  0.000000f ),
			Vector3( -0.276388f, -0.447220f, -0.850649f ),
			Vector3(  0.723607f, -0.447220f, -0.525725f ),
			Vector3(  0.276388f,  0.447220f,  0.850649f ),
			Vector3( -0.723607f,  0.447220f,  0.525725f ),
			Vector3( -0.723607f,  0.447220f, -0.525725f ),
			Vector3(  0.276388f,  0.447220f, -0.850649f ),
			Vector3(  0.894426f,  0.447216f,  0.000000f ),
			Vector3(  0.000000f,  1.000000f,  0.000000f ),
			Vector3( -0.162456f, -0.850654f,  0.499995f ),
			Vector3(  0.425323f, -0.850654f,  0.309011f ),
			Vector3(  0.262869f, -0.525738f,  0.809012f ),
			Vector3(  0.850648f, -0.525736f,  0.000000f ),
			Vector3(  0.425323f, -0.850654f, -0.309011f ),
			Vector3( -0.525730f, -0.850652f,  0.000000f ),
			Vector3( -0.688189f, -0.525736f,  0.499997f ),
			Vector3( -0.162456f, -0.850654f, -0.499995f ),
			Vector3( -0.688189f, -0.525736f, -0.499997f ),
			Vector3(  0.262869f, -0.525738f, -0.809012f ),
			Vector3(  0.951058f,  0.000000f,  0.309013f ),
			Vector3(  0.951058f,  0.000000f, -0.309013f ),
			Vector3(  0.000000f,  0.000000f,  1.000000f ),
			Vector3(  0.587786f,  0.000000f,  0.809017f ),
			Vector3( -0.951058f,  0.000000f,  0.309013f ),
			Vector3( -0.587786f,  0.000000f,  0.809017f ),
			Vector3( -0.587786f,  0.000000f, -0.809017f ),
			Vector3( -0.951058f,  0.000000f, -0.309013f ),
			Vector3(  0.587786f,  0.000000f, -0.809017f ),
			Vector3(  0.000000f,  0.000000f, -1.000000f ),
			Vector3(  0.688189f,  0.525736f,  0.499997f ),
			Vector3( -0.262869f,  0.525738f,  0.809012f ),
			Vector3( -0.850648f,  0.525736f,  0.000000f ),
			Vector3( -0.262869f,  0.525738f, -0.809012f ),
			Vector3(  0.688189f,  0.525736f, -0.499997f ),
			Vector3(  0.162456f,  0.850654f,  0.499995f ),
			Vector3(  0.525730f,  0.850652f,  0.000000f ),
			Vector3( -0.425323f,  0.850654f,  0.309011f ),
			Vector3( -0.425323f,  0.850654f, -0.309011f ),
			Vector3(  0.162456f,  0.850654f, -0.499995f )
		};

		data.UVs = {
			Vector3( 0.818181f, 0.000000f ),
			Vector3( 0.772726f, 0.078731f ),
			Vector3( 0.863635f, 0.078731f ),
			Vector3( 0.727272f, 0.157461f ),
			Vector3( 0.681818f, 0.078731f ),
			Vector3( 0.636363f, 0.157461f ),
			Vector3( 0.090909f, 0.000000f ),
			Vector3( 0.045454f, 0.078731f ),
			Vector3( 0.136363f, 0.078731f ),
			Vector3( 0.272727f, 0.000000f ),
			Vector3( 0.227273f, 0.078731f ),
			Vector3( 0.318182f, 0.078731f ),
			Vector3( 0.454545f, 0.000000f ),
			Vector3( 0.409090f, 0.078731f ),
			Vector3( 0.500000f, 0.078731f ),
			Vector3( 0.681818f, 0.236191f ),
			Vector3( 0.909090f, 0.157461f ),
			Vector3( 0.818181f, 0.157461f ),
			Vector3( 0.863635f, 0.236191f ),
			Vector3( 0.181818f, 0.157461f ),
			Vector3( 0.090909f, 0.157461f ),
			Vector3( 0.136363f, 0.236191f ),
			Vector3( 0.363636f, 0.157461f ),
			Vector3( 0.272727f, 0.157461f ),
			Vector3( 0.318182f, 0.236191f ),
			Vector3( 0.545454f, 0.157461f ),
			Vector3( 0.454545f, 0.157461f ),
			Vector3( 0.500000f, 0.236191f ),
			Vector3( 0.772726f, 0.236191f ),
			Vector3( 0.954545f, 0.236191f ),
			Vector3( 0.227273f, 0.236191f ),
			Vector3( 0.409090f, 0.236191f ),
			Vector3( 0.590909f, 0.236191f ),
			Vector3( 0.818181f, 0.314921f ),
			Vector3( 0.727272f, 0.314921f ),
			Vector3( 0.772726f, 0.393651f ),
			Vector3( 1.000000f, 0.314921f ),
			Vector3( 0.909091f, 0.314921f ),
			Vector3( 0.954545f, 0.393651f ),
			Vector3( 0.272727f, 0.314921f ),
			Vector3( 0.181818f, 0.314921f ),
			Vector3( 0.227273f, 0.393651f ),
			Vector3( 0.454545f, 0.314921f ),
			Vector3( 0.363636f, 0.314921f ),
			Vector3( 0.409090f, 0.393651f ),
			Vector3( 0.636363f, 0.314921f ),
			Vector3( 0.545454f, 0.314921f ),
			Vector3( 0.590909f, 0.393651f ),
			Vector3( 0.500000f, 0.393651f ),
			Vector3( 0.545454f, 0.472382f ),
			Vector3( 0.318182f, 0.393651f ),
			Vector3( 0.363636f, 0.472382f ),
			Vector3( 0.136363f, 0.393651f ),
			Vector3( 0.181818f, 0.472382f ),
			Vector3( 0.090909f, 0.314921f ),
			Vector3( 0.863635f, 0.393651f ),
			Vector3( 0.909090f, 0.472382f ),
			Vector3( 0.681818f, 0.393651f ),
			Vector3( 0.727272f, 0.472382f ),
			Vector3( 0.045454f, 0.236191f ),
			Vector3( 0.000000f, 0.157461f ),
			Vector3( 0.590909f, 0.078731f ),
			Vector3( 0.636363f, 0.000000f )
		};

		data.Normals = {
			Vector3(  0.1024f, -0.9435f,  0.3151f ),
			Vector3(  0.7002f, -0.6617f,  0.2680f ),
			Vector3( -0.2680f, -0.9435f,  0.1947f ),
			Vector3( -0.2680f, -0.9435f, -0.1947f ),
			Vector3(  0.1024f, -0.9435f, -0.3151f ),
			Vector3(  0.9050f, -0.3304f,  0.2680f ),
			Vector3(  0.0247f, -0.3304f,  0.9435f ),
			Vector3( -0.8897f, -0.3304f,  0.3151f ),
			Vector3( -0.5746f, -0.3304f, -0.7488f ),
			Vector3(  0.5346f, -0.3304f, -0.7779f ),
			Vector3(  0.8026f, -0.1256f,  0.5831f ),
			Vector3( -0.3066f, -0.1256f,  0.9435f ),
			Vector3( -0.9921f, -0.1256f,  0.0000f ),
			Vector3( -0.3066f, -0.1256f, -0.9435f ),
			Vector3(  0.8026f, -0.1256f, -0.5831f ),
			Vector3(  0.4089f,  0.6617f,  0.6284f ),
			Vector3( -0.4713f,  0.6617f,  0.5831f ),
			Vector3( -0.7002f,  0.6617f, -0.2680f ),
			Vector3(  0.0385f,  0.6617f, -0.7488f ),
			Vector3(  0.7240f,  0.6617f, -0.1947f ),
			Vector3(  0.2680f,  0.9435f, -0.1947f ),
			Vector3(  0.4911f,  0.7947f, -0.3568f ),
			Vector3(  0.4089f,  0.6617f, -0.6284f ),
			Vector3( -0.1024f,  0.9435f, -0.3151f ),
			Vector3( -0.1876f,  0.7947f, -0.5773f ),
			Vector3( -0.4713f,  0.6617f, -0.5831f ),
			Vector3( -0.3313f,  0.9435f,  0.0000f ),
			Vector3( -0.6071f,  0.7947f,  0.0000f ),
			Vector3( -0.7002f,  0.6617f,  0.2680f ),
			Vector3( -0.1024f,  0.9435f,  0.3151f ),
			Vector3( -0.1876f,  0.7947f,  0.5773f ),
			Vector3(  0.0385f,  0.6617f,  0.7488f ),
			Vector3(  0.2680f,  0.9435f,  0.1947f ),
			Vector3(  0.4911f,  0.7947f,  0.3568f ),
			Vector3(  0.7240f,  0.6617f,  0.1947f ),
			Vector3(  0.8897f,  0.3304f, -0.3151f ),
			Vector3(  0.7947f,  0.1876f, -0.5773f ),
			Vector3(  0.5746f,  0.3304f, -0.7488f ),
			Vector3( -0.0247f,  0.3304f, -0.9435f ),
			Vector3( -0.3035f,  0.1876f, -0.9342f ),
			Vector3( -0.5346f,  0.3304f, -0.7779f ),
			Vector3( -0.9050f,  0.3304f, -0.2680f ),
			Vector3( -0.9822f,  0.1876f,  0.0000f ),
			Vector3( -0.9050f,  0.3304f,  0.2680f ),
			Vector3( -0.5346f,  0.3304f,  0.7779f ),
			Vector3( -0.3035f,  0.1876f,  0.9342f ),
			Vector3( -0.0247f,  0.3304f,  0.9435f ),
			Vector3(  0.5746f,  0.3304f,  0.7488f ),
			Vector3(  0.7947f,  0.1876f,  0.5773f ),
			Vector3(  0.8897f,  0.3304f,  0.3151f ),
			Vector3(  0.3066f,  0.1256f, -0.9435f ),
			Vector3(  0.3035f, -0.1876f, -0.9342f ),
			Vector3(  0.0247f, -0.3304f, -0.9435f ),
			Vector3( -0.8026f,  0.1256f, -0.5831f ),
			Vector3( -0.7947f, -0.1876f, -0.5773f ),
			Vector3( -0.8897f, -0.3304f, -0.3151f ),
			Vector3( -0.8026f,  0.1256f,  0.5831f ),
			Vector3( -0.7947f, -0.1876f,  0.5773f ),
			Vector3( -0.5746f, -0.3304f,  0.7488f ),
			Vector3(  0.3066f,  0.1256f,  0.9435f ),
			Vector3(  0.3035f, -0.1876f,  0.9342f ),
			Vector3(  0.5346f, -0.3304f,  0.7779f ),
			Vector3(  0.9921f,  0.1256f,  0.0000f ),
			Vector3(  0.9822f, -0.1876f,  0.0000f ),
			Vector3(  0.9050f, -0.3304f, -0.2680f ),
			Vector3(  0.4713f, -0.6617f, -0.5831f ),
			Vector3(  0.1876f, -0.7947f, -0.5773f ),
			Vector3( -0.0385f, -0.6617f, -0.7488f ),
			Vector3( -0.4089f, -0.6617f, -0.6284f ),
			Vector3( -0.4911f, -0.7947f, -0.3568f ),
			Vector3( -0.7240f, -0.6617f, -0.1947f ),
			Vector3( -0.7240f, -0.6617f,  0.1947f ),
			Vector3( -0.4911f, -0.7947f,  0.3568f ),
			Vector3( -0.4089f, -0.6617f,  0.6284f ),
			Vector3(  0.7002f, -0.6617f, -0.2680f ),
			Vector3(  0.6071f, -0.7947f,  0.0000f ),
			Vector3(  0.3313f, -0.9435f,  0.0000f ),
			Vector3( -0.0385f, -0.6617f,  0.7488f ),
			Vector3(  0.1876f, -0.7947f,  0.5773f ),
			Vector3(  0.4713f, -0.6617f,  0.5831f )
		};

		data.Colors = { { 1, 1, 1, 1 } };

		data.Faces = {
			Face{ { Vertex( 0,  0,  0, 0), Vertex(13,  1,  0), Vertex(12,  2,  0) } },
			Face{ { Vertex( 1,  3,  1, 0), Vertex(13,  4,  1), Vertex(15,  5,  1) } },
			Face{ { Vertex( 0,  6,  2, 0), Vertex(12,  7,  2), Vertex(17,  8,  2) } },
			Face{ { Vertex( 0,  9,  3, 0), Vertex(17, 10,  3), Vertex(19, 11,  3) } },
			Face{ { Vertex( 0, 12,  4, 0), Vertex(19, 13,  4), Vertex(16, 14,  4) } },
			Face{ { Vertex( 1,  3,  5, 0), Vertex(15,  5,  5), Vertex(22, 15,  5) } },
			Face{ { Vertex( 2, 16,  6, 0), Vertex(14, 17,  6), Vertex(24, 18,  6) } },
			Face{ { Vertex( 3, 19,  7, 0), Vertex(18, 20,  7), Vertex(26, 21,  7) } },
			Face{ { Vertex( 4, 22,  8, 0), Vertex(20, 23,  8), Vertex(28, 24,  8) } },
			Face{ { Vertex( 5, 25,  9, 0), Vertex(21, 26,  9), Vertex(30, 27,  9) } },
			Face{ { Vertex( 1,  3, 10, 0), Vertex(22, 15, 10), Vertex(25, 28, 10) } },
			Face{ { Vertex( 2, 16, 11, 0), Vertex(24, 18, 11), Vertex(27, 29, 11) } },
			Face{ { Vertex( 3, 19, 12, 0), Vertex(26, 21, 12), Vertex(29, 30, 12) } },
			Face{ { Vertex( 4, 22, 13, 0), Vertex(28, 24, 13), Vertex(31, 31, 13) } },
			Face{ { Vertex( 5, 25, 14, 0), Vertex(30, 27, 14), Vertex(23, 32, 14) } },
			Face{ { Vertex( 6, 33, 15, 0), Vertex(32, 34, 15), Vertex(37, 35, 15) } },
			Face{ { Vertex( 7, 36, 16, 0), Vertex(33, 37, 16), Vertex(39, 38, 16) } },
			Face{ { Vertex( 8, 39, 17, 0), Vertex(34, 40, 17), Vertex(40, 41, 17) } },
			Face{ { Vertex( 9, 42, 18, 0), Vertex(35, 43, 18), Vertex(41, 44, 18) } },
			Face{ { Vertex(10, 45, 19, 0), Vertex(36, 46, 19), Vertex(38, 47, 19) } },
			Face{ { Vertex(38, 47, 20, 0), Vertex(41, 48, 20), Vertex(11, 49, 20) } },
			Face{ { Vertex(38, 47, 21, 0), Vertex(36, 46, 21), Vertex(41, 48, 21) } },
			Face{ { Vertex(36, 46, 22, 0), Vertex( 9, 42, 22), Vertex(41, 48, 22) } },
			Face{ { Vertex(41, 44, 23, 0), Vertex(40, 50, 23), Vertex(11, 51, 23) } },
			Face{ { Vertex(41, 44, 24, 0), Vertex(35, 43, 24), Vertex(40, 50, 24) } },
			Face{ { Vertex(35, 43, 25, 0), Vertex( 8, 39, 25), Vertex(40, 50, 25) } },
			Face{ { Vertex(40, 41, 26, 0), Vertex(39, 52, 26), Vertex(11, 53, 26) } },
			Face{ { Vertex(40, 41, 27, 0), Vertex(34, 40, 27), Vertex(39, 52, 27) } },
			Face{ { Vertex(34, 40, 28, 0), Vertex( 7, 54, 28), Vertex(39, 52, 28) } },
			Face{ { Vertex(39, 38, 29, 0), Vertex(37, 55, 29), Vertex(11, 56, 29) } },
			Face{ { Vertex(39, 38, 30, 0), Vertex(33, 37, 30), Vertex(37, 55, 30) } },
			Face{ { Vertex(33, 37, 31, 0), Vertex( 6, 33, 31), Vertex(37, 55, 31) } },
			Face{ { Vertex(37, 35, 32, 0), Vertex(38, 57, 32), Vertex(11, 58, 32) } },
			Face{ { Vertex(37, 35, 33, 0), Vertex(32, 34, 33), Vertex(38, 57, 33) } },
			Face{ { Vertex(32, 34, 34, 0), Vertex(10, 45, 34), Vertex(38, 57, 34) } },
			Face{ { Vertex(23, 32, 35, 0), Vertex(36, 46, 35), Vertex(10, 45, 35) } },
			Face{ { Vertex(23, 32, 36, 0), Vertex(30, 27, 36), Vertex(36, 46, 36) } },
			Face{ { Vertex(30, 27, 37, 0), Vertex( 9, 42, 37), Vertex(36, 46, 37) } },
			Face{ { Vertex(31, 31, 38, 0), Vertex(35, 43, 38), Vertex( 9, 42, 38) } },
			Face{ { Vertex(31, 31, 39, 0), Vertex(28, 24, 39), Vertex(35, 43, 39) } },
			Face{ { Vertex(28, 24, 40, 0), Vertex( 8, 39, 40), Vertex(35, 43, 40) } },
			Face{ { Vertex(29, 30, 41, 0), Vertex(34, 40, 41), Vertex( 8, 39, 41) } },
			Face{ { Vertex(29, 30, 42, 0), Vertex(26, 21, 42), Vertex(34, 40, 42) } },
			Face{ { Vertex(26, 21, 43, 0), Vertex( 7, 54, 43), Vertex(34, 40, 43) } },
			Face{ { Vertex(27, 29, 44, 0), Vertex(33, 37, 44), Vertex( 7, 36, 44) } },
			Face{ { Vertex(27, 29, 45, 0), Vertex(24, 18, 45), Vertex(33, 37, 45) } },
			Face{ { Vertex(24, 18, 46, 0), Vertex( 6, 33, 46), Vertex(33, 37, 46) } },
			Face{ { Vertex(25, 28, 47, 0), Vertex(32, 34, 47), Vertex( 6, 33, 47) } },
			Face{ { Vertex(25, 28, 48, 0), Vertex(22, 15, 48), Vertex(32, 34, 48) } },
			Face{ { Vertex(22, 15, 49, 0), Vertex(10, 45, 49), Vertex(32, 34, 49) } },
			Face{ { Vertex(30, 27, 50, 0), Vertex(31, 31, 50), Vertex( 9, 42, 50) } },
			Face{ { Vertex(30, 27, 51, 0), Vertex(21, 26, 51), Vertex(31, 31, 51) } },
			Face{ { Vertex(21, 26, 52, 0), Vertex( 4, 22, 52), Vertex(31, 31, 52) } },
			Face{ { Vertex(28, 24, 53, 0), Vertex(29, 30, 53), Vertex( 8, 39, 53) } },
			Face{ { Vertex(28, 24, 54, 0), Vertex(20, 23, 54), Vertex(29, 30, 54) } },
			Face{ { Vertex(20, 23, 55, 0), Vertex( 3, 19, 55), Vertex(29, 30, 55) } },
			Face{ { Vertex(26, 21, 56, 0), Vertex(27, 59, 56), Vertex( 7, 54, 56) } },
			Face{ { Vertex(26, 21, 57, 0), Vertex(18, 20, 57), Vertex(27, 59, 57) } },
			Face{ { Vertex(18, 20, 58, 0), Vertex( 2, 60, 58), Vertex(27, 59, 58) } },
			Face{ { Vertex(24, 18, 59, 0), Vertex(25, 28, 59), Vertex( 6, 33, 59) } },
			Face{ { Vertex(24, 18, 60, 0), Vertex(14, 17, 60), Vertex(25, 28, 60) } },
			Face{ { Vertex(14, 17, 61, 0), Vertex( 1,  3, 61), Vertex(25, 28, 61) } },
			Face{ { Vertex(22, 15, 62, 0), Vertex(23, 32, 62), Vertex(10, 45, 62) } },
			Face{ { Vertex(22, 15, 63, 0), Vertex(15,  5, 63), Vertex(23, 32, 63) } },
			Face{ { Vertex(15,  5, 64, 0), Vertex( 5, 25, 64), Vertex(23, 32, 64) } },
			Face{ { Vertex(16, 14, 65, 0), Vertex(21, 26, 65), Vertex( 5, 25, 65) } },
			Face{ { Vertex(16, 14, 66, 0), Vertex(19, 13, 66), Vertex(21, 26, 66) } },
			Face{ { Vertex(19, 13, 67, 0), Vertex( 4, 22, 67), Vertex(21, 26, 67) } },
			Face{ { Vertex(19, 11, 68, 0), Vertex(20, 23, 68), Vertex( 4, 22, 68) } },
			Face{ { Vertex(19, 11, 69, 0), Vertex(17, 10, 69), Vertex(20, 23, 69) } },
			Face{ { Vertex(17, 10, 70, 0), Vertex( 3, 19, 70), Vertex(20, 23, 70) } },
			Face{ { Vertex(17,  8, 71, 0), Vertex(18, 20, 71), Vertex( 3, 19, 71) } },
			Face{ { Vertex(17,  8, 72, 0), Vertex(12,  7, 72), Vertex(18, 20, 72) } },
			Face{ { Vertex(12,  7, 73, 0), Vertex( 2, 60, 73), Vertex(18, 20, 73) } },
			Face{ { Vertex(15,  5, 74, 0), Vertex(16, 61, 74), Vertex( 5, 25, 74) } },
			Face{ { Vertex(15,  5, 75, 0), Vertex(13,  4, 75), Vertex(16, 61, 75) } },
			Face{ { Vertex(13,  4, 76, 0), Vertex( 0, 62, 76), Vertex(16, 61, 76) } },
			Face{ { Vertex(12,  2, 77, 0), Vertex(14, 17, 77), Vertex( 2, 16, 77) } },
			Face{ { Vertex(12,  2, 78, 0), Vertex(13,  1, 78), Vertex(14, 17, 78) } },
			Face{ { Vertex(13,  1, 79, 0), Vertex( 1,  3, 79), Vertex(14, 17, 79) } }
		};

		mesh = MeshData(&data, Enum::VertexMode::Seperate);

		return mesh;
	}
}