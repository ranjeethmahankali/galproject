using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace RhinoInterface
{
    internal static class Unsafe
    {
        private const string dllName = "geom_algo_lib.dll";

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Test_GetSquare(int n);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ConvexHull_Create(
            [MarshalAs(UnmanagedType.LPArray)] double[] coordinates,
            ulong numPoints,
            ref IntPtr faceIndices,
            ref int nFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ReleaseInt(IntPtr arr, bool isArray);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ReleaseDouble(IntPtr arr, bool isArray);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_GetData(
            IntPtr meshptr,
            ref IntPtr vertices,
            ref int nVertices,
            ref IntPtr faces,
            ref int nFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr Mesh_Create(
            [MarshalAs(UnmanagedType.LPArray)] double[] vertCoords,
            int numVerts,
            [MarshalAs(UnmanagedType.LPArray)] int[] faceIndices,
            int numFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_Delete(IntPtr meshPtr);
    }
}
